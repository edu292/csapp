#include "cachelab.h"
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint64_t tag;
  uint32_t previous;
  uint32_t next;
} Line;

typedef struct {
  size_t valid_lines;
  Line *lines;
  uint32_t most_recent;
  uint32_t least_recent;
} Set;

typedef struct {
  size_t block_bits;
  size_t set_bits;
  size_t lines_per_set;
  Set *sets;
} Cache;

typedef struct {
  uint8_t hit : 1;
  uint8_t miss : 1;
  uint8_t eviction : 1;
} AccessStatus;

Cache *new_cache(size_t set_bits, size_t lines_per_set, size_t block_bits) {
  size_t sets_capacity = 1UL << set_bits;

  size_t cache_size = sizeof(Cache);
  size_t sets_size = sizeof(Set) * sets_capacity;
  size_t lines_size = sizeof(Line) * lines_per_set * sets_capacity;

  char *mem = malloc(cache_size + sets_size + lines_size);
  if (!mem)
    return NULL;

  Cache *c = (Cache *)mem;
  c->sets = (Set *)(mem + cache_size);
  Line *all_lines = (Line *)(mem + cache_size + sets_size);

  c->lines_per_set = lines_per_set;
  c->block_bits = block_bits;
  c->set_bits = set_bits;

  for (size_t i = 0; i < sets_capacity; i++) {
    Set *s = &c->sets[i];
    s->valid_lines = 0;
    s->lines = &all_lines[i * lines_per_set];
    s->most_recent = 0;
    s->least_recent = 0;
  }

  return c;
}

void move_to_most_recent(Set *s, uint32_t line_index, bool splice) {
  if (s->most_recent == line_index) {
    return;
  }

  Line *current = &s->lines[line_index];
  if (s->least_recent == line_index) {
    s->least_recent = current->previous;
  } else if (splice)
    s->lines[current->next].previous = current->previous;

  if (splice) {
    s->lines[current->previous].next = current->next;
  }

  current->next = s->most_recent;
  s->lines[s->most_recent].previous = line_index;
  s->most_recent = line_index;
}

AccessStatus cache_upsert(Cache *c, uint64_t address) {
  AccessStatus st = {false, false, false};

  uint32_t set_index = (address >> c->block_bits) & ((1 << c->set_bits) - 1);
  uint64_t tag = address >> (c->block_bits + c->set_bits);
  Set *s = &c->sets[set_index];

  uint32_t line_index = s->most_recent;
  for (size_t i = 0; i < s->valid_lines; i++) {
    Line *current = &s->lines[line_index];

    if (current->tag == tag) {
      move_to_most_recent(s, line_index, true);
      st.hit = true;
      return st;
    }

    if (i == c->lines_per_set - 1) {
      st.eviction = true;
      break;
    }

    line_index = current->next;
  }

  st.miss = true;
  if (!st.eviction) {
    line_index = s->valid_lines;
    s->valid_lines++;
  }

  Line *target = &s->lines[line_index];
  target->tag = tag;
  move_to_most_recent(s, line_index, st.eviction);

  return st;
}

void print_usage(char *argv[]) {
  printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
  printf("  -h: Optional help flag that prints usage info\n");
  printf("  -v: Optional verbose flag that displays trace info\n");
  printf(
      "  -s <s>: Number of set index bits (S = 2^s is the number of sets)\n");
  printf("  -E <E>: Associativity (number of lines per set)\n");
  printf("  -b <b>: Number of block bits (B = 2^b is the block size)\n");
  printf("  -t <tracefile>: Name of the valgrind trace to replay\n");
}

int main(int argc, char **argv) {
  int opt;
  size_t set_bits = 0, lines_per_set = 0, block_bits = 0;
  bool verbose = false;
  char *tracefile_path = NULL;

  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
    case 'h':
      print_usage(argv);
      exit(0);
    case 'v':
      verbose = true;
      break;
    case 's':
      set_bits = atoi(optarg);
      break;
    case 'E':
      lines_per_set = atoi(optarg);
      break;
    case 'b':
      block_bits = atoi(optarg);
      break;
    case 't':
      tracefile_path = optarg;
      break;
    default:
      print_usage(argv);
      exit(1);
    }
  }

  if (set_bits == 0 || lines_per_set == 0 || block_bits == 0 ||
      tracefile_path == NULL) {
    printf("Error: Missing required arguments.\n");
    print_usage(argv);
    exit(1);
  }

  FILE *file = fopen(tracefile_path, "r");
  if (file == NULL) {
    perror("Error opening file");
    return 1;
  }

  Cache *c = new_cache(set_bits, lines_per_set, block_bits);
  uint64_t hit_count = 0, miss_count = 0, eviction_count = 0;
  char operation;
  uint64_t address;
  uint32_t size;
  AccessStatus st;

  while (fscanf(file, " %c %lx,%d", &operation, &address, &size) == 3) {
    if (operation == 'I') {
      continue;
    }
    st = cache_upsert(c, address);
    hit_count += st.hit;
    miss_count += st.miss;
    eviction_count += st.eviction;

    if (verbose) {
      printf("%c %lx,%d ", operation, address, size);
      if (st.hit) {
        printf("hit");
      } else {
        if (st.miss)
          printf("miss");
        if (st.eviction)
          printf(" eviction");
      }
    }

    if (operation == 'M') {
      hit_count++;
      if (verbose)
        printf(" hit");
    }

    if (verbose)
      printf("\n");
  }

  fclose(file);
  printSummary(hit_count, miss_count, eviction_count);
}

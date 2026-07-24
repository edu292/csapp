#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>

#define ARRAY_COPY(src, size) array_copy((src), (size), sizeof(*(src)))

typedef void (*sort_func_t)(long *, size_t);
typedef void (*merge_func_t)(long[], long[], long[], size_t);

typedef struct {
  const char *name;
  sort_func_t func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

bool arrays_equal(const long *a, const long *b, long len) {
  for (long i = 0; i < len; i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

void *array_copy(const void *src, size_t array_length, size_t element_size) {
  if (src == NULL || array_length == 0 || element_size == 0) {
    return NULL;
  }

  size_t array_size = array_length * element_size;
  void *copy = malloc(array_size);

  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, src, array_size);

  return copy;
}

void merge_move(long src1[], long src2[], long dest[], size_t n) {
  size_t i1 = 0;
  size_t i2 = 0;
  size_t id = 0;
  while (i1 < n && i2 < n) {
    long value1 = src1[i1];
    long value2 = src2[i2];
    long take_v1 = value1 < value2;
    dest[id++] = take_v1 ? value1 : value2;
    i1 += take_v1;
    i2 += 1 - take_v1;
  }

  while (i1 < n)
    dest[id++] = src1[i1++];

  while (i2 < n)
    dest[id++] = src2[i2++];
}

void merge_jump(long src1[], long src2[], long dest[], size_t n) {
  long i1 = 0;
  long i2 = 0;
  long id = 0;
  while (i1 < n && i2 < n) {
    if (src1[i1] < src2[i2])
      dest[id++] = src1[i1++];
    else
      dest[id++] = src2[i2++];
  }

  while (i1 < n)
    dest[id++] = src1[i1++];

  while (i2 < n)
    dest[id++] = src2[i2++];
}

long *mergesort(long src[], size_t n, merge_func_t merge_func) {
  if (n == 1) {
    return src;
  }

  long half = n / 2;
  unsigned short remaining = n - half;
  long *start = ARRAY_COPY(src, half);
  long *end = ARRAY_COPY(src + half, remaining);
  merge_func(mergesort(start, half, merge_func),
             mergesort(end, remaining, merge_func), src, half);
  free(start);
  free(end);

  return src;
}

void mergesort_jump(long src[], size_t length) {
  mergesort(src, length, merge_jump);
}

void mergesort_move(long src[], size_t length) {
    mergesort(src, length, merge_move);
}

bench_result_t benchmark(sort_func_t sort_func, const long *src, long len,
                         int iterations) {
  long *working = malloc(len * sizeof(long));
  struct timespec start_t, end_t;
  unsigned int dummy;

  memcpy(working, src, len * sizeof(long));
  sort_func(working, len);

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int i = 0; i < iterations; i++) {
    memcpy(working, src, len * sizeof(long));
    sort_func(working, len);
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  free(working);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;

  unsigned long long total_cycles = end_c - start_c;
  res.cpe = (double)total_cycles / (iterations * len);

  return res;
}

int main(void) {
  algo_t algorithms[] = {{"merge_jump", mergesort_jump},
                         {"merge_move", mergesort_move}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    long input[8];
    long expected[8];
    size_t len;
  } tests[] = {{{5}, {5}, 1},
               {{2, 1}, {1, 2}, 2},
               {{1, 2, 3, 4}, {1, 2, 3, 4}, 4},
               {{4, 3, 2, 1}, {1, 2, 3, 4}, 4},
               {{3, 1, 4, 1, 5, 9, 2, 6}, {1, 1, 2, 3, 4, 5, 6, 9}, 8}};
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  printf("=== Correctness Tests ===\n");
  for (int a = 0; a < num_algos; a++) {
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
      long *buf = ARRAY_COPY(tests[i].input, tests[i].len);

      algorithms[a].func(buf, tests[i].len);

      if (arrays_equal(buf, tests[i].expected, tests[i].len)) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d\n", algorithms[a].name, i);
      }
    }
    printf("%-20s: %d/%d passed\n", algorithms[a].name, passed, num_tests);
  }

  const size_t BENCH_LEN = 2000;
  const int ITERATIONS = 1000;
  long *bench_data = malloc(BENCH_LEN * sizeof(long));

  srand(3210213);
  for (size_t i = 0; i < BENCH_LEN; i++) {
    bench_data[i] = rand() % 10000;
  }

  printf("\n=== Benchmarks (%d iterations, N=%zu) ===\n", ITERATIONS,
         BENCH_LEN);
  printf("%-20s | %-12s | %-10s\n", "Algorithm", "Time (s)", "CPE");
  printf("------------------------------------------------------\n");

  for (int a = 0; a < num_algos; a++) {
    bench_result_t res =
        benchmark(algorithms[a].func, bench_data, BENCH_LEN, ITERATIONS);
    printf("%-20s | %-12.4f | %-10.2f\n", algorithms[a].name, res.time_sec,
           res.cpe);
  }

  free(bench_data);
  return 0;
}

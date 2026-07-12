#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>

typedef void *(*memset_func_t)(void *s, int c, size_t n);

typedef struct {
  const char *name;
  memset_func_t func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

void print_buffer_snippet(const uint8_t *buf, size_t start, size_t len) {
  printf("[");
  for (size_t i = 0; i < len; i++) {
    printf("%02X%s", buf[start + i], (i == len - 1) ? "" : " ");
  }
  printf("]");
}

bench_result_t benchmark(memset_func_t func, size_t len, int iterations) {
  void *buf = malloc(len);
  struct timespec start_t, end_t;
  unsigned int dummy;

  func(buf, 0x00, len); // Warm-up

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int i = 0; i < iterations; i++) {
    func(buf, i & 0xFF, len);
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  free(buf);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;
  res.cpe = (double)(end_c - start_c) / (iterations * len);

  return res;
}

void *basic_memset(void *s, int c, size_t n) {
  size_t cnt = 0;
  unsigned char *schar = s;
  while (cnt < n) {
    *schar++ = (unsigned char)c;
    cnt++;
  }
  return s;
}

void *word_memset(void *s, int c, size_t n) {
  size_t k = sizeof(unsigned long);
  unsigned char *s_char = s;
  unsigned char c_char = (unsigned char)c;

  size_t cnt = 0;
  size_t until_aligned = (k - (uintptr_t)s % k) % k;
  if (until_aligned > n)
    until_aligned = n;
  for (; until_aligned; cnt++, until_aligned--) {
    *s_char++ = c_char;
  }

  unsigned long c_long = c_char;
  for (unsigned short b = 8; b < k * 8; b += 8)
    c_long |= (unsigned long)c_char << b;

  unsigned long *s_long = (unsigned long *)s_char;
  unsigned long limit = n > k ? (n - (k - 1)) : 0;
  for (; cnt < limit; cnt += k)
    *s_long++ = c_long;

  s_char = (unsigned char *)s_long;

  for (; cnt < n; cnt++)
    *s_char++ = c_char;

  return s;
}

int main(void) {
  algo_t algorithms[] = {{"basic memset", basic_memset},
                         {"word memset", word_memset},
                         {"stdlib memset", memset}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    size_t offset;
    size_t n;
    int c;
    const char *desc;
  } tests[] = {{0, 0, 0xFF, "Edge case: n = 0"},
               {0, 1, 0x11, "1 byte aligned"},
               {1, 1, 0x22, "1 byte unaligned (offset 1)"},
               {3, 5, 0x33, "5 bytes unaligned (offset 3)"},
               {0, 32, 0x44, "32 bytes aligned"},
               {7, 32, 0x55, "32 bytes unaligned (offset 7)"},
               {0, 64, 0x12345678, "LSB: c > 255 (should write 0x78)"},
               {0, 64, -1, "Negative value: c = -1 (should write 0xFF)"},
               {15, 128, 0x00, "128 bytes unaligned, zeroing"}};
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  const size_t BUF_SIZE = 256;
  const uint8_t GUARD_BYTE = 0xAA;

  uint8_t test_buf[BUF_SIZE];
  uint8_t expected_buf[BUF_SIZE];
  printf("=== Tests ===\n");
  for (int a = 0; a < num_algos; a++) {
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
      memset(test_buf, GUARD_BYTE, BUF_SIZE);
      memset(expected_buf, GUARD_BYTE, BUF_SIZE);

      uint8_t lsb_val = (uint8_t)(tests[i].c & 0xFF);
      for (size_t k = 0; k < tests[i].n; k++) {
        expected_buf[tests[i].offset + k] = lsb_val;
      }

      void *ret = algorithms[a].func(test_buf + tests[i].offset, tests[i].c,
                                     tests[i].n);

      if (memcmp(test_buf, expected_buf, BUF_SIZE) == 0) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d (%s)\n", algorithms[a].name, i,
               tests[i].desc);
        size_t view_start = (tests[i].offset >= 4) ? tests[i].offset - 4 : 0;
        size_t view_len = tests[i].n + 8;
        if (view_start + view_len > BUF_SIZE)
          view_len = BUF_SIZE - view_start;

        printf("  Expected: ");
        print_buffer_snippet(expected_buf, view_start, view_len);
        printf("\n  Result:   ");
        print_buffer_snippet(test_buf, view_start, view_len);
        printf("\n");
      }
    }
    printf("%-20s: %d/%d passed\n", algorithms[a].name, passed, num_tests);
  }
  const size_t BENCH_LEN = 16384;
  const int ITERATIONS = 50000;

  printf("\n=== Benchmarks (%d iterations, N=%zu bytes) ===\n", ITERATIONS,
         BENCH_LEN);
  printf("%-20s | %-12s | %-10s\n", "Algorithm", "Time (s)",
         "CPB (Cycles/Byte)");
  printf("------------------------------------------------------\n");

  for (int a = 0; a < num_algos; a++) {
    bench_result_t res = benchmark(algorithms[a].func, BENCH_LEN, ITERATIONS);
    printf("%-20s | %-12.4f | %-10.2f\n", algorithms[a].name, res.time_sec,
           res.cpe);
  }

  return 0;
}

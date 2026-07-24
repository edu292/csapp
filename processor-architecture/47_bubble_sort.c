#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void bubble_a(long *data, long count) {
  long i, last;
  for (last = count - 1; last > 0; last--) {
    for (i = 0; i < last; i++)
      if (data[i + 1] < data[i]) {
        long t = data[i + 1];
        data[i + 1] = data[i];
        data[i] = t;
      }
  }
}

void bubble_p(long *data, long count) {
  for (long *last = data + (count - 1); last != data; last--) {
    for (long *i = data; i != last; i++) {
      long current = *i;
      long next = *(i + 1);
      if (next < current) {
        *(i + 1) = current;
        *i = next;
      }
    }
  }
}

extern void bubble_p_swap(long *data, long count);

extern void bubble_p_swap_one_cmov(long *data, long count);

int arrays_equal(const long *a, const long *b, long len) {
  for (long i = 0; i < len; i++) {
    if (a[i] != b[i])
      return 0;
  }

  return 1;
}

double benchmark(void (*sort_func)(long *, long), const long *src, long len,
                 int iterations) {
  long *working = malloc(len * sizeof(long));
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < iterations; i++) {
    memcpy(working, src, len * sizeof(long));
    sort_func(working, len);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  free(working);
  return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void) {
  struct {
    long input[8];
    long expected[8];
    size_t len;
  } tests[] = {{{5}, {5}, 1},                   // Edge case: Single element
               {{2, 1}, {1, 2}, 2},             // Reverse sorted pair
               {{1, 2, 3, 4}, {1, 2, 3, 4}, 4}, // Already sorted
               {{4, 3, 2, 1}, {1, 2, 3, 4}, 4}, // Strictly reversed
               {{3, 1, 4, 1, 5, 9, 2, 6}, {1, 1, 2, 3, 4, 5, 6, 9}, 8}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed_a = 0, passed_p = 0, passed_p_swap = 0, passed_p_swap_one_cmov = 0;

  for (int i = 0; i < num_tests; i++) {
    long buf_a[8], buf_p[8], buf_p_swap[8], buf_p_swap_one_cmov[8];
    memcpy(buf_a, tests[i].input, tests[i].len * sizeof(long));
    memcpy(buf_p, tests[i].input, tests[i].len * sizeof(long));
    memcpy(buf_p_swap, tests[i].input, tests[i].len * sizeof(long));
    memcpy(buf_p_swap_one_cmov, tests[i].input, tests[i].len * sizeof(long));

    bubble_a(buf_a, tests[i].len);
    bubble_p(buf_p, tests[i].len);
    bubble_p_swap(buf_p_swap, tests[i].len);
    bubble_p_swap_one_cmov(buf_p_swap_one_cmov, tests[i].len);

    if (arrays_equal(buf_a, tests[i].expected, tests[i].len))
      passed_a++;
    else
      printf("FAIL: bubble_a | Test %d\n", i);

    if (arrays_equal(buf_p, tests[i].expected, tests[i].len))
      passed_p++;
    else
      printf("FAIL: bubble_p | Test %d\n", i);

    if (arrays_equal(buf_p_swap, tests[i].expected, tests[i].len))
      passed_p_swap++;
    else
      printf("FAIL: bubble_p_swap | Test %d\n", i);

    if (arrays_equal(buf_p_swap_one_cmov, tests[i].expected, tests[i].len))
      passed_p_swap_one_cmov++;
    else
      printf("FAIL: bubble_p_swap_one_cmov | Test %d\n", i);
  }

  printf("Correctness: bubble_a (%d/%d), bubble_p (%d/%d), bubble_p_swap "
         "(%d/%d), bubble_p_swap_one_cmov (%d/%d)\n",
         passed_a, num_tests, passed_p, num_tests, passed_p_swap, num_tests,
         passed_p_swap_one_cmov, num_tests);

  const size_t BENCH_LEN = 2000;
  const int ITERATIONS = 100;
  long *bench_data = malloc(BENCH_LEN * sizeof(long));

  srand(3210213);
  for (size_t i = 0; i < BENCH_LEN; i++) {
    bench_data[i] = rand() % 10000;
  }

  printf("\nBenchmarking over %d iterations (Array size: %zu)...\n", ITERATIONS,
         BENCH_LEN);

  double time_a = benchmark(bubble_a, bench_data, BENCH_LEN, ITERATIONS);
  printf("bubble_a:               %.4f seconds total\n", time_a);

  double time_p = benchmark(bubble_p, bench_data, BENCH_LEN, ITERATIONS);
  printf("bubble_p:               %.4f seconds total\n", time_p);

  double time_p_swap =
      benchmark(bubble_p_swap, bench_data, BENCH_LEN, ITERATIONS);
  printf("bubble_p_swap:          %.4f seconds total\n", time_p_swap);

  double time_p_swap_one_cmov =
      benchmark(bubble_p_swap_one_cmov, bench_data, BENCH_LEN, ITERATIONS);
  printf("bubble_p_swap_one_cmov: %.4f seconds total\n", time_p_swap_one_cmov);

  free(bench_data);
  return 0;
}

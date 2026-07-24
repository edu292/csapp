#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>

typedef void (*psum_func_t)(float *p, const float *a, size_t len);

typedef struct {
  const char *name;
  psum_func_t func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

int floats_equal(const float *a, const float *b, size_t len) {
  const float EPSILON = 1e-5f;
  for (size_t i = 0; i < len; i++) {
    if (fabsf(a[i] - b[i]) > EPSILON)
      return 0;
  }
  return 1;
}

void print_float_array(const float *arr, size_t len) {
  printf("[");
  for (size_t i = 0; i < len; i++) {
    printf("%.4f%s", arr[i], (i == len - 1) ? "" : ", ");
  }
  printf("]");
}

bench_result_t benchmark(psum_func_t psum_func, const float *src, long len,
                         int iterations) {
  float *dest = malloc(len * sizeof(float));
  struct timespec start_t, end_t;
  unsigned int dummy;

  psum_func(dest, src, len);

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int i = 0; i < iterations; i++) {
    psum_func(dest, src, len);
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  free(dest);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;

  unsigned long long total_cycles = end_c - start_c;
  res.cpe = (double)total_cycles / (iterations * len);

  return res;
}

void psum(float p[], const float a[], size_t len) {
  p[0] = a[0];
  for (size_t i = 1; i < len; i++)
    p[i] = p[i - 1] + a[i];
}

void psum_no_dd(float p[], const float a[], size_t len) {
  float sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum += a[i];
    p[i] = sum;
  }
}

void psum_6x1(float p[], const float a[], size_t len) {
  size_t i = 0;
  float s0= 0.0;
  for (; i + 3 < len; i += 4) {
    float a01 = a[i] + a[i + 1];
    float a23 = a[i + 2] + a[i + 3];
    float a012 = a01 + a[i + 2];

    p[i] = s0 + a[i];
    p[i + 1] = s0 + a01;
    p[i + 2] = s0 + a012;

    s0 += (a01 + a23);
    p[i+3] = s0;
  }

  for (; i < len; i++) {
    s0 += a[i];
    p[i] = s0;
  }
}

int main(void) {
  algo_t algorithms[] = {
      {"psum", psum}, {"psum_no_dd", psum_no_dd}, {"psum_6x1", psum_6x1}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    float input[5];
    float expected[5];
    size_t len;
  } tests[] = {
      {{1.0f}, {1.0f}, 1},
      {{1.5f, 2.0f}, {1.5f, 3.5f}, 2},
      {{1.0f, 2.0f, 3.0f, 4.0f}, {1.0f, 3.0f, 6.0f, 10.0f}, 4},
      {{0.5f, -0.5f, 2.0f, -1.0f}, {0.5f, 0.0f, 2.0f, 1.0f}, 4},
      {{1.1f, 2.2f, 3.3f, 4.4f, 5.5f}, {1.1f, 3.3f, 6.6f, 11.0f, 16.5f}, 5}};
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  printf("=== Tests ===\n");
  for (int a = 0; a < num_algos; a++) {
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
      float out_buf[5] = {0};

      algorithms[a].func(out_buf, tests[i].input, tests[i].len);

      if (floats_equal(out_buf, tests[i].expected, tests[i].len)) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d\n", algorithms[a].name, i);
        printf("  Expected: ");
        print_float_array(tests[i].expected, tests[i].len);
        printf("\n  Result:   ");
        print_float_array(out_buf, tests[i].len);
        printf("\n");
      }
    }
    printf("%-20s: %d/%d passed\n", algorithms[a].name, passed, num_tests);
  }

  const size_t BENCH_LEN = 10000;
  const int ITERATIONS = 10000;
  float *bench_data = malloc(BENCH_LEN * sizeof(float));

  srand(3210213);
  for (size_t i = 0; i < BENCH_LEN; i++) {
    bench_data[i] = (float)rand() / (float)RAND_MAX;
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

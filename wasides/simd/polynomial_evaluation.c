#include <bits/time.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <x86intrin.h>

#define VBYTES 32
#define VSIZE (VBYTES / sizeof(double))

typedef double vec_t __attribute__((vector_size(VBYTES), aligned(1)));

typedef double (*poly_func_t)(const double *a, double x, long degree);

int doubles_equal(double a, double b) { return fabs(a - b) < 1e-5; }

typedef struct {
  const char *name;
  poly_func_t func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

bench_result_t benchmark(poly_func_t func, const double *a, double x,
                         long degree, int iterations) {
  struct timespec start_t, end_t;
  unsigned int dummy;
  volatile double result;

  result = func(a, x, degree);

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int i = 0; i < iterations; i++) {
    result = func(a, x, degree);
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;

  unsigned long long total_cycles = end_c - start_c;
  res.cpe = (double)total_cycles / ((long)iterations * degree);

  return res;
}

double scalar(const double a[], double x, long degree) {
  long i;
  double result = a[0];
  double xpwr = x;
  for (i = 1; i <= degree; i++) {
    result += a[i] * xpwr;
    xpwr = x * xpwr;
  }

  return result;
}

double scalar_6x6(const double a[], double x, long degree) {
  double r0 = 0.0, r1 = 0.0, r2 = 0.0, r3 = 0.0, r4 = 0.0, r5 = 0.0;

  double xp0 = 1.0;
  double xp1 = x;
  double xp2 = x * x;
  double xp3 = xp2 * x;
  double xp4 = xp2 * xp2;
  double xp5 = xp4 * x;

  double x_step = xp3 * xp3;

  long i = 0;

  for (; i <= degree - 5; i += 6) {
    r0 += a[i] * xp0;
    r1 += a[i + 1] * xp1;
    r2 += a[i + 2] * xp2;
    r3 += a[i + 3] * xp3;
    r4 += a[i + 4] * xp4;
    r5 += a[i + 5] * xp5;

    xp0 *= x_step;
    xp1 *= x_step;
    xp2 *= x_step;
    xp3 *= x_step;
    xp4 *= x_step;
    xp5 *= x_step;
  }

  double result = r0 + r1 + r2 + r3 + r4 + r5;

  for (; i <= degree; i++) {
    result += a[i] * xp0;
    xp0 *= x;
  }

  return result;
}

double scalar_homer(const double a[], double x, long degree) {
  double result = a[degree];
  for (long i = degree - 1; i >= 0; i--)
    result = a[i] + x * result;

  return result;
}

double scalar_homer_6x6(const double a[], double x, long degree) {
  double r0 = 0.0, r1 = 0.0, r2 = 0.0, r3 = 0.0, r4 = 0.0, r5 = 0.0;

  double x6 = x * x * x * x * x * x;

  long i = degree;

  while (i >= 0 && (i % 6) != 5) {
    if (i % 6 == 4)
      r4 = a[i] + x6 * r4;
    else if (i % 6 == 3)
      r3 = a[i] + x6 * r3;
    else if (i % 6 == 2)
      r2 = a[i] + x6 * r2;
    else if (i % 6 == 1)
      r1 = a[i] + x6 * r1;
    else if (i % 6 == 0)
      r0 = a[i] + x6 * r0;
    i--;
  }

  for (; i >= 5; i -= 6) {
    r5 = a[i] + x6 * r5;
    r4 = a[i - 1] + x6 * r4;
    r3 = a[i - 2] + x6 * r3;
    r2 = a[i - 3] + x6 * r2;
    r1 = a[i - 4] + x6 * r1;
    r0 = a[i - 5] + x6 * r0;
  }

  return r0 + x * (r1 + x * (r2 + x * (r3 + x * (r4 + x * r5))));
}

double scalar_homer_6x1a(const double a[], double x, long degree) {
  double result = 0.0;
  double x2 = x * x;
  double x4 = x2 * x2;
  double x6 = x4 * x2;
  long i = degree;

  while (i >= 0 && (i % 6) != 5) {
    result = a[i] + x * result;
    i--;
  }

  for (; i >= 5; i -= 6) {
    result = (a[i - 5] + a[i - 4] * x) + x2 * (a[i - 3] + a[i - 2] * x) +
             x4 * (a[i - 1] + a[i] * x) + x6 * result;
  }

  return result;
}

double simd(const double a[], double x, long degree) {
  double result = 0.0;
  vec_t accum = {0};
  vec_t x_vec;
  x_vec[0] = 1.0;
  x_vec[1] = x;
  x_vec[2] = x * x;
  x_vec[3] = x_vec[2] * x;
  double x_step = x_vec[2] * x_vec[2];
  size_t len = degree + 1;

  const double *data = a;
  for (; len >= VSIZE; len -= VSIZE) {
    accum += *(vec_t *)data * x_vec;
    x_vec *= x_step;
    data += VSIZE;
  }

  for (; len; len--) {
    result += *data++ * x_vec[0];
    x_vec[0] *= x;
  }

  for (int i = 0; i < VSIZE; i++) {
    result += accum[i];
  }

  return result;
}

double simd_homer(const double a[], double x, long degree) {
  double x2 = x * x;
  double x4 = x2 * x2;
  double x8 = x4 * x4;

  vec_t x16 = {x8 * x8, x8 * x8, x8 * x8, x8 * x8};

  vec_t r0 = {0, 0, 0, 0};
  vec_t r1 = {0, 0, 0, 0};
  vec_t r2 = {0, 0, 0, 0};
  vec_t r3 = {0, 0, 0, 0};

  long i = degree;
  double result = 0.0;

  while (i >= 0 && (i % 16) != 15) {
    result = a[i] + x * result;
    i--;
  }

  if (i >= 15) {
    const vec_t *v_ptr = (const vec_t *)&a[i - 15];

    vec_t chunk3 = v_ptr[3];
    chunk3[3] +=
        x * result;

    r3 = chunk3;
    r2 = v_ptr[2];
    r1 = v_ptr[1];
    r0 = v_ptr[0];

    i -= 16;

    for (; i >= 15; i -= 16) {
      v_ptr = (const vec_t *)&a[i - 15];
      r3 = v_ptr[3] + x16 * r3;
      r2 = v_ptr[2] + x16 * r2;
      r1 = v_ptr[1] + x16 * r1;
      r0 = v_ptr[0] + x16 * r0;
    }

    vec_t x4_v = {x4, x4, x4, x4};
    vec_t x8_v = {x8, x8, x8, x8};
    vec_t x12_v = x4_v * x8_v;

    vec_t combined = r0 + x4_v * r1 + x8_v * r2 + x12_v * r3;

    vec_t x_lanes = {1.0, x, x2, x2 * x};
    combined = combined * x_lanes;

    result = combined[0] + combined[1] + combined[2] + combined[3];
  }

  return result;
}

int main(void) {
  algo_t algorithms[] = {{"scalar", scalar},
                         {"scalar homer", scalar_homer},
                         {"scalar homer 6x6", scalar_homer_6x6},
                         {"scalar homer 6x1a", scalar_homer_6x1a},
                         {"scalar 6x6", scalar_6x6},
                         {"simd", simd},
                         {"simd homer", simd_homer}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    double a[16];
    double x;
    long degree;
    double expected;
  } tests[] = {
      // P(x) = 5
      {{5.0}, 2.0, 0, 5.0},
      // P(x) = 2 + 3x | x=2 -> 2 + 6 = 8
      {{2.0, 3.0}, 2.0, 1, 8.0},
      // P(x) = 1 + 2x + 3x^2 | x=2 -> 1 + 4 + 12 = 17
      {{1.0, 2.0, 3.0}, 2.0, 2, 17.0},
      // P(x) = -1 + x - x^2 | x=3 -> -1 + 3 - 9 = -7
      {{-1.0, 1.0, -1.0}, 3.0, 2, -7.0},
      // P(x) = 0 + 0x + 0x^2 + x^3 | x=4 -> 64
      {{0.0, 0.0, 0.0, 1.0}, 4.0, 3, 64.0},
      // Degree 4 (5 elements): 1 + x + x^2 + x^3 + x^4 | x=2 -> 31
      {{1.0, 1.0, 1.0, 1.0, 1.0}, 2.0, 4, 31.0},

      // Degree 7 (8 elements, tests exactly 2x4 unroll): x^7 | x=2 -> 128
      {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0}, 2.0, 7, 128.0},

      // Degree 9 (10 elements, tests unroll tail logic): Alternating signs |
      // x=2 -> -341
      {{1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}, 2.0, 9, -341.0},

      // Degree 15 (16 elements, tests exactly 4x4 or 2x8 unroll): Sum of 2^i |
      // x=2 -> 65535
      {{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0},
       2.0,
       15,
       65535.0}};
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  printf("=== Tests ===\n");
  for (int alg = 0; alg < num_algos; alg++) {
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
      double out =
          algorithms[alg].func(tests[i].a, tests[i].x, tests[i].degree);

      if (doubles_equal(out, tests[i].expected)) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d\n", algorithms[alg].name, i);
        printf("  Expected: %.5f\n", tests[i].expected);
        printf("  Result:   %.5f\n", out);
      }
    }
    printf("%-20s: %d/%d passed\n", algorithms[alg].name, passed, num_tests);
  }

  const long BENCH_DEGREE = 2000;
  const int ITERATIONS = 50000;
  double *bench_coeffs = malloc((BENCH_DEGREE + 1) * sizeof(double));

  srand(3210213);
  for (long i = 0; i <= BENCH_DEGREE; i++) {
    bench_coeffs[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
  }
  double bench_x = 0.999;

  printf("\n=== Benchmarks (%d iterations, Degree=%ld) ===\n", ITERATIONS,
         BENCH_DEGREE);
  printf("%-20s | %-12s | %-10s\n", "Algorithm", "Time (s)", "CPE");
  printf("------------------------------------------------------\n");

  for (int alg = 0; alg < num_algos; alg++) {
    bench_result_t res = benchmark(algorithms[alg].func, bench_coeffs, bench_x,
                                   BENCH_DEGREE, ITERATIONS);
    printf("%-20s | %-12.4f | %-10.2f\n", algorithms[alg].name, res.time_sec,
           res.cpe);
  }

  free(bench_coeffs);
  return 0;
}

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>

#ifndef DATA_T
#define DATA_T double
#endif

typedef DATA_T data_t;

typedef struct {
  size_t length;
  data_t *data;
} vec_t, *vec_ptr;

size_t vec_length(vec_ptr vec) { return vec->length; }
data_t *get_vec_start(vec_ptr vec) { return vec->data; }

typedef void (*inner_prod_f)(vec_ptr u, vec_ptr v, data_t *dest);

typedef struct {
  const char *name;
  inner_prod_f func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

#define IS_EQUAL(a, b)                                                         \
  _Generic((a),                                                                \
      double: fabs(a - b) < 1e-5,                                              \
      float: fabsf(a - b) < 1e-5,                                              \
      default: a == b)

void sequential(vec_ptr u, vec_ptr v, data_t *dest) {
  long i;
  long length = vec_length(u);
  data_t *udata = get_vec_start(u);
  data_t *vdata = get_vec_start(v);
  data_t sum = (data_t)0;

  for (i = 0; i < length; i++) {
    sum += udata[i] * vdata[i];
  }
  *dest = sum;
}

void unrolled_6x1(vec_ptr u, vec_ptr v, data_t *dest) {
  long length = vec_length(u);
  data_t *udata = get_vec_start(u);
  data_t *vdata = get_vec_start(v);

  long i = 0;
  data_t sum = (data_t)0;
  for (; i < length - 5; i += 6) {
    sum += (((((sum + udata[i] * vdata[i]) + udata[i + 1] * vdata[i + 1]) +
              udata[i + 2] * vdata[i + 2]) +
             udata[i + 3] * vdata[i + 3]) +
            udata[i + 4] * vdata[i + 4]) +
           udata[i + 5] * vdata[i + 5];
  }

  for (; i < length; i++) {
    sum += udata[i] * vdata[i];
  }

  *dest = sum;
}

void unrolled_6x6(vec_ptr u, vec_ptr v, data_t *dest) {
  long length = vec_length(u);
  data_t *udata = get_vec_start(u);
  data_t *vdata = get_vec_start(v);

  long i = 0;
  data_t sum1 = (data_t)0;
  data_t sum2 = (data_t)0;
  data_t sum3 = (data_t)0;
  data_t sum4 = (data_t)0;
  data_t sum5 = (data_t)0;
  data_t sum6 = (data_t)0;
  for (; i < length - 5; i += 6) {
    sum1 += udata[i] * vdata[i];
    sum2 += udata[i + 1] * vdata[i + 1];
    sum3 += udata[i + 2] * vdata[i + 2];
    sum4 += udata[i + 3] * vdata[i + 3];
    sum5 += udata[i + 4] * vdata[i + 4];
    sum6 += udata[i + 5] * vdata[i + 5];
  }

  data_t sum = sum1 + sum2 + sum3 + sum4 + sum5 + sum6;

  for (; i < length; i++) {
    sum += udata[i] * vdata[i];
  }

  *dest = sum;
}

void unrolled_6x1a(vec_ptr u, vec_ptr v, data_t *dest) {
  long length = vec_length(u);
  data_t *udata = get_vec_start(u);
  data_t *vdata = get_vec_start(v);

  long i = 0;
  data_t sum = (data_t)0;
  for (; i < length - 5; i += 6) {
    sum += ((((udata[i] * vdata[i] + udata[i + 1] * vdata[i + 1]) +
              udata[i + 2] * vdata[i + 2]) +
             udata[i + 3] * vdata[i + 3]) +
            udata[i + 4] * vdata[i + 4]) +
           udata[i + 5] * vdata[i + 5];
  }

  for (; i < length; i++) {
    sum += udata[i] * vdata[i];
  }

  *dest = sum;
}

bench_result_t benchmark(inner_prod_f func, vec_ptr u, vec_ptr v,
                         int iterations) {
  data_t result;
  struct timespec start_t, end_t;
  unsigned int dummy;

  func(u, v, &result);

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int i = 0; i < iterations; i++) {
    func(u, v, &result);
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;
  res.cpe = (double)(end_c - start_c) / (iterations * vec_length(u));

  return res;
}

int main(void) {
  algo_t algorithms[] = {{"sequential", sequential},
                         {"unrolled 6x1", unrolled_6x1},
                         {"unrolled 6x6", unrolled_6x6},
                         {"unrolled 6x1a", unrolled_6x6}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    data_t u[4];
    data_t v[4];
    data_t expected;
    size_t len;
  } tests[] = {{{1, 2, 3, 4}, {1, 1, 1, 1}, 10, 4},
               {{1, 2, 3, 4}, {2, 2, 2, 2}, 20, 4},
               {{0, 0, 0, 0}, {5, 6, 7, 8}, 0, 4},
               {{-1, 2, -3, 4}, {1, 1, 1, 1}, 2, 4}};
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  printf("=== Correctness Tests ===\n");
  for (int a = 0; a < num_algos; a++) {
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
      vec_t vec_u = {tests[i].len, tests[i].u};
      vec_t vec_v = {tests[i].len, tests[i].v};
      data_t out = 0;

      algorithms[a].func(&vec_u, &vec_v, &out);

      if (IS_EQUAL(out, tests[i].expected)) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d\n", algorithms[a].name, i);

        printf("  Expected: %.2f\n", (double)tests[i].expected);
        printf("  Result:   %.2f\n", (double)out);
      }
    }
    printf("%-20s: %d/%d passed\n", algorithms[a].name, passed, num_tests);
  }

  const size_t BENCH_LEN = 10000;
  const int ITERATIONS = 10000;

  data_t *data_u = malloc(BENCH_LEN * sizeof(data_t));
  data_t *data_v = malloc(BENCH_LEN * sizeof(data_t));

  srand(3210213);
  for (size_t i = 0; i < BENCH_LEN; i++) {
    data_u[i] = (data_t)(rand() % 100);
    data_v[i] = (data_t)(rand() % 100);
  }

  vec_t bench_u = {BENCH_LEN, data_u};
  vec_t bench_v = {BENCH_LEN, data_v};

  printf("\n=== Benchmarks (%d iterations, N=%zu) ===\n", ITERATIONS,
         BENCH_LEN);
  printf("%-20s | %-12s | %-10s\n", "Algorithm", "Time (s)", "CPE");
  printf("------------------------------------------------------\n");

  for (int a = 0; a < num_algos; a++) {
    bench_result_t res =
        benchmark(algorithms[a].func, &bench_u, &bench_v, ITERATIONS);
    printf("%-20s | %-12.4f | %-10.2f\n", algorithms[a].name, res.time_sec,
           res.cpe);
  }

  free(data_u);
  free(data_v);
  return 0;
}

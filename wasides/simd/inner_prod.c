#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <x86intrin.h>

#define VBYTES 32

#ifndef DATA_T
#define DATA_T double
#endif

typedef DATA_T data_t;

#define VSIZE (VBYTES / sizeof(data_t))

#ifndef IDENT
#define IDENT 0
#endif

#ifndef OP
#define OP +
#endif

#define IS_EQUAL(a, b)                                                         \
  _Generic((a),                                                                \
      double: fabs((double)(a) - (double)(b)) < 1e-5,                          \
      float: fabsf((float)(a) - (float)(b)) < 1e-5f,                           \
      default: (a) == (b))

typedef data_t vec_t __attribute__((vector_size(VBYTES), aligned(1)));

typedef struct {
  size_t length;
  data_t *data;
} vector, *vec_ptr;

typedef void (*inner_prod_f)(vec_ptr u, vec_ptr v, data_t *dest);

typedef struct {
  const char *name;
  inner_prod_f func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

size_t vec_length(vec_ptr vec) { return vec->length; }
data_t *get_vec_start(vec_ptr vec) { return vec->data; }

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

void scalar(vec_ptr u, vec_ptr v, data_t *dest) {
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

void simd(vec_ptr u, vec_ptr v, data_t *dest) {
  vec_t sum = {0};
  data_t *udata = get_vec_start(u);
  data_t *vdata = get_vec_start(v);
  size_t len = vec_length(u);
  data_t result = 0;

  for (; len >= VSIZE; len -= VSIZE) {
    sum += *((vec_t *)udata) * *((vec_t *)vdata);
    udata += VSIZE;
    vdata += VSIZE;
  }

  for (; len; len--) {
    result += *vdata++ * *udata++;
  }

  for (int i = 0; i < VSIZE; i++) {
    result += sum[i];
  }

  *dest = result;
}

void simd_4x4(vec_ptr u, vec_ptr v, data_t *dest) {
  vec_t sum[4] = {0, 0, 0, 0};
  size_t len = vec_length(u);
  data_t result = 0;

  vec_t *vec_u = (vec_t *)get_vec_start(u);
  vec_t *vec_v = (vec_t *)get_vec_start(v);
  for (; len >= VSIZE * 4; len -= VSIZE * 4) {
    sum[0] += *vec_u * *vec_v;
    sum[1] += *(vec_u + 1) * *(vec_v + 1);
    sum[2] += *(vec_u + 2) * *(vec_v + 2);
    sum[3] += *(vec_u + 3) * *(vec_v + 3);
    vec_u += 4;
    vec_v += 4;
  }

  data_t *udata = (data_t *)vec_u;
  data_t *vdata = (data_t *)vec_v;

  for (; len; len--) {
    result += *vdata++ * *udata++;
  }

  for (int i = 0; i < 4; i++) {
    vec_t current = sum[i];
    for (int j = 0; j < VSIZE; j++) {
      result += current[i];
    }
  }

  *dest = result;
}

void simd_4x1a(vec_ptr u, vec_ptr v, data_t *dest) {
  vec_t sum = {0};
  size_t len = vec_length(u);
  data_t result = 0;

  vec_t *vec_u = (vec_t *)get_vec_start(u);
  vec_t *vec_v = (vec_t *)get_vec_start(v);
  for (; len >= VSIZE * 4; len -= VSIZE * 4) {
    sum += *vec_u * *vec_v + *(vec_u + 1) * *(vec_v + 1) +
           *(vec_u + 2) * *(vec_v + 2) + *(vec_u + 3) * *(vec_v + 3);
    vec_u += 4;
    vec_v += 4;
  }

  data_t *udata = (data_t *)vec_u;
  data_t *vdata = (data_t *)vec_v;

  for (; len; len--) {
    result += *vdata++ * *udata++;
  }

  for (int i = 0; i < 4; i++) {
    result += sum[i];
  }

  *dest = result;
}

int main(void) {
  algo_t algorithms[] = {{"scalar", scalar},
                         {"simd", simd},
                         {"simd4x4", simd_4x4},
                         {"simd_4x1a", simd_4x1a}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    data_t u[8];
    data_t v[8];
    size_t len;
  } tests[] = {
      {{1, 2, 3, 4}, {1, 1, 1, 1}, 4},
      {{1, 2, 3, 4}, {2, 2, 2, 2}, 4},
      {{0, 0, 0, 0}, {5, 6, 7, 8}, 4},
      {{-1, 2, -3, 4}, {1, 1, 1, 1}, 4},

      {{}, {}, 0},    // Zero length / empty vector
      {{5}, {10}, 1}, // Single element
      {{1, 2, 3},
       {4, 5, 6},
       3}, // Odd length (tests unrolling / SIMD tail handling)
      {{1, 2, 3, 4, 5},
       {1, 1, 1, 1, 1},
       5}, // Length > SIMD width but not a multiple

      // 3. Mathematical Edge Cases
      {{1, 0, -1, 0},
       {0, 1, 0, 1},
       4}, // Orthogonal vectors (expected dot product = 0)
      {{-2, -3, -4, -5},
       {-1, -2, -3, -4},
       4}, // All negative inputs (tests sign handling in ops)

      // 4. Larger vector (tests accumulator register stability across
      // iterations)
      {{1, 2, 3, 4, 5, 6, 7, 8}, {1, 1, 1, 1, 1, 1, 1, 1}, 8},
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);

  printf("=== Tests ===\n");
  for (int a = 0; a < num_algos; a++) {
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
      vector vec_u = {tests[i].len, tests[i].u};
      vector vec_v = {tests[i].len, tests[i].v};
      data_t out = 0;
      data_t expected = 0;
      scalar(&vec_u, &vec_v, &expected);

      algorithms[a].func(&vec_u, &vec_v, &out);

      if (IS_EQUAL(out, expected)) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d\n", algorithms[a].name, i);

        printf("  Expected: %.2f\n", (double)expected);
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

  vector bench_u = {BENCH_LEN, data_u};
  vector bench_v = {BENCH_LEN, data_v};

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

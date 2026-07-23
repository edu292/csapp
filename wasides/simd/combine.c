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

typedef data_t vec_t __attribute__((vector_size(VBYTES)));

typedef struct {
  size_t length;
  data_t *data;
} vector, *vec_ptr;

typedef void (*combine_f)(vec_ptr v, data_t *dest);

typedef struct {
  const char *name;
  combine_f func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
} bench_result_t;

size_t vec_length(vec_ptr vec) { return vec->length; }
data_t *get_vec_start(vec_ptr vec) { return vec->data; }

void scalar(vec_ptr v, data_t *dest) {
  size_t len = vec_length(v);
  data_t *data = get_vec_start(v);
  data_t result = IDENT;
  for (size_t i = 0; i < len; i++) {
    result = result OP data[i];
  }

  *dest = result;
}

void simd(vec_ptr v, data_t *dest) {
  long i;
  vec_t accum;
  data_t *data = get_vec_start(v);
  int cnt = vec_length(v);
  data_t result = IDENT;

  for (i = 0; i < VSIZE; i++)
    accum[i] = IDENT;

  while ((((size_t)data) % VBYTES) != 0 && cnt) {
    result = result OP * data++;
    cnt--;
  }

  while (cnt >= VSIZE) {
    vec_t chunk = *((vec_t *)data);
    accum = accum OP chunk;
    data += VSIZE;
    cnt -= VSIZE;
  }

  while (cnt) {
    result = result OP * data++;
    cnt--;
  }

  for (i = 0; i < VSIZE; i++)
    result = result OP accum[i];

  *dest = result;
}

void simd_4x4(vec_ptr v, data_t *dest) {
  long i;
  vec_t accum0, accum1, accum2, accum3;
  data_t *data = get_vec_start(v);
  int cnt = vec_length(v);
  data_t result = IDENT;

  for (i = 0; i < VSIZE; i++) {
    accum0[i] = IDENT;
    accum1[i] = IDENT;
    accum2[i] = IDENT;
    accum3[i] = IDENT;
  }

  for (; (((size_t)data) % VBYTES) != 0 && cnt; cnt--) {
    result = result OP * data++;
  }

  vec_t *vdata = (vec_t *)data;
  for (; cnt >= VSIZE * 4; cnt -= 4 * VSIZE) {
    accum0 = accum0 OP * vdata++;
    accum1 = accum1 OP * vdata++;
    accum2 = accum2 OP * vdata++;
    accum3 = accum3 OP * vdata++;
  }

  data = (data_t *)vdata;

  for (; cnt; cnt--)
    result = result OP * data++;

  for (i = 0; i < VSIZE; i++)
    result = result OP accum0[i] OP accum1[i] OP accum2[i] OP accum3[i];

  *dest = result;
}

void simd_4x1a(vec_ptr v, data_t *dest) {
  long i;
  vec_t accum;
  data_t *data = get_vec_start(v);
  int cnt = vec_length(v);
  data_t result = IDENT;

  for (i = 0; i < VSIZE; i++) {
    accum[i] = IDENT;
  }

  for (; (((size_t)data) % VBYTES) != 0 && cnt; cnt--) {
    result = result OP * data++;
  }

  vec_t *vdata = (vec_t *)data;
  for (; cnt >= VSIZE * 4; cnt -= 4 * VSIZE) {
    accum = accum OP (* vdata OP * (vdata + 1)) OP (* (vdata + 2) OP * (vdata + 3));
    vdata += 4;
  }

  for (; cnt; cnt--)
    result = result OP * data++;

  for (i = 0; i < VSIZE; i++)
    result = result OP accum[i];

  *dest = result;
}

bench_result_t benchmark(combine_f func, vec_ptr v, int iterations) {
  data_t result;
  struct timespec start_t, end_t;
  unsigned int dummy;

  func(v, &result);

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int i = 0; i < iterations; i++) {
    func(v, &result);
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;
  res.cpe = (double)(end_c - start_c) / (iterations * vec_length(v));

  return res;
}

int main(void) {
  algo_t algorithms[] = {{"scalar", scalar},
                         {"simd", simd},
                         {"simd 4x4", simd_4x4},
                         {"simd 4x1a", simd_4x1a}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  struct {
    data_t v[16];
    size_t len;
  } raw_tests[] = {{{}, 0},
                   {{5}, 1},
                   {{1, 2, 3, 4}, 4},
                   {{2, 2, 2, 2}, 4},
                   {{1, 1, 1, 1, 1, 1, 1, 1}, 8},
                   {{1, 2, 3, 4, 5, 6, 7, 8, 9}, 9},
                   {{2, 3, 1, 4, 2, 1, 3, 2, 1, 1, 2, 3, 4, 1, 2, 1}, 16},
                   {{-1, 2, -3, 4}, 4}};
  int num_tests = sizeof(raw_tests) / sizeof(raw_tests[0]);

  printf("=== Tests ===\n");
  for (int a = 0; a < num_algos; a++) {
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
      vector vec_v = {raw_tests[i].len, raw_tests[i].v};
      data_t out = 0;
      data_t expected = 0;

      scalar(&vec_v, &expected);
      algorithms[a].func(&vec_v, &out);

      if (IS_EQUAL(out, expected)) {
        passed++;
      } else {
        printf("FAIL: %s | Test %d (len=%zu)\n", algorithms[a].name, i,
               raw_tests[i].len);
        printf("  Expected: %.2f\n", (double)expected);
        printf("  Result:   %.2f\n", (double)out);
      }
    }
    printf("%-20s: %d/%d passed\n", algorithms[a].name, passed, num_tests);
  }

  const size_t BENCH_LEN = 10000;
  const int ITERATIONS = 10000;

  data_t *data_v;
  if (posix_memalign((void **)&data_v, VBYTES, BENCH_LEN * sizeof(data_t)) !=
      0) {
    perror("Allocation failed");
    return 1;
  }

  srand(3210213);
  for (size_t i = 0; i < BENCH_LEN; i++) {
    data_v[i] = (data_t)((rand() % 3) + 1);
  }

  vector bench_v = {BENCH_LEN, data_v};

  printf("\n=== Benchmarks (%d iterations, N=%zu) ===\n", ITERATIONS,
         BENCH_LEN);
  printf("%-20s | %-12s | %-10s\n", "Algorithm", "Time (s)", "CPE");
  printf("------------------------------------------------------\n");

  for (int a = 0; a < num_algos; a++) {
    bench_result_t res = benchmark(algorithms[a].func, &bench_v, ITERATIONS);
    printf("%-20s | %-12.4f | %-10.2f\n", algorithms[a].name, res.time_sec,
           res.cpe);
  }

  free(data_v);
  return 0;
}

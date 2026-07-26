#include "fcyc.h"
#include "clock.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>
#include <x86intrin.h>

#define K 3
#define MAXSAMPLES 20
#define EPSILON 0.01

static int kbest = K, compensate = 0, clear_cache = 0, maxsamples = MAXSAMPLES;
static double epsilon = EPSILON;
static int cache_bytes = (1 << 19), cache_block = 32;

static int *cache_buf = NULL;
static double *values = NULL;
static int samplecount = 0;
static volatile int sink = 0;

static void clear() {
  int x = sink, incr = cache_block / sizeof(int);
  if (incr <= 0)
    incr = 1;

  if (!cache_buf) {
    /* FIX: malloc replaced by calloc to prevent reading from unmapped
     * demand-paged memory */
    cache_buf = calloc(1, cache_bytes);
    if (!cache_buf)
      exit(1);
  }

  int *cptr = cache_buf, *cend = cptr + (cache_bytes / sizeof(int));
  while (cptr < cend) {
    x += *cptr;
    cptr += incr;
  }
  sink = x;
}

static void init_sampler() {
  if (values)
    free(values);
  values = calloc(kbest, sizeof(double));
  samplecount = 0;
}

static void add_sample(double val) {
  int pos = 0;
  if (samplecount < kbest) {
    pos = samplecount;
    values[pos] = val;
  } else if (val < values[kbest - 1]) {
    pos = kbest - 1;
    values[pos] = val;
  } else
    return;

  samplecount++;
  while (pos > 0 && values[pos - 1] > values[pos]) {
    double temp = values[pos - 1];
    values[pos - 1] = values[pos];
    values[pos] = temp;
    pos--;
  }
}

static int has_converged() {
  return (samplecount >= kbest) &&
         ((1 + epsilon) * values[0] >= values[kbest - 1]);
}

/* Generalized execution macro for fcyc handlers */
#define RUN_FCYC(CALL)                                                         \
  init_sampler();                                                              \
  do {                                                                         \
    if (clear_cache)                                                           \
      clear();                                                                 \
    if (compensate)                                                            \
      start_comp_counter();                                                    \
    else                                                                       \
      start_counter();                                                         \
    CALL;                                                                      \
    add_sample(compensate ? get_comp_counter() : get_counter());               \
  } while (!has_converged() && samplecount < maxsamples);                      \
  double result = values[0];                                                   \
  free(values);                                                                \
  values = NULL;                                                               \
  return result;

double fcyc(test_funct f, int *params) { RUN_FCYC(f(params)) }
double fcyc_v(test_funct_v f, void *params[]) { RUN_FCYC(f(params)) }

/* Config Setters */
void set_fcyc_clear_cache(int clear) { clear_cache = clear; }
void set_fcyc_cache_block(int bytes) { cache_block = bytes; }
void set_fcyc_compensate(int compensate_arg) { compensate = compensate_arg; }
void set_fcyc_k(int k) { kbest = k; }
void set_fcyc_maxsamples(int maxsamples_arg) { maxsamples = maxsamples_arg; }
void set_fcyc_epsilon(double epsilon_arg) { epsilon = epsilon_arg; }

void set_fcyc_cache_size(int bytes) {
  if (bytes != cache_bytes) {
    cache_bytes = bytes;
    if (cache_buf) {
      free(cache_buf);
      cache_buf = NULL;
    }
  }
}

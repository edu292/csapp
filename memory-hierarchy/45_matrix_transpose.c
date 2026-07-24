#include <linux/perf_event.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>

typedef void (*transpose_func_t)(int n, const int src[n][n], int dst[n][n]);

typedef struct {
  const char *name;
  transpose_func_t func;
} algo_t;

typedef struct {
  double time_sec;
  double cpe;
  long long l1d_alloc_misses;
  long long l1d_writebacks;
} bench_result_t;

int perf_event_setup(uint64_t raw_config) {
  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(pe));
  pe.type = PERF_TYPE_RAW;
  pe.size = sizeof(pe);
  pe.config = raw_config;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  return (int)syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
}

bench_result_t benchmark(transpose_func_t func, int n, int iterations) {
  int (*src)[n] = malloc(sizeof(int[n][n]));
  int (*dst)[n] = malloc(sizeof(int[n][n]));

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      src[i][j] = rand() % 1000;
    }
  }

  // 0x0151: L1D.REPLACEMENT (L1D Misses / Line Fills)
  // 0x02F0: L2_TRANS.L1D_WB (L1D Evicted Modified Lines / Write-Backs to L2)
  int perf_fd_miss = perf_event_setup(0x0151);
  int perf_fd_wb = perf_event_setup(0x02F0);

  // Warm-up run
  func(n, src, dst);

  struct timespec start_t, end_t;
  unsigned int dummy;

  if (perf_fd_miss != -1) {
    ioctl(perf_fd_miss, PERF_EVENT_IOC_RESET, 0);
    ioctl(perf_fd_miss, PERF_EVENT_IOC_ENABLE, 0);
  }
  if (perf_fd_wb != -1) {
    ioctl(perf_fd_wb, PERF_EVENT_IOC_RESET, 0);
    ioctl(perf_fd_wb, PERF_EVENT_IOC_ENABLE, 0);
  }

  clock_gettime(CLOCK_MONOTONIC, &start_t);
  unsigned long long start_c = __rdtscp(&dummy);

  for (int it = 0; it < iterations; it++) {
    func(n, src, dst);
    __asm__ volatile("" : : "g"(dst) : "memory");
  }

  unsigned long long end_c = __rdtscp(&dummy);
  clock_gettime(CLOCK_MONOTONIC, &end_t);

  long long l1d_misses = -1, l1d_writebacks = -1;

  if (perf_fd_miss != -1) {
    ioctl(perf_fd_miss, PERF_EVENT_IOC_DISABLE, 0);
    if (read(perf_fd_miss, &l1d_misses, sizeof(long long)) == -1) {
      l1d_misses = -1;
    }
    close(perf_fd_miss);
  }

  if (perf_fd_wb != -1) {
    ioctl(perf_fd_wb, PERF_EVENT_IOC_DISABLE, 0);
    if (read(perf_fd_wb, &l1d_writebacks, sizeof(long long)) == -1) {
      l1d_writebacks = -1;
    }
    close(perf_fd_wb);
  }

  free(src);
  free(dst);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;

  unsigned long long total_elements = (unsigned long long)n * n * iterations;
  res.cpe = (double)(end_c - start_c) / total_elements;
  res.l1d_alloc_misses = l1d_misses;
  res.l1d_writebacks = l1d_writebacks;

  return res;
}

void transpose_naive(int dim, const int src[dim][dim], int dst[dim][dim]) {
  for (int i = 0; i < dim; i++)
    for (int j = 0; j < dim; j++)
      dst[j][i] = src[i][j];
}

#define DEFINE_TRANSPOSE_BLOCKED(BSIZE)                                        \
  void transpose_blocked_##BSIZE(int dim, const int src[dim][dim],             \
                                 int dst[dim][dim]) {                          \
    int i = 0, j = 0;                                                          \
    for (i = 0; i <= dim - BSIZE; i += BSIZE) {                                \
      for (j = 0; j <= dim - BSIZE; j += BSIZE) {                              \
        for (int ii = i; ii < i + BSIZE; ii++) {                               \
          for (int jj = j; jj < j + BSIZE; jj++) {                             \
            dst[jj][ii] = src[ii][jj];                                         \
          }                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    for (int r = 0; r < dim; r++) {                                            \
      for (int c = (r < i) ? j : 0; c < dim; c++) {                            \
        dst[c][r] = src[r][c];                                                 \
      }                                                                        \
    }                                                                          \
  }

DEFINE_TRANSPOSE_BLOCKED(2)
DEFINE_TRANSPOSE_BLOCKED(4)
DEFINE_TRANSPOSE_BLOCKED(16)
DEFINE_TRANSPOSE_BLOCKED(32)
DEFINE_TRANSPOSE_BLOCKED(128)

void _transpose_recursive_1(int r, int c, int rows, int cols, int full_dim,
                            const int src[full_dim][full_dim],
                            int dst[full_dim][full_dim]) {
  if (rows == 1 && cols == 1) {
    dst[c][r] = src[r][c];
    return;
  }

  if (rows >= cols) {
    int r1 = rows / 2;
    int r2 = rows - r1;
    _transpose_recursive_1(r, c, r1, cols, full_dim, src, dst);
    _transpose_recursive_1(r + r1, c, r2, cols, full_dim, src, dst);
  } else {
    int c1 = cols / 2;
    int c2 = cols - c1;
    _transpose_recursive_1(r, c, rows, c1, full_dim, src, dst);
    _transpose_recursive_1(r, c + c1, rows, c2, full_dim, src, dst);
  }
}

void transpose_recursive_1(int dim, const int src[dim][dim],
                           int dst[dim][dim]) {
  _transpose_recursive_1(0, 0, dim, dim, dim, src, dst);
}

void _transpose_recursive_16(int r, int c, int rows, int cols, int full_dim,
                             const int src[full_dim][full_dim],
                             int dst[full_dim][full_dim]) {
  if (rows <= 16 && cols <= 16) {
    for (int i = r; i < r  + rows; i++) {
      for (int j = c; j < c + cols; j++) {
        dst[j][i] = src[i][j];
      }
    }
    return;
  }

  if (rows >= cols) {
    int r1 = rows / 2;
    int r2 = rows - r1;
    _transpose_recursive_16(r, c, r1, cols, full_dim, src, dst);
    _transpose_recursive_16(r + r1, c, r2, cols, full_dim, src, dst);
  } else {
    int c1 = cols / 2;
    int c2 = cols - c1;
    _transpose_recursive_16(r, c, rows, c1, full_dim, src, dst);
    _transpose_recursive_16(r, c + c1, rows, c2, full_dim, src, dst);
  }
}

void transpose_recursive_16(int dim, const int src[dim][dim],
                            int dst[dim][dim]) {
  _transpose_recursive_16(0, 0, dim, dim, dim, src, dst);
}

int main(void) {
  algo_t algorithms[] = {{"naive", transpose_naive},
                         {"blocked_2x2", transpose_blocked_2},
                         {"blocked_4x4", transpose_blocked_4},
                         {"blocked_16x16", transpose_blocked_16},
                         {"blocked_32x32", transpose_blocked_32},
                         {"blocked_128x128", transpose_blocked_128},
                         {"recursive 1", transpose_recursive_1},
                         {"recursive 16", transpose_recursive_16}};
  int num_algos = sizeof(algorithms) / sizeof(algorithms[0]);

  int test_sizes[] = {15, 32, 129, 256, 513};
  int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

  printf("=== Tests ===\n");
  int all_passed = 1;

  for (int a = 0; a < num_algos; a++) {
    int passed_count = 0;

    for (int t = 0; t < num_tests; t++) {
      int n = test_sizes[t];

      int (*src)[n] = malloc(sizeof(int[n][n]));
      int (*expected)[n] = malloc(sizeof(int[n][n]));
      int (*dst)[n] = calloc(n * n, sizeof(int));

      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          src[i][j] = rand() % 1000;
        }
      }

      transpose_naive(n, src, expected);
      algorithms[a].func(n, src, dst);

      int test_ok = 1;
      for (int i = 0; i < n && test_ok; i++) {
        for (int j = 0; j < n; j++) {
          if (dst[i][j] != expected[i][j]) {
            test_ok = 0;
            printf("FAIL: Algorithm '%s' | Test %dx%d | First mismatch at "
                   "[%d][%d]\n",
                   algorithms[a].name, n, n, i, j);
            break;
          }
        }
      }

      if (test_ok)
        passed_count++;
      else
        all_passed = 0;

      free(src);
      free(expected);
      free(dst);
    }
    printf("%-20s: %d/%d tests passed\n", algorithms[a].name, passed_count,
           num_tests);
  }

  if (!all_passed) {
    printf("\nTest failure detected. Aborting benchmarks.\n");
    return 1;
  }

  const int BENCH_N = 512;
  const int ITERATIONS = 1000;

  unsigned long long total_accesses = 2ULL * BENCH_N * BENCH_N * ITERATIONS;

  printf("\n=== Benchmarks (%d iterations, Dim N=%d, %llu total mem ops) ===\n",
         ITERATIONS, BENCH_N, total_accesses);
  printf("%-20s | %-10s | %-8s | %-18s | %-18s\n", "Algorithm", "Time (s)",
         "CPE", "L1D Fill Miss %", "L1D Write-Back %");
  printf("---------------------------------------------------------------------"
         "-------------------\n");

  for (int a = 0; a < num_algos; a++) {
    bench_result_t res = benchmark(algorithms[a].func, BENCH_N, ITERATIONS);

    char miss_str[64] = "N/A", wb_str[64] = "N/A";

    if (res.l1d_alloc_misses >= 0) {
      double rate = ((double)res.l1d_alloc_misses / total_accesses) * 100.0;
      snprintf(miss_str, sizeof(miss_str), "%.2f%%", rate);
    }
    if (res.l1d_writebacks >= 0) {
      double rate = ((double)res.l1d_writebacks / total_accesses) * 100.0;
      snprintf(wb_str, sizeof(wb_str), "%.2f%%", rate);
    }

    printf("%-20s | %-10.4f | %-8.2f | %-18s | %-18s\n", algorithms[a].name,
           res.time_sec, res.cpe, miss_str, wb_str);
  }
  return 0;
}

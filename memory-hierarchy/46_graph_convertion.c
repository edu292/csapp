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

typedef void (*convert_func_t)(int n, int matrix[n][n]);

typedef struct {
  const char *name;
  convert_func_t func;
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

bench_result_t benchmark(convert_func_t func, int n, int iterations) {
  int (*matrix)[n] = malloc(sizeof(int[n][n]));
  int (*backup)[n] = malloc(sizeof(int[n][n]));

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      backup[i][j] = rand() % 2;
    }
  }

  // 0x0151: L1D.REPLACEMENT (L1D Misses / Line Fills)
  // 0x02F0: L2_TRANS.L1D_WB (L1D Evicted Modified Lines / Write-Backs to L2)
  int perf_fd_miss = perf_event_setup(0x0151);
  int perf_fd_wb = perf_event_setup(0x02F0);

  // Warm-up run
  memcpy(matrix, backup, sizeof(int[n][n]));
  func(n, matrix);

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
    func(n, matrix);
    __asm__ volatile("" : : "g"(matrix) : "memory");
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

  free(matrix);
  free(backup);

  bench_result_t res;
  res.time_sec =
      (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1e9;

  unsigned long long total_elements = (unsigned long long)n * n * iterations;
  res.cpe = (double)(end_c - start_c) / total_elements;
  res.l1d_alloc_misses = l1d_misses;
  res.l1d_writebacks = l1d_writebacks;

  return res;
}

void naive(int dim, int G[dim][dim]) {
  int i, j;
  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      G[j][i] = G[j][i] | G[i][j];
}

void blocked(int dim, int G[dim][dim]) {
  int i, j;
  for (i = 0; i < dim - 16; i += 16)
    for (j = 0; j < dim - 16; j += 16)
      for (int ii = i; ii < i + 16; ii++)
        for (int jj = j; jj < j + 16; jj++)
          G[jj][ii] = G[jj][ii] | G[ii][jj];

  for (int r = 0; r < dim; r++)
    for (int c = (r < i) ? j : 0; c < dim; c++)
      G[c][r] = G[c][r] | G[r][c];
}

void blocked_optimized(int dim, int G[dim][dim]) {
  const int B = 16;

  for (int i = 0; i < dim; i += B) {
    for (int j = i; j < dim; j += B) {
      int i_max = (i + B < dim) ? i + B : dim;
      int j_max = (j + B < dim) ? j + B : dim;

      if (i == j) {
        for (int ii = i; ii < i_max; ii++) {
          for (int jj = ii + 1; jj < j_max; jj++) {
            int val = G[ii][jj] | G[jj][ii];
            G[ii][jj] = val;
            G[jj][ii] = val;
          }
        }
      } else {
        for (int ii = i; ii < i_max; ii++) {
          for (int jj = j; jj < j_max; jj++) {
            int val = G[ii][jj] | G[jj][ii];
            G[ii][jj] = val;
            G[jj][ii] = val;
          }
        }
      }
    }
  }
}

void _recursive_optmized(int i, int j, int rows, int cols, int full_dim,
                         int G[full_dim][full_dim]) {
  if (rows <= 16 && cols <= 16) {
    if (i == j) {
      for (int r = i; r < i + rows; r++) {
        for (int c = r + 1; c < j + cols; c++) {
          int val = G[r][c] | G[c][r];
          G[r][c] = val;
          G[c][r] = val;
        }
      }
    } else {
      for (int r = i; r < i + rows; r++) {
        for (int c = j; c < j + cols; c++) {
          int val = G[c][r] | G[r][c];
          G[c][r] = val;
          G[r][c] = val;
        }
      }
    }
    return;
  }

  if (rows >= cols) {
    int half1 = rows / 2;
    int half2 = rows - half1;
    _recursive_optmized(i, j, half1, cols, full_dim, G);
    _recursive_optmized(i + half1, j, half2, cols, full_dim, G);
  } else {
    int half1 = cols / 2;
    int half2 = cols - half1;
    _recursive_optmized(i, j, rows, half1, full_dim, G);
    _recursive_optmized(i, j + half1, rows, half2, full_dim, G);
  }
}

void recursive_optmized(int dim, int G[dim][dim]) {
  _recursive_optmized(0, 0, dim, dim, dim, G);
};

void _recursive(int i, int j, int rows, int cols, int full_dim,
                int G[full_dim][full_dim]) {
  if (rows <= 16 && cols <= 16) {
    for (int r = i; r < i + rows; r++)
      for (int c = j; c < j + cols; c++)
        G[c][r] = G[c][r] | G[r][c];
    return;
  }

  if (rows >= cols) {
    int half1 = rows / 2;
    int half2 = rows - half1;
    _recursive(i, j, half1, cols, full_dim, G);
    _recursive(i + half1, j, half2, cols, full_dim, G);
  } else {
    int half1 = cols / 2;
    int half2 = cols - half1;
    _recursive(i, j, rows, half1, full_dim, G);
    _recursive(i, j + half1, rows, half2, full_dim, G);
  }
}

void recursive(int dim, int G[dim][dim]) { _recursive(0, 0, dim, dim, dim, G); }

int main(void) {
  algo_t algorithms[] = {{"naive", naive},
                         {"blocked", blocked},
                         {"recursive", recursive},
                         {"blocked optmized", blocked_optimized},
                         {"recursive optmized", recursive_optmized}};
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
      int (*actual)[n] = malloc(sizeof(int[n][n]));

      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          src[i][j] = rand() % 2;
        }
      }

      memcpy(expected, src, sizeof(int[n][n]));
      memcpy(actual, src, sizeof(int[n][n]));

      naive(n, expected);
      algorithms[a].func(n, actual);

      int test_ok = 1;
      for (int i = 0; i < n && test_ok; i++) {
        for (int j = 0; j < n; j++) {
          if (actual[i][j] != expected[i][j]) {
            test_ok = 0;
            printf("FAIL: Algorithm '%s' | Test %dx%d | First mismatch at "
                   "[%d][%d] (got %d, expected %d)\n",
                   algorithms[a].name, n, n, i, j, actual[i][j],
                   expected[i][j]);
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
      free(actual);
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

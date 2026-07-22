#include "clock.h"
#include "config.h"
#include "defs.h"
#include "fcyc.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <x86intrin.h>

/* Constants and Macros */
#define MAX_BENCHMARKS 100
#define DIM_CNT 5
#define BSIZE 32 /* cache block size / SIMD alignment in bytes */
#define MAX_DIM 1280
#define ODD_DIM 96

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

extern team_t team;

typedef struct {
  lab_test_func tfunct;
  double cpes[DIM_CNT];
  char *description;
  unsigned short valid;
} bench_t;

/* Globals */
static int test_dim_rotate[] = {64, 128, 256, 512, 1024};
static int test_dim_smooth[] = {32, 64, 128, 256, 512};
static double rotate_baseline_cpes[] = {R64, R128, R256, R512, R1024};
static double smooth_baseline_cpes[] = {S32, S64, S128, S256, S512};

static bench_t benchmarks_rotate[MAX_BENCHMARKS];
static bench_t benchmarks_smooth[MAX_BENCHMARKS];
static int rotate_benchmark_count = 0;
static int smooth_benchmark_count = 0;

/* Provide enough padding to perfectly align all three image arrays */
static pixel data[(3 * MAX_DIM * MAX_DIM) + 256];
static pixel *orig = NULL;
static pixel *copy_of_orig = NULL;
static pixel *result = NULL;

double rotate_maxmean = 0.0;
char *rotate_maxmean_desc = NULL;
double smooth_maxmean = 0.0;
char *smooth_maxmean_desc = NULL;

/******************** Benchmarking Registration *************************/

void add_smooth_function(lab_test_func f, char *description) {
  benchmarks_smooth[smooth_benchmark_count++] =
      (bench_t){f, {0}, description, 0};
}

void add_rotate_function(lab_test_func f, char *description) {
  benchmarks_rotate[rotate_benchmark_count++] =
      (bench_t){f, {0}, description, 0};
}

static inline int random_in_interval(int low, int high) {
  return (rand() % (high - low)) + low;
}

/******************** Image Setup & Verification *************************/

static void create(int dim) {
  /* FIX: Ensure ALL buffers are 32-byte aligned for AVX/SSE vector
   * instructions */
  uintptr_t curr = (uintptr_t)data;

  curr = (curr + (BSIZE - 1)) & ~(uintptr_t)(BSIZE - 1);
  orig = (pixel *)curr;

  curr = (uintptr_t)(orig + dim * dim);
  curr = (curr + (BSIZE - 1)) & ~(uintptr_t)(BSIZE - 1);
  result = (pixel *)curr;

  curr = (uintptr_t)(result + dim * dim);
  curr = (curr + (BSIZE - 1)) & ~(uintptr_t)(BSIZE - 1);
  copy_of_orig = (pixel *)curr;

  int total_pixels = dim * dim;
  for (int i = 0; i < total_pixels; i++) {
    orig[i].red = random_in_interval(0, 65536);
    orig[i].green = random_in_interval(0, 65536);
    orig[i].blue = random_in_interval(0, 65536);
    copy_of_orig[i] = orig[i];

    result[i].red = 0;
    result[i].green = 0;
    result[i].blue = 0;
  }
}

static inline int compare_pixels(pixel p1, pixel p2) {
  return (p1.red != p2.red) || (p1.green != p2.green) || (p1.blue != p2.blue);
}

static int check_orig(int dim) {
  int total = dim * dim;
  for (int i = 0; i < total; i++) {
    if (compare_pixels(orig[i], copy_of_orig[i])) {
      printf("\nError: Original image has been changed!\n");
      return 1;
    }
  }
  return 0;
}

static int check_rotate(int dim) {
  if (check_orig(dim))
    return 1;

  int err = 0, badi = 0, badj = 0;
  pixel orig_bad = {0}, res_bad = {0};

  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      if (compare_pixels(orig[RIDX(i, j, dim)],
                         result[RIDX(dim - 1 - j, i, dim)])) {
        err++;
        badi = i;
        badj = j;
        orig_bad = orig[RIDX(i, j, dim)];
        res_bad = result[RIDX(dim - 1 - j, i, dim)];
      }
    }
  }

  if (err) {
    printf("\nERROR: Dimension=%d, %d errors\n", dim, err);
    printf("E.g., The following two pixels should have equal value:\n");
    printf("src[%d][%d].{red,green,blue} = {%d,%d,%d}\n", badi, badj,
           orig_bad.red, orig_bad.green, orig_bad.blue);
    printf("dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n", (dim - 1 - badj),
           badi, res_bad.red, res_bad.green, res_bad.blue);
  }
  return err;
}

static pixel check_average(int dim, int i, int j, pixel *src) {
  int sum0 = 0, sum1 = 0, sum2 = 0, num = 0;
  int top_left_i = max(i - 1, 0), top_left_j = max(j - 1, 0);
  int bottom_right_i = min(i + 1, dim - 1),
      bottom_right_j = min(j + 1, dim - 1);

  for (int ii = top_left_i; ii <= bottom_right_i; ii++) {
    for (int jj = top_left_j; jj <= bottom_right_j; jj++) {
      num++;
      sum0 += src[RIDX(ii, jj, dim)].red;
      sum1 += src[RIDX(ii, jj, dim)].green;
      sum2 += src[RIDX(ii, jj, dim)].blue;
    }
  }
  return (pixel){(unsigned short)(sum0 / num), (unsigned short)(sum1 / num),
                 (unsigned short)(sum2 / num)};
}

static int check_smooth(int dim) {
  if (check_orig(dim))
    return 1;

  int err = 0, badi = 0, badj = 0;
  pixel right = {0}, wrong = {0};

  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      pixel smoothed = check_average(dim, i, j, orig);
      if (compare_pixels(result[RIDX(i, j, dim)], smoothed)) {
        err++;
        badi = i;
        badj = j;
        wrong = result[RIDX(i, j, dim)];
        right = smoothed;
      }
    }
  }

  if (err) {
    printf("\nERROR: Dimension=%d, %d errors\n", dim, err);
    printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n", badi, badj,
           wrong.red, wrong.green, wrong.blue);
    printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n", badi,
           badj, right.red, right.green, right.blue);
  }
  return err;
}

/******************** Benchmarking Logic *************************/

void func_wrapper(void *arglist[]) {
  lab_test_func f = (lab_test_func)arglist[0];
  int mydim = *((int *)arglist[1]);
  f(mydim, (pixel *)arglist[2], (pixel *)arglist[3]);
}

void test_rotate(int bench_index) {
  char *description = benchmarks_rotate[bench_index].description;

  /* Check correctness for odd dimension */
  create(ODD_DIM);
  benchmarks_rotate[bench_index].tfunct(ODD_DIM, orig, result);
  if (check_rotate(ODD_DIM))
    return;

  for (int test_num = 0; test_num < DIM_CNT; test_num++) {
    int dim = test_dim_rotate[test_num];
    create(dim);

    benchmarks_rotate[bench_index].tfunct(dim, orig, result);
    if (check_rotate(dim))
      return;

    void *arglist[4] = {(void *)benchmarks_rotate[bench_index].tfunct, &dim,
                        orig, result};
    create(dim);

    double work = (double)dim * dim;
    benchmarks_rotate[bench_index].cpes[test_num] =
        fcyc_v((test_funct_v)&func_wrapper, arglist) / work;
  }

  /* Print results */
  printf("Rotate: Version = %s:\nDim\t", description);
  for (int i = 0; i < DIM_CNT; i++)
    printf("\t%d", test_dim_rotate[i]);

  printf("\tMean\nYour CPEs");
  for (int i = 0; i < DIM_CNT; i++)
    printf("\t%.1f", benchmarks_rotate[bench_index].cpes[i]);

  printf("\nBaseline CPEs");
  for (int i = 0; i < DIM_CNT; i++)
    printf("\t%.1f", rotate_baseline_cpes[i]);

  double prod = 1.0;
  printf("\nSpeedup\t");
  for (int i = 0; i < DIM_CNT; i++) {
    if (benchmarks_rotate[bench_index].cpes[i] <= 0.0)
      exit(EXIT_FAILURE);
    double ratio =
        rotate_baseline_cpes[i] / benchmarks_rotate[bench_index].cpes[i];
    prod *= ratio;
    printf("\t%.1f", ratio);
  }

  double mean = pow(prod, 1.0 / DIM_CNT);
  printf("\t%.1f\n\n", mean);
  if (mean > rotate_maxmean) {
    rotate_maxmean = mean;
    rotate_maxmean_desc = description;
  }
}

void test_smooth(int bench_index) {
  char *description = benchmarks_smooth[bench_index].description;

  create(ODD_DIM);
  benchmarks_smooth[bench_index].tfunct(ODD_DIM, orig, result);
  if (check_smooth(ODD_DIM))
    return;

  for (int test_num = 0; test_num < DIM_CNT; test_num++) {
    int dim = test_dim_smooth[test_num];
    create(dim);

    benchmarks_smooth[bench_index].tfunct(dim, orig, result);
    if (check_smooth(dim))
      return;

    void *arglist[4] = {(void *)benchmarks_smooth[bench_index].tfunct, &dim,
                        orig, result};
    create(dim);

    double work = (double)dim * dim;
    benchmarks_smooth[bench_index].cpes[test_num] =
        fcyc_v((test_funct_v)&func_wrapper, arglist) / work;
  }

  /* Print results */
  printf("Smooth: Version = %s:\nDim\t", description);
  for (int i = 0; i < DIM_CNT; i++)
    printf("\t%d", test_dim_smooth[i]);

  printf("\tMean\nYour CPEs");
  for (int i = 0; i < DIM_CNT; i++)
    printf("\t%.1f", benchmarks_smooth[bench_index].cpes[i]);

  printf("\nBaseline CPEs");
  for (int i = 0; i < DIM_CNT; i++)
    printf("\t%.1f", smooth_baseline_cpes[i]);

  double prod = 1.0;
  printf("\nSpeedup\t");
  for (int i = 0; i < DIM_CNT; i++) {
    if (benchmarks_smooth[bench_index].cpes[i] <= 0.0)
      exit(EXIT_FAILURE);
    double ratio =
        smooth_baseline_cpes[i] / benchmarks_smooth[bench_index].cpes[i];
    prod *= ratio;
    printf("\t%.1f", ratio);
  }

  double mean = pow(prod, 1.0 / DIM_CNT);
  printf("\t%.1f\n\n", mean);
  if (mean > smooth_maxmean) {
    smooth_maxmean = mean;
    smooth_maxmean_desc = description;
  }
}

/******************** Main & Argument Parsing *************************/

void usage(char *progname) {
  fprintf(stderr, "Usage: %s [-hqg] [-f <func_file>] [-d <dump_file>]\n",
          progname);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int c; /* FIX: getopt returns int, not char. Prevents infinite looping*/
  int quit_after_dump = 0, skip_teamname_check = 0, autograder = 0, seed = 1729;
  char *bench_func_file = NULL, *func_dump_file = NULL;

  register_rotate_functions();
  register_smooth_functions();

  while ((c = getopt(argc, argv, "tgqf:d:s:h")) != -1) {
    switch (c) {
    case 't':
      skip_teamname_check = 1;
      break;
    case 's':
      seed = atoi(optarg);
      break;
    case 'g':
      autograder = 1;
      break;
    case 'q':
      quit_after_dump = 1;
      break;
    case 'f':
      bench_func_file = strdup(optarg);
      break;
    case 'd':
      func_dump_file = strdup(optarg);
      FILE *fp = fopen(func_dump_file, "w");
      if (!fp)
        exit(-5);
      for (int i = 0; i < rotate_benchmark_count; i++)
        fprintf(fp, "R:%s\n", benchmarks_rotate[i].description);
      for (int i = 0; i < smooth_benchmark_count; i++)
        fprintf(fp, "S:%s\n", benchmarks_smooth[i].description);
      fclose(fp);
      break;
    case 'h':
    default:
      usage(argv[0]);
    }
  }

  if (quit_after_dump)
    exit(EXIT_SUCCESS);

  if (!skip_teamname_check) {
    if (strcmp("bovik", team.team) == 0)
      exit(1);
    printf("Teamname: %s\nMember 1: %s\nEmail 1: %s\n", team.team, team.name1,
           team.email1);
    if (*team.name2)
      printf("Member 2: %s\nEmail 2: %s\n", team.name2, team.email2);
    printf("\n");
  }

  srand(seed);

  if (autograder) {
    rotate_benchmark_count = smooth_benchmark_count = 1;
    benchmarks_rotate[0] = (bench_t){rotate, {0}, "rotate() function", 1};
    benchmarks_smooth[0] = (bench_t){smooth, {0}, "smooth() function", 1};
  } else if (bench_func_file != NULL) {
    char func_line[256];
    FILE *fp = fopen(bench_func_file, "r");
    if (!fp)
      exit(-5);
    while (fgets(func_line, 256, fp)) {
      char *func_name = func_line, **strptr = &func_name;
      char flag = strsep(strptr, ":")[0];
      func_name = strsep(strptr, "\n");

      if (flag == 'R') {
        for (int i = 0; i < rotate_benchmark_count; i++)
          if (strcmp(benchmarks_rotate[i].description, func_name) == 0)
            benchmarks_rotate[i].valid = 1;
      } else if (flag == 'S') {
        for (int i = 0; i < smooth_benchmark_count; i++)
          if (strcmp(benchmarks_smooth[i].description, func_name) == 0)
            benchmarks_smooth[i].valid = 1;
      }
    }
    fclose(fp);
  } else {
    for (int i = 0; i < rotate_benchmark_count; i++)
      benchmarks_rotate[i].valid = 1;
    for (int i = 0; i < smooth_benchmark_count; i++)
      benchmarks_smooth[i].valid = 1;
  }

  set_fcyc_cache_size(1 << 14);
  set_fcyc_clear_cache(1);
  set_fcyc_compensate(1);

  for (int i = 0; i < rotate_benchmark_count; i++)
    if (benchmarks_rotate[i].valid)
      test_rotate(i);
  for (int i = 0; i < smooth_benchmark_count; i++)
    if (benchmarks_smooth[i].valid)
      test_smooth(i);

  if (autograder) {
    printf("\nbestscores:%.1f:%.1f:\n", rotate_maxmean, smooth_maxmean);
  } else {
    printf("Summary of Your Best Scores:\n");
    printf("  Rotate: %3.1f (%s)\n", rotate_maxmean, rotate_maxmean_desc);
    printf("  Smooth: %3.1f (%s)\n", smooth_maxmean, smooth_maxmean_desc);
  }

  return 0;
}

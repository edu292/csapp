/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/
#include "defs.h"
#include <immintrin.h>
#include <stdint.h>

/*
 * Please fill in the following team struct
 */
team_t team = {"edusk", /* Team name */

               "Eduardo Skoroboatei Gomes", /* First member full name */
               "eduskoroboatei@gmail.com",  /* First member email address */

               "no_one", "no_one_really"};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/*
 * naive_rotate - The naive baseline version of rotate
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) {
  int i, j;

  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      dst[RIDX(dim - 1 - j, i, dim)] = src[RIDX(i, j, dim)];
}

static const int8_t PAD_MASK_BYTES[32] = {
    0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, 9, 10, 11, -1, -1,
    0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, 9, 10, 11, -1, -1};

static const int8_t UNPAD_MIRROR_MASK_BYTES[32] = {
    0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, -1, -1, -1, -1};
/*
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) {
  __m256i pad_mask = _mm256_loadu_si256((const __m256i *)PAD_MASK_BYTES);
  __m256i unpad_mask =
      _mm256_loadu_si256((const __m256i *)UNPAD_MIRROR_MASK_BYTES);
  __m256i row[8];
  int i, j;

  for (i = 0; i < dim - 7; i += 8) {
    for (j = 0; j < dim - 7; j += 8) {

      for (int k = 0; k < 8; k++) {
        __m256i raw =
            _mm256_loadu_si256((const __m256i *)&src[RIDX(i + k, j, dim)]);
        row[k] = _mm256_shuffle_epi8(raw, pad_mask);
      }

      __m256i t0 = _mm256_unpacklo_epi64(row[0], row[1]);
      __m256i t1 = _mm256_unpackhi_epi64(row[0], row[1]);
      __m256i t2 = _mm256_unpacklo_epi64(row[2], row[3]);
      __m256i t3 = _mm256_unpackhi_epi64(row[2], row[3]);
      __m256i t4 = _mm256_unpacklo_epi64(row[4], row[5]);
      __m256i t5 = _mm256_unpackhi_epi64(row[4], row[5]);
      __m256i t6 = _mm256_unpacklo_epi64(row[6], row[7]);
      __m256i t7 = _mm256_unpackhi_epi64(row[6], row[7]);

      row[0] = _mm256_permute2x128_si256(t0, t4, 0x20);
      row[1] = _mm256_permute2x128_si256(t1, t5, 0x20);
      row[2] = _mm256_permute2x128_si256(t2, t6, 0x20);
      row[3] = _mm256_permute2x128_si256(t3, t7, 0x20);
      row[4] = _mm256_permute2x128_si256(t0, t4, 0x31);
      row[5] = _mm256_permute2x128_si256(t1, t5, 0x31);
      row[6] = _mm256_permute2x128_si256(t2, t6, 0x31);
      row[7] = _mm256_permute2x128_si256(t3, t7, 0x31);

      for (int k = 0; k < 8; k++) {
        __m256i packed = _mm256_shuffle_epi8(row[k], unpad_mask);

        pixel *dst_row = &dst[RIDX(dim - 1 - (j + k), i, dim)];

        _mm_storeu_si128((__m128i *)dst_row, _mm256_castsi256_si128(packed));
        _mm_storeu_si64((uint8_t *)dst_row + 16,
                        _mm256_extracti128_si256(packed, 1));
      }
    }
  }

  for (int r = 0; r < dim; r++) {
    for (int c = (r < j) ? j : 0; c < dim; c++) {
      dst[RIDX(dim - 1 - c, r, dim)] = src[RIDX(r, c, dim)];
    }
  }
} /*********************************************************************
   * register_rotate_functions - Register all of your different versions
   *     of the rotate kernel with the driver by calling the
   *     add_rotate_function() for each test function. When you run the
   *     driver program, it will test and report the performance of each
   *     registered test function.
   *********************************************************************/

void register_rotate_functions() {
  add_rotate_function(&naive_rotate, naive_rotate_descr);
  add_rotate_function(&rotate, rotate_descr);
  /* ... Register additional test functions here */
}

/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
  unsigned red;
  unsigned green;
  unsigned blue;
  unsigned num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/*
 * initialize_pixel_sum - Initializes all fields of sum to 0
 */
static void initialize_pixel_sum(pixel_sum *sum) {
  sum->red = sum->green = sum->blue = 0;
  sum->num = 0;
  return;
}

/*
 * accumulate_sum - Accumulates field values of p in corresponding
 * fields of sum
 */
static void accumulate_sum(pixel_sum *sum, pixel p) {
  sum->red += (int)p.red;
  sum->green += (int)p.green;
  sum->blue += (int)p.blue;
  sum->num++;
  return;
}

/*
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) {
  current_pixel->red = (unsigned short)(sum.red / sum.num);
  current_pixel->green = (unsigned short)(sum.green / sum.num);
  current_pixel->blue = (unsigned short)(sum.blue / sum.num);
  return;
}

/*
 * avg - Returns averaged pixel value at (i,j)
 */
static pixel avg(int dim, int i, int j, pixel *src) {
  int ii, jj;
  pixel_sum sum;
  pixel current_pixel;

  initialize_pixel_sum(&sum);
  for (ii = max(i - 1, 0); ii <= min(i + 1, dim - 1); ii++)
    for (jj = max(j - 1, 0); jj <= min(j + 1, dim - 1); jj++)
      accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

  assign_sum_to_pixel(&current_pixel, sum);
  return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) {
  int i, j;

  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth.
 * IMPORTANT: This is the version you will be graded on
 */
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) { naive_smooth(dim, src, dst); }

/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_smooth_functions() {
  add_smooth_function(&smooth, smooth_descr);
  add_smooth_function(&naive_smooth, naive_smooth_descr);
  /* ... Register additional test functions here */
}

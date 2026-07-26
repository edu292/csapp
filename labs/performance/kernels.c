/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/
#include "defs.h"
#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>

/*
 * Please fill in the following team struct
 */
team_t team = {"edusk", /* Team name */

               "Eduardo Skoroboatei Gomes", /* First member full name */
               "eduskoroboatei@gmail.com",  /* First member email address */

               "", ""};

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

#define BLOCK_SIZE 32
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) {
  // Byte stride moving vertically down a column
  const long long stride = dim * sizeof(pixel);

  // Gather indices: offset by 0, +1, +2, +3 rows moving downwards
  __m256i v_indices = _mm256_setr_epi64x(0, stride, 2LL * stride, 3LL * stride);

  // Shuffle mask: Compress 6-byte pixels down, dropping the 2-byte over-reads.
  __m256i shuf_mask =
      _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, -1, -1, -1, -1,
                       0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, -1, -1, -1, -1);

  // Permute mask: Cross-lane pack the 32-bit chunks into 24 contiguous bytes.
  __m256i perm_mask = _mm256_setr_epi32(0, 1, 2, 4, 5, 6, 7, 7);

  for (int j_blk = 0; j_blk < dim; j_blk += BLOCK_SIZE) {
    for (int i_blk = 0; i_blk < dim; i_blk += BLOCK_SIZE) {

      for (int j = j_blk; j < j_blk + BLOCK_SIZE; ++j) {
        // Destination row j is written sequentially left-to-right
        uint8_t *dst_ptr = (uint8_t *)&dst[RIDX(j, i_blk, dim)];

        // For Counter-Clockwise: dst row j comes from src column (dim - 1 - j)
        int src_col = dim - 1 - j;

        for (int i = i_blk; i < i_blk + BLOCK_SIZE; i += 4) {
          // Source reads column 'src_col', top-to-bottom
          const long long *base_ptr =
              (const long long *)&src[RIDX(i, src_col, dim)];

          // Gather and compress
          __m256i v = _mm256_i64gather_epi64(base_ptr, v_indices, 1);
          v = _mm256_shuffle_epi8(v, shuf_mask);
          v = _mm256_permutevar8x32_epi32(v, perm_mask);

          // Store 24 bytes (4 pixels) to destination
          _mm_storeu_si128((__m128i *)dst_ptr, _mm256_castsi256_si128(v));
          _mm_storel_epi64((__m128i *)(dst_ptr + 16),
                           _mm256_extracti128_si256(v, 1));

          dst_ptr += 24;
        }
      }
    }
  }
}

/*
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

char blocked_rotate_descr[] =
    "blocked rotate: iterating in blocks with 8 unrolled loops";
void blocked_rotate(int dim, pixel *src, pixel *dst) {
  int dim0 = dim - 1;
  for (int i = 0; i < dim; i += 32)
    for (int j = 0; j < dim; j += 32)
      for (int ii = i; ii < i + 32; ii++)
        for (int jj = j; jj < j + 32; jj += 8) {
          dst[RIDX(dim0 - jj, ii, dim)] = src[RIDX(ii, jj, dim)];
          int jj1 = jj + 1;
          dst[RIDX(dim0 - jj1, ii, dim)] = src[RIDX(ii, jj1, dim)];
          int jj2 = jj + 2;
          dst[RIDX(dim0 - jj2, ii, dim)] = src[RIDX(ii, jj2, dim)];
          int jj3 = jj + 3;
          dst[RIDX(dim0 - jj3, ii, dim)] = src[RIDX(ii, jj3, dim)];
          int jj4 = jj + 4;
          dst[RIDX(dim0 - jj4, ii, dim)] = src[RIDX(ii, jj4, dim)];
          int jj5 = jj + 5;
          dst[RIDX(dim0 - jj5, ii, dim)] = src[RIDX(ii, jj5, dim)];
          int jj6 = jj + 6;
          dst[RIDX(dim0 - jj6, ii, dim)] = src[RIDX(ii, jj6, dim)];
          int jj7 = jj + 7;
          dst[RIDX(dim0 - jj7, ii, dim)] = src[RIDX(ii, jj7, dim)];
        }
}

char scalar_store_decr[] = "scalar store";
void scalar_store_store(int dim, pixel *src, pixel *dst) {
  // 32-byte aligned intermediate buffer to hold 32 pixels (192 bytes total)
  __attribute__((aligned(32))) pixel buf[BLOCK_SIZE];

  // L1 Cache-blocked traversal
  for (int j_blk = 0; j_blk < dim; j_blk += BLOCK_SIZE) {
    for (int i_blk = 0; i_blk < dim; i_blk += BLOCK_SIZE) {

      // Process the 32x32 block
      for (int j_dst = j_blk; j_dst < j_blk + BLOCK_SIZE; ++j_dst) {

        // For CCW rotation:
        // Destination row 'j_dst' comes from source column 'dim - 1 - j_dst'
        int src_j = dim - 1 - j_dst;

        // 1. Scalar Gather: Read 32 pixels down the source column
        // Because we cache block, these vertical strided reads hit the L1 cache
        for (int k = 0; k < BLOCK_SIZE; ++k) {
          int src_i = i_blk + k; // Source row advances as dest column advances
          buf[k] = src[RIDX(src_i, src_j, dim)];
        }

        // 2. SIMD Store: Write 192 bytes (6x 32-byte chunks) left-to-right to
        // destination
        uint8_t *dst_ptr = (uint8_t *)&dst[RIDX(j_dst, i_blk, dim)];
        const __m256i *buf_ptr = (const __m256i *)buf;

        // Unrolled vector stores for maximum throughput.
        // We use aligned loads for the stack buffer, and unaligned stores for
        // safety on dst.
        _mm256_storeu_si256((__m256i *)(dst_ptr + 0),
                            _mm256_load_si256(&buf_ptr[0]));
        _mm256_storeu_si256((__m256i *)(dst_ptr + 32),
                            _mm256_load_si256(&buf_ptr[1]));
        _mm256_storeu_si256((__m256i *)(dst_ptr + 64),
                            _mm256_load_si256(&buf_ptr[2]));
        _mm256_storeu_si256((__m256i *)(dst_ptr + 96),
                            _mm256_load_si256(&buf_ptr[3]));
        _mm256_storeu_si256((__m256i *)(dst_ptr + 128),
                            _mm256_load_si256(&buf_ptr[4]));
        _mm256_storeu_si256((__m256i *)(dst_ptr + 160),
                            _mm256_load_si256(&buf_ptr[5]));
      }
    }
  }
}

void register_rotate_functions() {
  add_rotate_function(&naive_rotate, naive_rotate_descr);
  add_rotate_function(&blocked_rotate, blocked_rotate_descr);
  add_rotate_function(rotate, rotate_descr);
  add_rotate_function(scalar_store_store, scalar_store_decr);
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
static inline void smooth_boundary(int dim, int i, int j, pixel *src,
                                   pixel *dst) {
  int min_i = (i > 0) ? (i - 1) : 0;
  int max_i = (i < dim - 1) ? (i + 1) : (dim - 1);
  int min_j = (j > 0) ? (j - 1) : 0;
  int max_j = (j < dim - 1) ? (j + 1) : (dim - 1);

  uint32_t r = 0, g = 0, b = 0;
  int num = 0;

  for (int r_i = min_i; r_i <= max_i; r_i++) {
    for (int c_j = min_j; c_j <= max_j; c_j++) {
      pixel p = src[RIDX(r_i, c_j, dim)];
      r += p.red;
      g += p.green;
      b += p.blue;
      num++;
    }
  }

  dst[RIDX(i, j, dim)] =
      (pixel){(unsigned short)(r / num), (unsigned short)(g / num),
              (unsigned short)(b / num)};
}

char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) {
  for (int j = 0; j < dim; j++) {
    smooth_boundary(dim, 0, j, src, dst);
    smooth_boundary(dim, dim - 1, j, src, dst);
  }

  for (int i = 1; i < dim - 1; i++) {
    smooth_boundary(dim, i, 0, src, dst);
    smooth_boundary(dim, i, dim - 1, src, dst);
  }

  for (int i = 1; i < dim - 1; i++) {
    pixel *p0 = src + (i - 1) * dim;
    pixel *p1 = src + i * dim;
    pixel *p2 = src + (i + 1) * dim;
    pixel *dst_row = dst + i * dim;

    uint32_t c0_r = p0[0].red + p1[0].red + p2[0].red;
    uint32_t c0_g = p0[0].green + p1[0].green + p2[0].green;
    uint32_t c0_b = p0[0].blue + p1[0].blue + p2[0].blue;

    uint32_t c1_r = p0[1].red + p1[1].red + p2[1].red;
    uint32_t c1_g = p0[1].green + p1[1].green + p2[1].green;
    uint32_t c1_b = p0[1].blue + p1[1].blue + p2[1].blue;

    for (int j = 1; j < dim - 1; j++) {
      uint32_t c2_r = p0[j + 1].red + p1[j + 1].red + p2[j + 1].red;
      uint32_t c2_g = p0[j + 1].green + p1[j + 1].green + p2[j + 1].green;
      uint32_t c2_b = p0[j + 1].blue + p1[j + 1].blue + p2[j + 1].blue;

      dst_row[j] = (pixel){(unsigned short)((c0_r + c1_r + c2_r) / 9),
                                 (unsigned short)((c0_g + c1_g + c2_g) / 9),
                                 (unsigned short)((c0_b + c1_b + c2_b) / 9)};

      c0_r = c1_r;
      c0_g = c1_g;
      c0_b = c1_b;
      c1_r = c2_r;
      c1_g = c2_g;
      c1_b = c2_b;
    }
  }
}
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

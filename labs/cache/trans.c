/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <immintrin.h>

int min(int x, int y) { return x < y ? x : y; }

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  __m256i row[8];
  int i, j;

  for (i = 0; i <= N - 8; i += 8) {
    for (j = 0; j <= M - 8; j += 8) {

      for (int k = 0; k < 8; k++) {
        row[k] = _mm256_loadu_si256((const __m256i *)&A[i + k][j]);
      }

      __m256i t0 = _mm256_unpacklo_epi32(row[0], row[1]);
      __m256i t1 = _mm256_unpackhi_epi32(row[0], row[1]);
      __m256i t2 = _mm256_unpacklo_epi32(row[2], row[3]);
      __m256i t3 = _mm256_unpackhi_epi32(row[2], row[3]);
      __m256i t4 = _mm256_unpacklo_epi32(row[4], row[5]);
      __m256i t5 = _mm256_unpackhi_epi32(row[4], row[5]);
      __m256i t6 = _mm256_unpacklo_epi32(row[6], row[7]);
      __m256i t7 = _mm256_unpackhi_epi32(row[6], row[7]);

      __m256i v0 = _mm256_unpacklo_epi64(t0, t2);
      __m256i v1 = _mm256_unpackhi_epi64(t0, t2);
      __m256i v2 = _mm256_unpacklo_epi64(t1, t3);
      __m256i v3 = _mm256_unpackhi_epi64(t1, t3);
      __m256i v4 = _mm256_unpacklo_epi64(t4, t6);
      __m256i v5 = _mm256_unpackhi_epi64(t4, t6);
      __m256i v6 = _mm256_unpacklo_epi64(t5, t7);
      __m256i v7 = _mm256_unpackhi_epi64(t5, t7);

      row[0] = _mm256_permute2x128_si256(v0, v4, 0x20);
      row[1] = _mm256_permute2x128_si256(v1, v5, 0x20);
      row[2] = _mm256_permute2x128_si256(v2, v6, 0x20);
      row[3] = _mm256_permute2x128_si256(v3, v7, 0x20);
      row[4] = _mm256_permute2x128_si256(v0, v4, 0x31);
      row[5] = _mm256_permute2x128_si256(v1, v5, 0x31);
      row[6] = _mm256_permute2x128_si256(v2, v6, 0x31);
      row[7] = _mm256_permute2x128_si256(v3, v7, 0x31);

      for (int k = 0; k < 8; k++) {
        _mm256_storeu_si256((__m256i *)&B[j + k][i], row[k]);
      }
    }
  }

  for (int r = 0; r < N; r++) {
    for (int c = (r < i) ? j : 0; c < M; c++) {
      B[c][r] = A[r][c];
    }
  }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      B[j][i] = A[i][j];
    }
  }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}

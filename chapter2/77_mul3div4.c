#include <limits.h>
#include <stdio.h>

int mul3div4(int x) {
  unsigned short w = sizeof(int) << 3;
  int mul3 = ((x << 2) - x);
  int bias = mul3 >> (w - 1) & ((1 << 2) - 1);
  return (mul3 + bias) >> 2;
}

int main(void) {
  // Array of test cases covering exact divisions, rounding direction,
  // zero, and values that might overflow during the intermediate
  // multiplication.
  struct {
    int x;
    int expected;
  } tests[] = {
      // --- Small Positive Numbers ---
      {0, 0}, // (0 * 3) / 4 = 0
      {4, 3}, // (4 * 3) / 4 = 3
      {8, 6}, // (8 * 3) / 4 = 6
      {5, 3}, // (5 * 3) / 4 = 15 / 4 = 3.75 -> 3 (Round toward 0)
      {1, 0}, // (1 * 3) / 4 = 3 / 4 = 0.75 -> 0

      // --- Small Negative Numbers (Must Round Toward Zero) ---
      {-4, -3}, // (-4 * 3) / 4 = -3
      {-5, -3}, // (-5 * 3) / 4 = -15 / 4 = -3.75 -> -3
      {-1, 0},  // (-1 * 3) / 4 = -3 / 4 = -0.75 -> 0

      // --- Intermediate Multiplication Overflow Boundaries ---
      // These values would overflow a 32-bit int if multiplied by 3 directly.
      // E.g., INT_MAX * 3 = 6442450941 (exceeds 2147483647)
      {INT_MAX, INT_MAX * 3 / 4},   // Mathematical (2147483647 * 3) / 4 =
                               // 1610612735.25 -> 1610612735
      {1073741824, 1073741824 * 3 / 4}, // Exactly 2^30. 3 * 2^30 overflows 32-bit space,
                               // but final result fits.

      // --- Negative Intermediate Overflows ---
      {INT_MIN,
       INT_MIN * 3 / 4}, // Mathematical (-2147483648 * 3) / 4 = -1610612736
      {-1073741824, -1073741824 * 3 / 4}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = mul3div4(tests[i].x);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | x = %d | Expected: %d | Got: %d\n", i, tests[i].x,
             tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

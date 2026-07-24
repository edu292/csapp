#include <limits.h>
#include <stdio.h>

int tsub_ok(int x, int y) {
    unsigned short w = sizeof(int) << 3;

    unsigned is_different_sign = (unsigned) (x ^ y) >> (w - 1);
    int result = x - y;
    unsigned result_has_y_sign = (unsigned) ~(result ^ y) >> (w - 1);

    return !(is_different_sign & result_has_y_sign);
}

int main(void) {
  // Array of test cases to cover normal subtraction, zero edge cases,
  // positive overflow, and negative overflow.
  struct {
    int x;
    int y;
    int expected;
  } tests[] = {
      // --- Normal Subtractions (No Overflow) ---
      {10, 5, 1},
      {-10, -5, 1},
      {5, 10, 1},
      {-5, 10, 1},
      {10, -5, 1},
      {0, 0, 1},
      {INT_MAX, 0, 1},
      {INT_MIN, 0, 1},
      {INT_MAX, INT_MAX, 1},
      {INT_MIN, INT_MIN, 1},

      // --- Positive Overflow Boundaries (x is pos, y is neg -> x - y > TMax) ---
      {INT_MAX, -1, 0},                    // TMax - (-1) -> Overflow
      {2147483640, -10, 0},                // Close to TMax overflow
      {INT_MAX, INT_MIN, 0},               // TMax - TMin -> Severe overflow

      // --- Negative Overflow Boundaries (x is neg, y is pos -> x - y < TMin) ---
      {INT_MIN, 1, 0},                     // TMin - 1 -> Overflow
      {-2147483640, 10, 0},                // Close to TMin overflow
      {INT_MIN, INT_MAX, 0},               // TMin - TMax -> Severe overflow
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = tsub_ok(tests[i].x, tests[i].y);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | x = %d, y = %d | Expected: %d | Got: %d\n",
             i, tests[i].x, tests[i].y, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

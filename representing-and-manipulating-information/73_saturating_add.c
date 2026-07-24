#include <limits.h>
#include <stdio.h>

int saturating_add_sub(int x, int y) {
  unsigned short w = sizeof(int) << 3;
  unsigned sign_bit_mask = 0b1 << (w - 1);
  unsigned is_same_sign = ~(x ^ y) & sign_bit_mask;
  int result = x + y;
  int has_overflown = (x ^ result) & is_same_sign;
  unsigned threshold =
      (unsigned) has_overflown - ((unsigned)~x  >> (w - 1));

  unsigned result_mask = ~(has_overflown >> (w - 1));

  return (result_mask & result) | (~result_mask & threshold);
}

int saturating_add(int x, int y) {
  unsigned short w = sizeof(int) << 3;
  unsigned sign_bit_mask = 0b1 << (w - 1);
  unsigned is_same_sign = ~(x ^ y) & sign_bit_mask;
  int result = x + y;
  int has_overflown = (x ^ result) & is_same_sign;
  unsigned threshold =
      has_overflown ^ ~(x  >> (w - 1));

  unsigned result_mask = ~(has_overflown >> (w - 1));

  return (result_mask & result) | (~result_mask & threshold);
}

int main(void) {
  // Array of test cases to cover normal addition, edge cases,
  // positive overflow (saturating to TMax), and negative overflow (saturating
  // to TMin).
  struct {
    int x;
    int y;
    int expected;
  } tests[] = {
      // --- Normal Additions (No Overflow) ---
      {5, 10, 15},
      {-5, -10, -15},
      {-5, 10, 5},
      {10, -5, 5},
      {0, 0, 0},
      {INT_MAX, 0, INT_MAX},
      {INT_MIN, 0, INT_MIN},

      // --- Positive Overflow Boundaries (Should saturate to TMax) ---
      {INT_MAX, 1, INT_MAX},       // TMax + 1 -> TMax
      {INT_MAX, INT_MAX, INT_MAX}, // TMax + TMax -> TMax
      {2147483640, 10, INT_MAX},   // Close to TMax overflow

      // --- Negative Overflow Boundaries (Should saturate to TMin) ---
      {INT_MIN, -1, INT_MIN},      // TMin - 1 -> TMin
      {INT_MIN, INT_MIN, INT_MIN}, // TMin + TMin -> TMin
      {-2147483640, -10, INT_MIN}  // Close to TMin overflow
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = saturating_add(tests[i].x, tests[i].y);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | x = %d, y = %d | Expected: %d | Got: %d\n", i,
             tests[i].x, tests[i].y, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

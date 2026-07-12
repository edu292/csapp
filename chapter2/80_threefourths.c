#include <limits.h>
#include <stdio.h>

int threefourths(int x) {
  short w = sizeof(int) << 3;

  int bias = x >> (w - 1) & 3;

  int quotient = (x + bias) >> 2;
  int quotient_threefourths = (quotient << 1) + quotient;

  int remainder = x - (quotient << 2);
  int remainder_triple = (remainder << 1) + remainder;
  int remainder_threefourths = (remainder_triple + bias) >> 2;

  return quotient_threefourths + remainder_threefourths;
}

int main(void) {
  struct {
    int input;
    int expected;
  } tests[] = {
      {0, 0},
      {4, 3},
      {7, 5}, // 7 * 0.75 = 5.25 -> round toward zero -> 5
      {8, 6},
      {-4, -3},
      {-7, -5}, // -7 * 0.75 = -5.25 -> round toward zero -> -5
      {-8, -6},
      {1, 0},  // 1 * 0.75 = 0.75 -> round toward zero -> 0
      {-1, 0}, // -1 * 0.75 = -0.75 -> round toward zero -> 0
      {INT_MAX,
       1610612735}, // Avoid overflow: (2147483647 * 3) / 4 without spilling
      {INT_MIN, -1610612736} // Avoid overflow
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = threefourths(tests[i].input);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Input: %d (0x%08X) | Expected: %d | Got: %d\n", i,
             tests[i].input, tests[i].input, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

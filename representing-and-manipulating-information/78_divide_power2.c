#include <limits.h>
#include <stdio.h>

int divide_power2(int x, int k) {
  unsigned short w = sizeof(int) << 3;
  int bias = (x >> (w - 1)) & ((1 << k) - 1);
  return (x + bias) >> k;
}

int main(void) {
  // Array of test cases covering positive numbers, negative numbers (rounding
  // rules), zero, and extreme boundaries like INT_MIN and INT_MAX.
  struct {
    int x;
    int k;
    int expected;
  } tests[] = {
      // --- Positive Even Divisions (Exact) ---
      {16, 2, 4},    // 16 / 4 = 4
      {1024, 10, 1}, // 1024 / 1024 = 1
      {0, 5, 0},     // 0 / 32 = 0

      // --- Positive Odd Divisions (Round Down / Towards Zero) ---
      {15, 2, 3}, // 15 / 4 = 3.75 -> 3
      {1, 1, 0},  // 1 / 2 = 0.5 -> 0

      // --- Negative Even Divisions (Exact) ---
      {-16, 2, -4}, // -16 / 4 = -4
      {-32, 5, -1}, // -32 / 32 = -1

      // --- Negative Odd Divisions (Must Round TOWARDS Zero) ---
      // Crucial: A raw arithmetic shift right (-15 >> 2) yields -4.
      // C division expects -15 / 4 = -3.75 -> -3.
      {-15, 2, -3},
      {-1, 1, 0},  // -1 / 2 = -0.5 -> 0
      {-5, 2, -1}, // -5 / 4 = -1.25 -> -1

      // --- Extreme Edge Cases ---
      {INT_MAX, 1, INT_MAX / 2},
      {INT_MIN, 1, INT_MIN / 2},
      {INT_MIN, 30, -2} // -2147483648 / 1073741824 = -2
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = divide_power2(tests[i].x, tests[i].k);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | x = %d, k = %d | Expected: %d | Got: %d\n", i,
             tests[i].x, tests[i].k, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

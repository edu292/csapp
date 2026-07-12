#include <limits.h>
#include <stdio.h>

int signed_high_prod(int x, int y) {
  unsigned short w = sizeof(int) << 3;
  long long result = (long long)x * y;
  return (int)(result >> w);
}

unsigned unsigned_high_prod(unsigned x, unsigned y) {
  unsigned short w = sizeof(int) << 3;
  unsigned x_sign_mask = (int)x >> (w - 1);
  unsigned y_sign_mask = (int)y >> (w - 1);
  unsigned result = signed_high_prod(x, y);

  return result + (x & y_sign_mask) + (y & x_sign_mask);
}

int main(void) {
  // Array of test cases to cover small numbers, exact boundaries,
  // and maximum unsigned values.
  struct {
    unsigned x;
    unsigned y;
    unsigned expected;
  } tests[] = {
      // --- Small Numbers (High product should be 0) ---
      {0, 0, 0},
      {12345, 1, 0},
      {0xFFFFFFFF, 0, 0},
      {0xFFFF, 0xFFFF, 0}, // 0xFFFF * 0xFFFF fits within 32 bits

      // --- Simple Power of 2 Overflows ---
      {0x10000, 0x10000, 1}, // (2^16) * (2^16) = 2^32 -> Upper 32-bits is 1
      {0x20000, 0x10000, 2}, // (2^17) * (2^16) = 2^33 -> Upper 32-bits is 2

      // --- Harder Boundary Test Cases ---
      {0xFFFFFFFF, 2, 1}, // 0xFFFFFFFF * 2 = 0x1FFFFFFFE -> Upper 32-bits is 1
      {0xFFFFFFFF, 0x10000, 0xFFFF}, // Upper 16 bits shift up completely

      // --- Maximum Unsigned Val Boundaries ---
      // 0xFFFFFFFF * 0xFFFFFFFF = 0xFFFFFFFE00000001
      {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE},

      // 0x80000000 * 0x80000000 = 0x4000000000000000
      {0x80000000, 0x80000000, 0x40000000}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    unsigned result = unsigned_high_prod(tests[i].x, tests[i].y);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | x = 0x%08X, y = 0x%08X | Expected: 0x%08X | Got: "
             "0x%08X\n",
             i, tests[i].x, tests[i].y, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

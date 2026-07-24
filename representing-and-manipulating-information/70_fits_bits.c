#include <stdio.h>

int fits_bits_ub(int x, int n) {
  unsigned lower_bound_mask = ((1 << (n - 1)) - 1);
  unsigned short w = sizeof(int) << 3;
  unsigned sign_bit_mask = 0b1 << (w - 1);
  int value_in_n_bits =
      (x & lower_bound_mask) - ((x & sign_bit_mask) >> (w - n));

  return value_in_n_bits == x;
}

int fits_bits(int x, int n) {
    unsigned short w = sizeof(int) << 3;
    unsigned short shift = (w - n);
    return x << shift >> shift == x;
}

int main(void) {
  // Array of test cases to cover boundary conditions, positive/negative limits,
  // and edge cases
  struct {
    int x;
    int n;
    int expected;
  } tests[] = {
      {0, 1, 1},  // Minimum bit width (n=1): 0 fits in 1 bit (range -1 to 0)
      {-1, 1, 1}, // Minimum bit width (n=1): -1 fits in 1 bit
      {1, 1, 0},  // Minimum bit width (n=1): 1 requires 2 bits (range -2 to 1)

      {5, 4, 1},  // 5 (0101) fits in 4 bits (range -8 to 7)
      {-8, 4, 1}, // -8 (1000) is the exact minimum for 4 bits
      {7, 4, 1},  // 7 (0111) is the exact maximum for 4 bits
      {8, 4, 0},  // 8 (1000) requires 5 bits (positive overflow for 4 bits)
      {-9, 4, 0}, // -9 requires 5 bits (negative overflow for 4 bits)

      {127, 8, 1},  // 127 fits in 8 bits (range -128 to 127)
      {-128, 8, 1}, // -128 fits in 8 bits
      {128, 8, 0},  // 128 requires 9 bits
      {-129, 8, 0}, // -129 requires 9 bits

      {2147483647, 32, 1},  // TMax fits in 32 bits
      {-2147483648, 32, 1}, // TMin fits in 32 bits
      {-2, 1, 0}, // 14: n=1 edge case. -2 (1110) does NOT fit in 1 bit (range
      {1, 2, 1},  // 15: n=2 boundary. 1 (01) fits in 2 bits (range -2 to 1)
      {-2, 2, 1}, // 16: n=2 boundary. -2 (10) fits in 2 bits
      {2, 2, 0},  // 17: n=2 overflow. 2 (0010) requires 3 bits
      {-3, 2, 0}, // 18: n=2 underflow. -3 (1101) requires 3 bits
      {0x7FFFFFFF, 31, 0} // 19: n=31 boundary. TMax does NOT fit in 31 bits
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = fits_bits(tests[i].x, tests[i].n);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Input: x = %d, n = %d | Expected: %d | Got: %d\n",
             i, tests[i].x, tests[i].n, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

#include <stdio.h>

unsigned rotate_left(unsigned x, int n) {
  unsigned short w = sizeof(int) << 3;
  unsigned mask = ((1 << (n + 1)) - 1);
  unsigned to_sum = x >> (w - n) & mask;
  return (x << n) + to_sum;
}

int main(void) {
  // Array of test cases to cover normal rotations, edge cases, and multi-byte
  // values
  struct {
    unsigned input;
    int n;
    unsigned expected;
  } tests[] = {
      {0x12345678, 0, 0x12345678}, // Edge case: Rotate by 0 (no change)
      {0x12345678, 32,
       0x12345678}, // Edge case: Rotate by 32 (full circle, no change)
      {0x80000000, 1, 0x00000001},  // MSB wraps around to become LSB
      {0x00000001, 31, 0x80000000}, // LSB moves all the way to MSB
      {0x12345678, 4,
       0x23456781}, // Nibble-aligned rotation (hex digits shift left)
      {0x12345678, 8, 0x34567812},  // Byte-aligned rotation
      {0x12345678, 16, 0x56781234}, // Half-word rotation
      {0xABCDEF01, 4, 0xBCDEF01A},  // Verify upper byte wrap-around
      {0x00000000, 5, 0x00000000},  // Rotating all zeros
      {0xFFFFFFFF, 11, 0xFFFFFFFF}  // Rotating all ones
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    unsigned result = rotate_left(tests[i].input, tests[i].n);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Input: 0x%08X, n = %2d | Expected: 0x%08X | Got: "
             "0x%08X\n",
             i, tests[i].input, tests[i].n, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

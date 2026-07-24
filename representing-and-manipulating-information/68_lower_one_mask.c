#include <stdio.h>
int lower_one_mask(int n) {
  unsigned short w = sizeof(int) << 3;
  return (1 << (n >> 1) << (n >> 1) << (n & 0b1) ) - 1;
}

int main(void) {
  // Array of test cases covering edge cases (0, 32) and normal cases
  struct {
    int n;
    unsigned expected;
  } tests[] = {
      {0, 0x00000000},  // Edge case: 0 bits
      {1, 0x00000001},  // 1 bit
      {2, 0x00000003},  // 2 bits
      {4, 0x0000000F},  // Half a byte
      {8, 0x000000FF},  // Full byte
      {16, 0x0000FFFF}, // Half the integer (16 bits)
      {31, 0x7FFFFFFF}, // All bits except MSB
      {32, 0xFFFFFFFF}  // Edge case: All 32 bits set
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    unsigned result = lower_one_mask(tests[i].n);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d (n = %d) | Expected: 0x%08X | Got: 0x%08X\n", i,
             tests[i].n, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

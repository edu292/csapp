#include <stdio.h>

int leftmost_one(unsigned x) {
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;

    return x - (x >> 1);
}

int main(void) {
  struct {
    unsigned input;
    unsigned expected;
  } tests[] = {
      {0x00000000, 0x00000000}, // Edge case: No bits set
      {0x00000001, 0x00000001}, // Edge case: LSB only
      {0x00000002, 0x00000002}, // Single bit set
      {0x00000003, 0x00000002}, // Multiple bits, check left-most
      {0x00000008, 0x00000008}, // Single bit set
      {0x0000000A, 0x00000008}, // 10 (1010) -> 8 (1000)
      {0x000000FF, 0x00000080}, // Lower byte full
      {0x7FFFFFFF, 0x40000000}, // Max positive signed int (all but MSB set)
      {0x80000000, 0x80000000}, // Edge case: MSB only set
      {0xFFFFFFFF, 0x80000000}  // All bits set
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    unsigned result = leftmost_one(tests[i].input);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Input: 0x%08X | Expected: 0x%08X | Got: 0x%08X\n",
             i, tests[i].input, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

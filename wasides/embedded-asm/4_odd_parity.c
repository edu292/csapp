#include <stdio.h>

int odd_parity(unsigned long x) {
  unsigned char odd_parity_byte;
  int odd_byte_counter = 0;
  for (int i = sizeof(unsigned long); i > 0; i--) {
    asm("testb %b[x], %b[x]\n\t"
        "setnp %[op]"
        : [op] "=r"(odd_parity_byte)
        : [x] "r"(x));
    odd_byte_counter += odd_parity_byte;

    x >>= 8;
  }

  return odd_byte_counter & 1;
}

int main(void) {
  struct {
    unsigned long input;
    int expected;
  } tests[] = {
      {0x0UL, 0},                // 0 bits set (even)
      {0x1UL, 1},                // 1 bit set (odd)
      {0x3UL, 0},                // 2 bits set (even)
      {0x7UL, 1},                // 3 bits set (odd)
      {0x0000000AUL, 0},         // 1010 -> 2 bits (even)
      {0x0000000FUL, 0},         // 1111 -> 4 bits (even)
      {0x000000FFUL, 0},         // 8 bits (even)
      {0x000001FFUL, 1},         // 9 bits (odd)
      {0x7FFFFFFFUL, 1},         // 31 bits (odd)
      {0xFFFFFFFFUL, 0},         // 32 bits (even)
      {0x8000000000000000UL, 1}, // MSB only (odd)
      {0xFFFFFFFFFFFFFFFFUL, 0}  // All 64 bits set (even)
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = odd_parity(tests[i].input);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Input: 0x%016lX | Expected: %d | Got: %d\n", i,
             tests[i].input, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

#include <stdio.h>
typedef signed packed_t;

int xbyte(packed_t word, int bytenum) {
    unsigned short w = sizeof(int) << 3;
    int selected_byte = (word >> (bytenum << 3)) & 0xFF;
    return selected_byte << (w - 8) >> (w - 8);
}

int main(void) {
  // Array of test cases to cover different byte positions (0-3), 
  // positive/negative values, and sign-extension boundaries.
  struct {
    packed_t word;
    int bytenum;
    int expected;
  } tests[] = {
      // --- Testing Byte 0 (Least Significant Byte) ---
      {0x0000007F, 0, 127},    // Max positive signed byte (0x7F -> 127)
      {0x00000080, 0, -128},   // Min negative signed byte (0x80 -> -128)
      {0x000000FF, 0, -1},     // All ones signed byte (0xFF -> -1)
      {0x00000000, 0, 0},      // Zero

      // --- Testing Byte 1 ---
      {0x00007F00, 1, 127},    // Max positive in byte 1
      {0x00008000, 1, -128},   // Min negative in byte 1
      {0x0000FF00, 1, -1},     // -1 in byte 1

      // --- Testing Byte 2 ---
      {0x007F0000, 2, 127},    // Max positive in byte 2
      {0x00800000, 2, -128},   // Min negative in byte 2
      {0x00FF0000, 2, -1},     // -1 in byte 2

      // --- Testing Byte 3 (Most Significant Byte) ---
      {0x7F000000, 3, 127},    // Max positive in byte 3
      {0x80000000, 3, -128},   // Min negative in byte 3
      {0xFF000000, 3, -1},     // -1 in byte 3

      // --- Mixed Content Stress Tests ---
      {0xAABBCCDD, 0, -35},    // 0xDD -> -35
      {0xAABBCCDD, 1, -52},    // 0xCC -> -52
      {0xAABBCCDD, 2, -69},    // 0xBB -> -69
      {0xAABBCCDD, 3, -86}     // 0xAA -> -86
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = xbyte(tests[i].word, tests[i].bytenum);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Word: 0x%08X, Byte Num: %d | Expected: %d | Got: %d\n",
             i, tests[i].word, tests[i].bytenum, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

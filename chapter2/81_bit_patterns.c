#include <stdio.h>
unsigned a(int k) { return ~0LL << k; }

unsigned b(int k, int j) { return a(j) ^ a(j + k); }

int main() {
  struct {
    int k;
    unsigned expected;
  } tests_a[] = {
      {0, 0xFFFFFFFF},  // 32 ones, 0 zeros
      {1, 0xFFFFFFFE},  // 31 ones, 1 zero
      {8, 0xFFFFFF00},  // 24 ones, 8 zeros
      {16, 0xFFFF0000}, // 16 ones, 16 zeros
      {31, 0x80000000}, // 1 one,  31 zeros
      {32, 0x00000000}  // 0 ones,  32 zeros
  };

  // Test Setup for Function B: 0^(w-k-j) 1^k 0^j
  struct {
    int k;
    int j;
    unsigned expected;
  } tests_b[] = {
      {32, 0, 0xFFFFFFFF}, // 0 zeros, 32 ones, 0 zeros
      {16, 8, 0x00FFFF00}, // 8 zeros, 16 ones, 8 zeros
      {1, 0, 0x00000001},  // 31 zeros, 1 one, 0 zeros
      {1, 31, 0x80000000}, // 0 zeros, 1 one, 31 zeros
      {0, 4, 0x00000000},  // k=0 means no ones at all
      {8, 12, 0x000FF000}  // 12 zeros, 8 ones, 12 zeros
  };

  int passed = 0;
  int total_tests = 0;

  // Run Tests for A
  int num_tests_a = sizeof(tests_a) / sizeof(tests_a[0]);
  total_tests += num_tests_a;
  for (int i = 0; i < num_tests_a; i++) {
    unsigned result = (unsigned)a(tests_a[i].k);
    if (result == tests_a[i].expected) {
      passed++;
    } else {
      printf("FAIL [A]: Test %d | k: %d | Expected: 0x%08X | Got: 0x%08X\n", i,
             tests_a[i].k, tests_a[i].expected, result);
    }
  }

  // Run Tests for B
  int num_tests_b = sizeof(tests_b) / sizeof(tests_b[0]);
  total_tests += num_tests_b;
  for (int i = 0; i < num_tests_b; i++) {
    unsigned result = (unsigned)b(tests_b[i].k, tests_b[i].j);
    if (result == tests_b[i].expected) {
      passed++;
    } else {
      printf(
          "FAIL [B]: Test %d | k: %d, j: %d | Expected: 0x%08X | Got: 0x%08X\n",
          i, tests_b[i].k, tests_b[i].j, tests_b[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, total_tests);
  return (passed == total_tests) ? 0 : 1;
}

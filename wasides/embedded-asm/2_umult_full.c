#include <stdio.h>

void umult_full(unsigned long x, unsigned long y, unsigned long *dest) {
  asm("movq %[x], %%rax\n\t"
      "mulq %[y]\n\t"
      "movq %%rax, %[l]\n\t"
      "movq %%rdx, %[h]"
      : [l] "=m"(dest[0]), [h] "=m"(dest[1])
      : [x] "r"(x), [y] "r"(y)
      : "%rax", "%rdx");
}

int main(void) {
  struct {
    unsigned long x;
    unsigned long y;
    unsigned long expected_low;
    unsigned long expected_high;
  } tests[] = {// Zero & Simple
               {0x0UL, 0x0UL, 0x0UL, 0x0UL},
               {0x1UL, 0x0UL, 0x0UL, 0x0UL},
               {0x5UL, 0x2UL, 0xAUL, 0x0UL},

               // Single word max boundary (2^64 - 1) * 1
               {0xFFFFFFFFFFFFFFFFUL, 0x1UL, 0xFFFFFFFFFFFFFFFFUL, 0x0UL},

               // 2^32 * 2^32 = 2^64 (Low wraps to 0, high becomes 1)
               {0x100000000UL, 0x100000000UL, 0x0UL, 0x1UL},

               // Max unsigned long * 2
               {0xFFFFFFFFFFFFFFFFUL, 0x2UL, 0xFFFFFFFFFFFFFFFEUL, 0x1UL},

               // Max unsigned long * Max unsigned long
               // (2^64 - 1)^2 = 2^128 - 2^65 + 1
               // Low: 0x0000000000000001, High: 0xFFFFFFFFFFFFFFFE
               {0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL,
                0x0000000000000001UL, 0xFFFFFFFFFFFFFFFEUL}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    unsigned long dest[2] = {0, 0};
    umult_full(tests[i].x, tests[i].y, dest);

    if (dest[0] == tests[i].expected_low && dest[1] == tests[i].expected_high) {
      passed++;
    } else {
      printf("FAIL: Test %d | x: 0x%016lX, y: 0x%016lX\n"
             "  Expected: [Low: 0x%016lX, High: 0x%016lX]\n"
             "  Got:      [Low: 0x%016lX, High: 0x%016lX]\n",
             i, tests[i].x, tests[i].y, tests[i].expected_low,
             tests[i].expected_high, dest[0], dest[1]);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

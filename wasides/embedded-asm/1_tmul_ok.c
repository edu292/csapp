#include <limits.h>
#include <stdio.h>
int tmul_ok(long x, long y, long *dest) {
  unsigned char ok;
  asm("imulq %[x], %[y]\n\t"
      "movq %[y], %[d]\n\t"
      "setae %[o]"
      : [d] "=m"(*dest), [o] "=r"(ok)
      : [x] "r"(x), [y] "r"(y));

  return (int)ok;
}

int main(void) {
  struct {
    long x;
    long y;
    int expected_ok;
    long expected_val;
  } tests[] = {
      // Simple / Edge Cases
      {0, 0, 1, 0},
      {0, LONG_MAX, 1, 0},
      {1, 1, 1, 1},
      {1, LONG_MAX, 1, LONG_MAX},
      {-1, LONG_MAX, 1, -LONG_MAX},
      {5, 10, 1, 50},
      {-5, 10, 1, -50},

      // Boundary Non-Overflow Cases
      {LONG_MAX, 1, 1, LONG_MAX},
      {LONG_MIN, 1, 1, LONG_MIN},
      {LONG_MAX / 2, 2, 1, (LONG_MAX / 2) * 2},

      // Overflow Cases
      {LONG_MAX, 2, 0, 0},
      {LONG_MIN, 2, 0, 0},
      {LONG_MAX, LONG_MAX, 0, 0},
      {LONG_MIN, -1, 0, 0}, // Corner case: -LONG_MIN > LONG_MAX
      {LONG_MIN, LONG_MIN, 0, 0},
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    long result_val = 0;
    int ok = tmul_ok(tests[i].x, tests[i].y, &result_val);

    int test_passed = 0;
    if (ok == tests[i].expected_ok) {
      if (ok == 1) {
        test_passed = (result_val == tests[i].expected_val);
      } else {
        test_passed = 1;
      }
    }

    if (test_passed) {
      passed++;
    } else {
      printf("FAIL: Test %d | x: %ld, y: %ld | OK expected: %d, got: %d | Val "
             "expected: %ld, got: %ld\n",
             i, tests[i].x, tests[i].y, tests[i].expected_ok, ok,
             tests[i].expected_val, result_val);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

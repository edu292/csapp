#include <math.h>
#include <stdio.h>
double dsqrt(double x) {
  double sqrt;
  asm("sqrtsd %[x], %[r]" : [r] "=x"(sqrt) : [x] "x"(x));
  return sqrt;
};

int main(void) {
  struct {
    double input;
    double expected;
    int is_nan_expected;
  } tests[] = {// Exact Perfect Squares
               {0.0, 0.0, 0},
               {1.0, 1.0, 0},
               {4.0, 2.0, 0},
               {100.0, 10.0, 0},

               // Decimals & Fractions
               {0.25, 0.5, 0},
               {2.0, 1.4142135623730951, 0},

               // Large / Small Extreme Values
               {1e16, 1e8, 0},
               {1e-16, 1e-8, 0},
               {INFINITY, INFINITY, 0},

               // Domain Errors (Negative values -> NaN)
               {-1.0, 0.0, 1},
               {-INFINITY, 0.0, 1}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;
  const double eps =
      1e-12;

  for (int i = 0; i < num_tests; i++) {
    double result = dsqrt(tests[i].input);
    int test_passed = 0;

    if (tests[i].is_nan_expected) {
      test_passed = isnan(result);
    } else if (isinf(tests[i].expected)) {
      test_passed = isinf(result) && (result > 0 == tests[i].expected > 0);
    } else {
      test_passed = fabs(result - tests[i].expected) < eps;
    }

    if (test_passed) {
      passed++;
    } else {
      printf("FAIL: Test %d | Input: %g | Expected: %g | Got: %g\n", i,
             tests[i].input, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

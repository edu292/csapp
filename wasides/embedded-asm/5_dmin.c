#include <math.h>
#include <stdio.h>

double dmin(double x, double y) {
  double min;
  asm("vminsd %[x], %[y], %[min]" : [min] "=x"(min) : [x] "x"(x), [y] "x"(y));

  return min;
}

int main(void) {
  struct {
    double x;
    double y;
    double expected;
  } tests[] = {// Basic Cases
               {0.0, 0.0, 0.0},
               {1.0, 2.0, 1.0},
               {2.0, 1.0, 1.0},
               {-1.0, -2.0, -2.0},
               {-5.5, 2.3, -5.5},

               // Edge / Boundary Cases
               {0.0, -0.0, -0.0},
               {1e-307, 1e-308, 1e-308},
               {-INFINITY, 0.0, -INFINITY},
               {INFINITY, 100.0, 100.0}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    double result = dmin(tests[i].x, tests[i].y);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | Inputs: (%g, %g) | Expected: %g | Got: %g\n", i,
             tests[i].x, tests[i].y, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

#include <stdio.h>
#include <string.h>
unsigned f2u(float f) {
  unsigned int u;
  memcpy(&u, &f, sizeof(u));
  return  u;
}

int float_le(float x, float y) {
  unsigned ux = f2u(x);
  unsigned uy = f2u(y);

  unsigned sx = ux >> 31;
  unsigned sy = uy >> 31;

  return !(ux & 0x7FFFFFFF | uy & 0x7FFFFFFF) | sx & !sy | (ux & sx << 31 >> 31) + (uy &);
}

int main(void) {
  struct {
    float x;
    float y;
    int expected;
  } tests[] = {
      {0.0f, 0.0f, 1},
      {-0.0f, 0.0f, 1},   // +0 and -0 are equal
      {0.0f, -0.0f, 1},   // +0 and -0 are equal
      {3.14f, 5.5f, 1},   // Positive normal
      {5.5f, 3.14f, 0},
      {-5.5f, -3.14f, 1}, // Negative normal
      {-3.14f, -5.5f, 0},
      {-1.0f, 2.0f, 1},   // Mixed signs
      {2.0f, -1.0f, 0},
      {-1.0f / 0.0f, 1.0f / 0.0f, 1},  // -Inf <= +Inf
      {1.0f / 0.0f, -1.0f / 0.0f, 0},  // +Inf <= -Inf
      {1.0f / 0.0f, 1.0f / 0.0f, 1}    // +Inf <= +Inf
  };

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    int result = float_le(tests[i].x, tests[i].y);

    if (result == tests[i].expected) {
      passed++;
    } else {
      printf("FAIL: Test %d | x: %f, y: %f | Expected: %d | Got: %d\n",
             i, tests[i].x, tests[i].y, tests[i].expected, result);
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

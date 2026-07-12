#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool check_conditions(int x) {
  bool any_one = !!x;
  bool any_zero = !!~x;

  int w = sizeof(int) << 3;

  bool lsb_any_one = !!(x & 0xFF);

  unsigned unsigned_x = (unsigned)x;
  unsigned msb = unsigned_x >> (w - 8);
  bool msb_any_zero = !!(msb ^ 0xFF);

  return any_one && any_zero && lsb_any_one && msb_any_zero;
}

int main(void) {
  int test_cases[] = {
      0x00000000, 0xFFFFFFFF, 0x55555555,
      0x000000FF, 0xFFFFFF00, 0x00123400,
      0xFF1234FF, INT_MIN, INT_MAX
  };

  size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

  printf("%-12s | %-10s\n", "Input (Hex)", "Result");
  printf("-------------------------\n");

  for (size_t i = 0; i < num_tests; i++) {
    int x = test_cases[i];
    bool result = check_conditions(x);

    printf("0x%08X   | %d\n", x, result);
  }

  return 0;
}

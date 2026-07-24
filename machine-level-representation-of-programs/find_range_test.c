#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum { NEG, ZERO, POS, OTHER } range_t;

extern range_t find_range(float x);

int main(void) {
  uint64_t failed = 0;

  for (uint64_t i = 0; i <= UINT32_MAX; i++) {
    uint32_t bits = (uint32_t) i;

    uint32_t sign = bits & 0x80000000;
    uint32_t exp = bits & 0x7F800000;
    uint32_t frac = bits & 0x007FFFFF;

    range_t expected;
    if (exp == 0x7F800000 && frac != 0) {
      expected = OTHER; // NaN
    } else if ((bits & 0x7FFFFFFF) == 0) {
      expected = ZERO; // +0.0 (0x00000000) or -0.0 (0x80000000)
    } else if (sign == 0) {
      expected = POS; // Normal, Subnormal, and +Inf
    } else {
      expected = NEG; // Normal, Subnormal, and -Inf
    }

    float x;
    memcpy(&x, &bits, sizeof(x));
    range_t actual = find_range(x);

    if (actual != expected) {
      failed++;
      printf("FAIL: Bits 0x%08X | Sign: %d, Exp: 0x%02X, Frac: 0x%06X | "
             "Expected: %d | Got: %d\n",
             bits, sign >> 31, exp >> 23, frac, expected, actual);
    }

    if (bits % 500000000 == 0 && bits != 0) {
      printf("Progress: %u values tested...\n", bits);
    }
  }

  uint64_t passed = (uint64_t) UINT32_MAX + 1 - failed;
  printf("\n--- Structured Test Results ---\n");
  printf("Total Patterns Checked: %lu\n", passed + failed);
  printf("Passed:                 %lu\n", passed);
  printf("Failed:                 %lu\n", failed);

  return (failed > 0) ? 1 : 0;
}

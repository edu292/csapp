#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void *calloc(size_t nmemb, size_t size) {
  unsigned short w = sizeof(size_t) << 3;
  size_t valid_input = (!nmemb - 1) & (!size - 1);
  void *buf = malloc(nmemb * size);
  memset(buf, 0, nmemb * size);

  return (void *)((size_t)buf & valid_input);
}

int main(void) {
  // Array of test cases covering normal allocations, zero arguments, and
  // overflows
  struct {
    size_t nmemb;
    size_t size;
    int expect_null; // 1 if we expect NULL (overflow/zero), 0 if we expect a
                     // valid pointer
  } tests[] = {      // --- Normal Allocations ---
               {10, sizeof(int), 0},
               {100, 1, 0},
               {1, 1, 0},

               // --- Zero Edge Cases (Must return NULL) ---
               {0, 10, 1},
               {10, 0, 1},
               {0, 0, 1},

               // --- Potential/Actual Overflow Boundaries ---
               // SIZE_MAX is the maximum value a size_t can hold
               {SIZE_MAX, 2, 1},         // Clear overflow
               {2, SIZE_MAX, 1},         // Clear overflow
               {SIZE_MAX / 2 + 1, 2, 1}, // Exact boundary overflow
               {SIZE_MAX, SIZE_MAX, 1},  // Extreme overflow

               // Multi-precision boundary check: components that multiply to
               // exceed SIZE_MAX
               {((size_t)1 << (sizeof(size_t) * 4)),
                ((size_t)1 << (sizeof(size_t) * 4)), 1}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);
  int passed = 0;

  for (int i = 0; i < num_tests; i++) {
    void *result = calloc(tests[i].nmemb, tests[i].size);

    if (tests[i].expect_null) {
      if (result == NULL) {
        passed++;
      } else {
        printf("FAIL: Test %d | nmemb = %zu, size = %zu | Expected: NULL | "
               "Got: %p\n",
               i, tests[i].nmemb, tests[i].size, result);
        free(result);
      }
    } else {
      if (result != NULL) {
        size_t total_bytes = tests[i].nmemb * tests[i].size;
        int is_zeroed = 1;
        char *ptr = (char *)result;

        for (size_t j = 0; j < total_bytes; j++) {
          if (ptr[j] != 0) {
            is_zeroed = 0;
            break;
          }
        }

        if (is_zeroed) {
          passed++;
        } else {
          printf("FAIL: Test %d | nmemb = %zu, size = %zu | Expected: Zeroed "
                 "Memory | Got: Corrupt/Uninitialized Memory\n",
                 i, tests[i].nmemb, tests[i].size);
        }
        free(result); // Always free successful allocations
      } else {
        printf("FAIL: Test %d | nmemb = %zu, size = %zu | Expected: Valid "
               "Pointer | Got: NULL\n",
               i, tests[i].nmemb, tests[i].size);
      }
    }
  }

  printf("Result: %d/%d tests passed.\n", passed, num_tests);
  return (passed == num_tests) ? 0 : 1;
}

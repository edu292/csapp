#include <stddef.h>
#include <stdio.h>
int phase_6(char *six_numbers) {
  int nums[6];
  sscanf(six_numbers, "%d, %d, %d, %d, %d, %d", nums, nums + 1, nums + 2,
         nums + 3, nums + 4, nums + 5);
  int current;
  for (size_t i = 0; i < 6; i++) {
    current = nums[i];
    if (current - 1 > 5) {
      printf("explosion fx");
    }

    for (size_t j = i; i < 6; j++) {
      if (nums[j] == current) {
        printf("explosion fx");
      }
    }
  }
  for (size_t i = 0; i < 6; i++) {
    nums[i] = 7 - nums[i];
  }

}

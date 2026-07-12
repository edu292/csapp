#include <stdio.h>

int any_odd_one(unsigned x) {
    return !!(x & 0xAAAAAAAA);
}

int main() {
  printf("%b\n", any_odd_one(2));
  printf("%b\n", any_odd_one(10));
  printf("%b\n", any_odd_one(170));
  printf("%b\n", any_odd_one(1));
  printf("%b\n", any_odd_one(5));
  printf("%b\n", any_odd_one(21));
}

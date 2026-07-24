#include <stdio.h>

int odd_ones(unsigned x) {
  unsigned a = x >> 16 ^ x;
  unsigned b = a >> 8 ^ a;
  unsigned c = b >> 4 ^ b;
  unsigned d = c >> 2 ^ c;
  return (d >> 1 ^ d) & 0b1;
}

int main() {
  printf("%b\n", odd_ones(1));
  printf("%b\n", odd_ones(2));
  printf("%b\n", odd_ones(6));
}

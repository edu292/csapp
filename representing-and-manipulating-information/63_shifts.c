#include <limits.h>
#include <stdio.h>

unsigned srl(unsigned x, int k) {
  unsigned xsra = (int)x >> k;

  short w = (sizeof(int) << 3);
  unsigned mask = ~((~0U) << (w - k));

  return xsra & mask;
}

int sra(int x, int k) {
  int xsrl = (unsigned)x >> k;

  short w = (sizeof(int) << 3);
  int sign_bit = (unsigned)x >> (w - 1);
  int mask = (0 - sign_bit) << (w - k);

  return xsrl | mask;
}

int main() {
  unsigned x = 2352;
  printf("%.8b\n", x);

  unsigned logical_shifted = srl(x, 2);
  printf("%.8b\n", logical_shifted);
  printf("%u\n", logical_shifted);

  printf("\n");

  int y = -1;
  printf("%.8b\n", y);
  int arithmetic_shifted = sra(y, 2);
  printf("%.8b\n", arithmetic_shifted);
  printf("%d\n", arithmetic_shifted);
}

#include <stdio.h>
int fun1(unsigned word) { return (int)((word << 24) >> 24); }
int fun2(unsigned word) { return ((int)word << 24) >> 24; }

int main() {
  unsigned a = 0x00000076;
  unsigned b = 0x87654321;
  unsigned c = 0x000000C9;
  unsigned d = 0xEDCBA987;

  printf("%2x\n", fun1(a));
  printf("%2x\n", fun2(a));
  printf("\n");
  printf("%2x\n", fun1(b));
  printf("%2x\n", fun2(b));
  printf("\n");
  printf("%2x\n", fun1(c));
  printf("%2x\n", fun2(c));
  printf("\n");
  printf("%2x\n", fun1(d));
  printf("%2x\n", fun2(d));
}

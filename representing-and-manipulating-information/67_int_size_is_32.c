#include <limits.h>
#include <stdio.h>
int int_size_is_32() {
  return (unsigned) INT_MIN >> 15 >> 1 >> 15;
}

int main() {
  printf("%b\n", ~0xFFFF);
  printf("\n");
  printf("%b\n", int_size_is_32());
}

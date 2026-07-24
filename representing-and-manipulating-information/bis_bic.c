#include <stdio.h>

int bis(int x, int m) {
    return x | m;
}

int bic(int x, int m) {
    return x & (~m);
}

int bool_or(int x, int y) {
  int result = bis(x, y);
  return result;
}

int bool_xor(int x, int y) {
  int result = bis(bic(y, x), bic(x, y));
  return result;
}

int main() { 
    printf("%b", bool_or(0b0011, 0b1100));
    printf("\n");
    printf("%b", bool_xor(0b1111, 0b1100));
    return 0; 
}

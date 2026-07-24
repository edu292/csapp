#include <limits.h>
#include <stdio.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len) {
  for (int i = 0; i < len; i++)
    printf(" %.2x", start[i]);
  printf("\n");
}

void show_short(short x) {show_bytes((byte_pointer) &x, sizeof(short));}
void show_long(long x) {show_bytes((byte_pointer) &x, sizeof(long));}
void show_double(double x) {show_bytes((byte_pointer) &x, sizeof(double));}

int main() {
    short a = 1024;
    long b = -1;
    double c = 3.1415123861238712361293981369123128361;
    show_short(a);
    show_long(b);
    show_double(c);
}

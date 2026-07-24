/* Determine whether arguments can be added without
overflow */

#include <stdio.h>
int uadd_ok(unsigned x, unsigned y) { return x + y > x; }

int main() {
  unsigned x = 0x8fffffff;
  unsigned y = 0x70000001;

  printf("%u\n", x);
  printf("%u\n", y);

  printf("%u\n", uadd_ok(x, y));

  printf("%u\n", x + y);
}

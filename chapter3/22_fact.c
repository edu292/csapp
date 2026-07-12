#include <stdio.h>
long fact_do(int n) {
  long result = 1;
  do {
    result *= n;
    n = n - 1;
  } while (n > 1);
  return result;
}

long fact_do_goto(long n) {
  long result = 1;
loop:
  result *= n;
  n = n - 1;
  if (n > 1)
    goto loop;
  return result;
}

int main() {
    printf("%064lb\n", fact_do(13));
    printf("%ld\n", fact_do(13));
}

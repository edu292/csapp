#include <stdio.h>

float sum_elements(float a[], unsigned length) {
  float result = 0;
  for (unsigned i = 0; i < length; i++) {
    result += a[i];
  }

  return result;
}

int main() {
  float a[0];
  float sum = sum_elements(a, sizeof(a) / sizeof(float));
  printf("%f", sum);
}

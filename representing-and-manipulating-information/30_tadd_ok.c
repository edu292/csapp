#include <stdint.h>
#include <stdio.h>

int tadd_ok(int x, int y) {
    if (x > 0 && y > 0) {
        return y < INT32_MAX - x;
    } else if (x < 0 && y < 0) {
        return x > INT32_MIN + y;
    }

    return 1;
}

int main() {
    int a = 0x7FFFFFFF;
    int b = 0x80000001;
    printf("%b", tadd_ok(a, b));
}

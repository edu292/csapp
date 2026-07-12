#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

bool tmul_ok(int x, int y) {
    uint64_t res = (uint64_t)x * y;
    return res >= INT32_MIN && res <= INT32_MAX;
}

int main() {
    printf("%zu", sizeof(size_t));
}

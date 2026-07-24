#include <stdint.h>
#include <stdio.h>

uintptr_t lsb_x_to_y(uintptr_t x, uintptr_t y) {
    return (y & ~0xFF) | (x & 0xFF);
}

int main() {
    uintptr_t x = 0x89ABCDEF;
    uintptr_t y = 0x76543210;
    uintptr_t res = lsb_x_to_y(x, y);

    printf("%.2lX", res);
}

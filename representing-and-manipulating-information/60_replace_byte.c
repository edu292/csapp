#include <stdio.h>

unsigned replace_byte(unsigned x, unsigned short i, unsigned char b) {
    unsigned bits_move = i * 8;
    return (x & ~(0xFF << bits_move)) + (b << bits_move);
}

int main() {
    printf("%.2X\n", replace_byte(0x12345678, 2, 0xAB));
    printf("%.2X\n", replace_byte(0x12345678, 0, 0xAB));
}

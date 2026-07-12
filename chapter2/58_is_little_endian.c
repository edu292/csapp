# include <stdbool.h>
#include <stdio.h>

bool is_little_endian() {
    short a = 1;
    return *(char *)&a == 1;
}


int main() {
    printf("%b", is_little_endian());
    return 0;
}

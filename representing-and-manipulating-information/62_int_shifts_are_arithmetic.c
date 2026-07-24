#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

bool int_shifts_are_arithmetic() {
    int x = -1;
    return (x >> 1) == -1;
}

int main() {
    printf("%b", int_shifts_are_arithmetic());
}

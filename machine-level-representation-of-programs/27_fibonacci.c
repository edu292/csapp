#include <stdio.h>
void fibonacci(unsigned len) {
    unsigned long a = 0;
    unsigned long b = 1;
    unsigned long t;
    unsigned i = 0;
    while (i < len) {
        printf("%lu\n", a);
        t = b;
        b = a + b;
        a = t;
        i++;
    }
}

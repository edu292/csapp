#include <stdio.h>
#define POS_INFINITY (*(double *)&(unsigned long){0x7FF0000000000000})
#define NEG_INFINITY (*(double *)&(unsigned long){0xFFF0000000000000})
#define NEG_ZERO (*(double *)&(unsigned long){0x8000000000000000})

int main() {
    printf("%lf\n", POS_INFINITY);
    printf("%lf\n", NEG_INFINITY);
    printf("%lf\n", NEG_ZERO);

    return 0;
}


#include <stdbool.h>

void cond(short a, short *p) {
  if (a && *p < a)
    *p = a;
}

void cond_goto(short a, short *p) {
    bool t = a && * p < a;
    if (!t) goto done;
    *p = a;
done:
    return;
}

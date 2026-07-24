#include <stdio.h>
#include <string.h>

int strlonger(char *s, char *t) {
    return strlen(s) - strlen(t) > 0;
}

int main() {
    char a[] = "Hello, World";
    char b[] = "Hello, World";
    printf("%b", strlonger(a, b));
}

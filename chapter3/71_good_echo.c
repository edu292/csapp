#include <limits.h>
#include <stdio.h>

#define BUFFER_SIZE 2

void good_echo() {
  char buffer[BUFFER_SIZE];

  while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
    if (fputs(buffer, stdout) == EOF) {
      return;
    };
  }
}

int main() {
  while (1) {
    good_echo();
    printf("\n");
  }
  return 0;
}

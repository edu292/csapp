#include <stdint.h>

long decode2(long x, long y, long z) {
    y -= z;
    return  y * x ^ ((y << 63) >> 63); 
}

/*
decode2:
  subq %rdx, %rsi
  imulq %rsi, %rdi
  movq %rsi, %rax
  salq $63, %rax
  sarq $63, %rax
  xorq %rdi, %rax
  ret
*/

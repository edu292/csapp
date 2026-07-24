short loop_while(short a, short b) {
  short result = 0;
  while (a > b) {
    result = result + a + b;
    a = a - 1;
  }
  return result;
}

/*
short loop_while(short a, short b)
a in %rdi, b in %rsi
loop_while:
  movl $0, %eax
  jmp .L2
.L3:
  leaq (,%rsi,%rdi), %rdx
  addq %rdx, %rax
  subq $1, %rdi
.L2:
  cmpq %rsi, %rdi
  jg .L3
  rep; ret
 */

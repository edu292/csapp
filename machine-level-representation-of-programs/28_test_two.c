short test_two(unsigned short x) {
  long val = 0;
  for (short i = 64; i != 0; i--) {
      val = (val << 1) | (x & 0x1);
      x >>= 1;
  }
  return val;
}

/*
test fun_b(unsigned test x)
x in %rdi
test_two:
  movl $1, %edx
  movl $65, %eax
.L10:
  movq %rdi, %rcx
  andl $1, %ecx
  addq %rax, %rax
  orq %rcx, %rax
  shrq %rdi
  addq $1, %rdx
  jne .L10
  rep; ret
*/

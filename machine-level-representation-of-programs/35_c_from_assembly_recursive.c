long rfun(unsigned long x) {
  if (x == 0)
    return 0;
  unsigned long nx = x >> 2;
  long rv = rfun(nx);
  return x + rv;
}

/*
long rfun(unsigned long x)
x in %rdi
rfun:
  pushq %rbx
  movq %rdi, %rbx
  movl $0, %eax
  testq %rdi, %rdi
  je .L2
  shrq $2, %rdi
  call rfun
  addq %rbx, %rax
.L2:
  popq %rbx
  ret
*/

/*
A) The value stored in %rbx is x(%rdi)
*/

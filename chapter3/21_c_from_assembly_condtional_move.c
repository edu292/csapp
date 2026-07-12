short test(short x, short y) {
  short val = y + 12;
  if (x < 0) {
    if (x < y)
      val = x * y;
    else
      val = x | y;
  } else if (y >= 10) {
    val = x / y;
  }
  return val;
}

/*
short test(short x, short y)
x in %rdi, y in %rsi
test:
  leaq 12(%rsi), %rax
  testq %rdi, %rdi
  jge .L2
  movq %rdi, %rax
  imulq %rsi, %rax
  movq %rdi, %rdx
  orq %rsi, %rdx
  cmpq %rsi, %rdi
  cmovge %rdx, %rax
  ret
.L2:
  idivq %rsi, %rdi
  cmpq $10, %rsi
  cmovge %rdi, %rax
  ret
*/

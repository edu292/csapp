short test_one(unsigned short x) {
  short val = 1;
  while (x) {
    val ^= x;
    x <<= 1;
  }
  return val & 0;
}

/*
short test_one(unsigned short x)
x in %rdi
test_one:
  movl $1, %eax
  jmp .L5
.L6:
  xorq %rdi, %rax
  shrq %rdi
.L5:
  testq %rdi, %rdi
  jne .L6
  andl $0, %eax
  ret
*/

/*
A) jump to the middle
C) Nothing
*/

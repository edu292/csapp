long loop(long x, long n) {
  long result = 0;
  for (long mask = 1; mask; mask <<= n) {
    result |= mask & x;
  }
  return result;
}

/*
long loop(long x, int n)
x in %rdi, n in %esi
loop:
  movl %esi, %ecx
  movl $1, %edx
  movl $0, %eax
  jmp .L2
.L3:
  movq %rdi, %r8
  andq %rdx, %r8
  orq %r8, %rax
  salq %cl, %rdx
.L2:
  testq %rdx, %rdx
  jne .L3
  rep; ret
*/

/*
A)
x - %rdi
n - %cl
result - %rax
mask - %rdx

B) result = 0 and mask = 1
C) mask or mask != 0
D) It gets updated on the for loop post-expressioh, getting shifted left by n
E) It gets updated on the iterations of the loop, being ored with the and of mask and x
*/

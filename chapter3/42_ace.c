struct ACE {
  short v;
  struct ACE *p;
};

short test(struct ACE *ptr) {
    int result = 1;
    while (ptr) {
        result *= ptr->v;
        ptr = ptr->p;
    }

    return result;
};

/*
short test(struct ACE *ptr)
ptr in %rdi
test:
  movl $1, %eax
  jmp .L2
.L3:
  imulq (%rdi), %rax
  movq 2(%rdi), %rdi
.L2:
  testq %rdi, %rdi
  jne .L3
  rep; ret
*/

/*
B) the ACE struct corresponds to a linked list and test implements a multiplication aggregation on it.
*/

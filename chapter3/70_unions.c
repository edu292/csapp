union ele {
  struct {
    long *p;
    long y;
  } e1;
  struct {
    long x;
    union ele *next;
  } e2;
};

void proc(union ele *up) { 
    up->e2.x = *(up->e2.next->e1.p) - up->e2.next->e1.y; 
}

/*
void proc (union ele *up)
up in %rdi
proc:
  movq 8(%rdi), %rax
  movq (%rax), %rdx
  movq (%rdx), %rdx
  subq 8(%rax), %rdx
  movq %rdx, (%rdi)
  ret
*/

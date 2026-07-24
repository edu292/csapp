struct test {
  short *p;
  struct {
    short x;
    short y;
  } s;
  struct test *next;
};

void st_init(struct test *st) {
  st->s.y = st->s.x;
  st->p = &st->s.y;
  st->next = st;
}

/*
assembly:
void st_init(struct test *st)
st in %rdi
st_init:
  movl 8(%rdi), %eax
  movl %eax, 10(%rdi)
  leaq 10(%rdi), %rax
  movq %rax, (%rdi)
  movq %rdi, 12(%rdi)
  ret
*/

/*
A)
p: 0
s.x = 8
x.y = 10
next = 16

B) 24 bytes
*/

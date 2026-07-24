int procprob(int a, char b, long *u, char *v) {
    *u += a;
    *v += b;
    return sizeof(a) + sizeof(b);
}

/*
procprob:
  movslq %edi, %rdi
  addq %rdi, (%rdx)
  addb %sil, (%rcx)
  movl $6, %eax
  ret
*/

#define N 16
typedef int fix_matrix[N][N];

void fix_set_diag(fix_matrix A, int val) {
  long i;
  for (i = 0; i < N; i++)
    A[i][i] = val;
}

void fix_set_diag_opt(fix_matrix A, int val) {
  int *diag_ptr = &A[0][0];
  int *end_ptr = &A[N][N];
  do {
    *diag_ptr = val;
    diag_ptr += N + 1;
  } while (diag_ptr != end_ptr);
}

/*
void fix_set_diag(fix_matrix A, int val)
A in %rdi, val in %rsi
fix_set_diag:
  movl $0, %eax
.L13:
  movl %esi, (%rdi,%rax)
  addq $68, %rax
  cmpq $1088, %rax
  jne .L13
  rep; ret
*/

double fcvt2(int *ip, float *fp, double *dp, long l) {
  int i = *ip;
  float f = *fp;
  double d = *dp;
  *ip = (int)d;
  *fp = (float)i;
  *dp = (double)l;
  return (double)f;
}

/*
double fcvt2(int *ip, float *fp, double *dp, long l)
ip in %rdi, fp in %rsi, dp in %rdx, l in %rcx
Result returned in %xmm0
fcvt2:
  movl (%rdi), %eax
  vmovss (%rsi), %xmm0
  vcvttsd2si (%rdx), %r8d
  movl %r8d, (%rdi)
  vcvtsi2ss %eax, %xmm1, %xmm1
  vmovss %xmm1, (%rsi)
  vcvtsi2sdq %rcx, %xmm1, %xmm1
  vmovsd %xmm1, (%rdx)
  vunpcklps %xmm0, %xmm0, %xmm0
  vcvtps2pd %xmm0, %xmm0
  ret
*/

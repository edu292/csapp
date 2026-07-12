double funct3(int *ap, double b, long c, float *dp) {
  float a = (double)*ap;
  float d = *dp;
  float cf = (float)c;
  double result;

  if (b > a) {
    result = d * cf;
  } else {
    result = 2 * d + cf;
  }

  return result;
}
/*
double funct3(int *ap, double b, long c, float *dp)
ap in %rdi, b in %xmm0, c in %rsi, dp in %rdx
funct3:
  vmovss (%rdx), %xmm1
  vcvtsi2sd (%rdi), %xmm2, %xmm2
  vucomis %xmm2, %xmm0
  jbe .L8
  vcvtsi2ssq %rsi, %xmm0, %xmm0
  vmulss %xmm1, %xmm0, %xmm1
  vunpcklps %xmm1, %xmm1, %xmm1
  vcvtps2pd %xmm1, %xmm0
  ret
.L8:
  vaddss %xmm1, %xmm1, %xmm1
  vcvtsi2ssq %rsi, %xmm0, %xmm0
  vaddss %xmm1, %xmm0, %xmm0
  vunpcklps %xmm0, %xmm0, %xmm0
  vcvtps2pd %xmm0, %xmm0
  ret
*/

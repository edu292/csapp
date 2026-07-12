double funct2(double w, int x, float y, long z) {
    return x * y - z / w;
}
/*
double funct2(double w, int x, float y, long z)
w in %xmm0, x in %edi, y in %xmm1, z in %rsi
funct2:
  vcvtsi2ss %edi, %xmm2, %xmm2       Convert Signed Integer 2 Scalar Single Precision
  vmulss %xmm1, %xmm2, %xmm1
  vunpcklps %xmm1, %xmm1, %xmm1
  vcvtps2pd %xmm1, %xmm2
  vcvtsi2sdq %rsi, %xmm1, %xmm1      Convert Signed Quadword Integer 2 Scalar Double Precision
  vdivsd %xmm1, %xmm0, %xmm0
  vsubsd %xmm0, %xmm2, %xmm0
  ret
*/

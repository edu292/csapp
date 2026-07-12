.global find_range
find_range:
  xorps %xmm1, %xmm1
  ucomiss %xmm1, %xmm0
  jp .L1
  jb .L2
  ja .L3
  movl $1, %eax
  ret
.L1:
  movl $3, %eax
  ret
.L2:
  movl $0, %eax
  ret
.L3:
  movl $2, %eax
  ret

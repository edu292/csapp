.global find_range
find_range:
  xorps %xmm1, %xmm1
  ucomiss %xmm1, %xmm0
  movl $1, %eax
  movl $0, %edx
  cmovb %edx, %eax

  movl $2, %edx
  cmova %edx, %eax

  movl $3, %edx
  cmovp %edx, %eax
  ret

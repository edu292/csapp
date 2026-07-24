fcvt2:
	movl	(%rdi), %eax
	vxorps	%xmm1, %xmm1, %xmm1
	vcvttsd2sil	(%rdx), %r8d
	vmovss	(%rsi), %xmm0
	vcvtsi2ssl	%eax, %xmm1, %xmm2
	vcvtsi2sdq	%rcx, %xmm1, %xmm1
	vcvtss2sd	%xmm0, %xmm0, %xmm0
	movl	%r8d, (%rdi)
	vmovss	%xmm2, (%rsi)
	vmovlpd	%xmm1, (%rdx)
	ret

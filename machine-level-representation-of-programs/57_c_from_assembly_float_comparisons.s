funct3:
	vxorps	%xmm1, %xmm1, %xmm1
	vmovapd	%xmm0, %xmm3
	vmovss	(%rdx), %xmm2
	vcvtsi2ssq	%rsi, %xmm1, %xmm0
	vcvtsi2ssl	(%rdi), %xmm1, %xmm1
	vcvtss2sd	%xmm1, %xmm1, %xmm1
	vcomisd	%xmm1, %xmm3
	jbe	.L6
	vmulss	%xmm0, %xmm2, %xmm0
	vcvtss2sd	%xmm0, %xmm0, %xmm0
	ret
.L6:
	vaddss	%xmm2, %xmm2, %xmm2
	vaddss	%xmm0, %xmm2, %xmm0
	vcvtss2sd	%xmm0, %xmm0, %xmm0
	ret

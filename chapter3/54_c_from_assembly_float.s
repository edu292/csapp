funct2:
	vmovapd	%xmm0, %xmm3
	vxorps	%xmm2, %xmm2, %xmm2
	vcvtsi2ssl	%edi, %xmm2, %xmm0
	vcvtsi2sdq	%rsi, %xmm2, %xmm2
	vdivsd	%xmm3, %xmm2, %xmm2
	vmulss	%xmm1, %xmm0, %xmm0
	vcvtss2sd	%xmm0, %xmm0, %xmm0
	vsubsd	%xmm2, %xmm0, %xmm0
	ret

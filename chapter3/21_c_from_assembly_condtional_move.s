test:
	leal	12(%rsi), %eax
	testw	%di, %di
	js	.L5
	cmpw	$9, %si
	jle	.L1
	movswl	%di, %eax
	movswl	%si, %esi
	cltd
	idivl	%esi
.L1:
	ret
.L5:
	cmpw	%di, %si
	jle	.L3
	movl	%edi, %eax
	imull	%esi, %eax
	ret
.L3:
	movl	%esi, %eax
	orl	%edi, %eax
	ret

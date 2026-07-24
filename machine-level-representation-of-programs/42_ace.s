test:
	movl	$1, %eax
	jmp	.L2
.L3:
	movswl	(%rdi), %edx
	imull	%edx, %eax
	movq	8(%rdi), %rdi
.L2:
	testq	%rdi, %rdi
	jne	.L3
	ret

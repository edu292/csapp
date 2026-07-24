switch_prob:
	subq	$60, %rsi
	cmpq	$5, %rsi
	ja	.L2
	leaq	.L4(%rip), %rdx
	movslq	(%rdx,%rsi,4), %rax
	addq	%rdx, %rax
	jmp	*%rax
.L4:
	.long	.L7-.L4
	.long	.L2-.L4
	.long	.L7-.L4
	.long	.L6-.L4
	.long	.L5-.L4
	.long	.L3-.L4
.L7:
	leaq	0(,%rdi,8), %rax
	ret
.L6:
	movq	%rdi, %rax
	sarq	$3, %rax
	ret
.L5:
	movq	%rdi, %rax
	salq	$4, %rax
	subq	%rdi, %rax
	movq	%rax, %rdi
.L3:
	imulq	%rdi, %rdi
.L2:
	leaq	76(%rdi), %rax
	ret

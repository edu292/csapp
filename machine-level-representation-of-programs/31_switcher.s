switcher:
	cmpq	$7, %rdi
	ja	.L2
	leaq	.L4(%rip), %rsi
	movslq	(%rsi,%rdi,4), %rax
	addq	%rsi, %rax
	jmp	*%rax
.L4:
	.long	.L7-.L4
	.long	.L2-.L4
	.long	.L3-.L4
	.long	.L2-.L4
	.long	.L6-.L4
	.long	.L5-.L4
	.long	.L2-.L4
	.long	.L3-.L4
.L5:
	movq	%rdi, %rdx
	xorq	$15, %rdx
.L7:
	leaq	112(%rdx), %rdi
	jmp	.L6
.L3:
	addq	%rdx, %rdi
	salq	$2, %rdi
	jmp	.L6
.L2:
	movq	(%rcx), %rdi
.L6:
	movq	%rdi, (%rcx)
	ret

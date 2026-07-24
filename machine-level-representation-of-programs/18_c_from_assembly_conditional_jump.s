test:
	movq	%rdx, %rcx
	leaq	(%rdx,%rsi), %rax
	subq	%rdi, %rax
	cmpq	$5, %rdx
	jle	.L2
	cmpq	$2, %rsi
	jle	.L3
	movq	%rdi, %rax
	cqto
	idivq	%rcx
	ret
.L3:
	movq	%rdi, %rax
	cqto
	idivq	%rsi
	ret
.L2:
	cmpq	$2, %rdx
	jg	.L1
	movq	%rdx, %rax
	cqto
	idivq	%rsi
.L1:
	ret

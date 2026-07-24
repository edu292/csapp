switch3:
	cmpl	$4, %edx
	ja	.L10
	movl	%edx, %edx
	leaq	.L4(%rip), %rcx
	movslq	(%rcx,%rdx,4), %rax
	addq	%rcx, %rax
	jmp	*%rax
.L4:
	.long	.L8-.L4
	.long	.L7-.L4
	.long	.L6-.L4
	.long	.L5-.L4
	.long	.L9-.L4
.L8:
	movq	(%rsi), %rax
	movq	(%rdi), %rdx
	movq	%rdx, (%rsi)
	ret
.L7:
	movq	(%rsi), %rax
	addq	(%rdi), %rax
	movq	%rax, (%rdi)
	ret
.L6:
	movq	$59, (%rdi)
	movq	(%rsi), %rax
	ret
.L5:
	movq	(%rsi), %rax
	movq	%rax, (%rdi)
.L9:
	movl	$27, %eax
	ret
.L10:
	movl	$12, %eax
	ret

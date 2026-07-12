proc:
	movq	8(%rdi), %rdx
	movq	(%rdx), %rax
	movq	(%rax), %rax
	subq	8(%rdx), %rax
	movq	%rax, (%rdi)
	ret

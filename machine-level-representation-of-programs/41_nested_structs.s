st_init:
	movzwl	8(%rdi), %eax
	movw	%ax, 10(%rdi)
	leaq	10(%rdi), %rax
	movq	%rax, (%rdi)
	movq	%rdi, 16(%rdi)
	ret

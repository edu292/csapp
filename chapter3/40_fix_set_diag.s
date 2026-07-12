fix_set_diag:
	movq	%rdi, %rax
	leaq	1088(%rdi), %rdx
.L2:
	movl	%esi, (%rax)
	addq	$68, %rax
	cmpq	%rdx, %rax
	jne	.L2
	ret

fix_set_diag_opt:
	movq	%rdi, %rax
	leaq	1088(%rdi), %rdx
.L5:
	movl	%esi, (%rax)
	addq	$68, %rax
	cmpq	%rdx, %rax
	jne	.L5
	ret

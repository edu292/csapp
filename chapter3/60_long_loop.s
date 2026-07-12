loop:
	movq	%rsi, %rcx
	movl	$1, %eax
	movl	$0, %edx
	jmp	.L2
.L3:
	movq	%rax, %r8
	andq	%rdi, %r8
	orq	%r8, %rdx
	salq	%cl, %rax
.L2:
	testq	%rax, %rax
	jne	.L3
	movq	%rdx, %rax
	ret

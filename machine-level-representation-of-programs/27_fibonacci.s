fibonacci:
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$8, %rsp
	movl	%edi, %r13d
	movl	$0, %ebx
	movl	$1, %r12d
	movl	$0, %ebp
	jmp	.L2
.L3:
	movq	%rbp, %rsi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leaq	0(%rbp,%r12), %rax
	addl	$1, %ebx
	movq	%r12, %rbp
	movq	%rax, %r12
.L2:
	cmpl	%r13d, %ebx
	jb	.L3
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	ret

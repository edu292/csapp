rfun:
	testq	%rdi, %rdi
	jne	.L8
	movl	$0, %eax
	ret
.L8:
	pushq	%rbx
	movq	%rdi, %rbx
	shrq	$2, %rdi
	call	rfun
	addq	%rbx, %rax
	popq	%rbx
	ret

cread:
	testq	%rdi, %rdi
	je	.L3
	movq	(%rdi), %rax
	ret
.L3:
	movl	$0, %eax
	ret

cread_alt:
	subq	$24, %rsp
	movq	$0, (%rsp)
	testq	%rdi, %rdi
	je	.L9
.L10:
	movq	(%rdi), %rax
	addq	$24, %rsp
	ret
.L9:
	movq	%rsp, %rdi
	jmp	.L5

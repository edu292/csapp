bubble_a:
	leaq	-1(%rsi), %r9
	jmp	.L2
	.p2align 5
.L3:
	addq	$1, %rax
.L5:
	cmpq	%r9, %rax
	jge	.L7
	leaq	8(%rdi,%rax,8), %rsi
	movq	(%rsi), %rcx
	leaq	(%rdi,%rax,8), %rdx
	movq	(%rdx), %r8
	cmpq	%r8, %rcx
	jge	.L3
	movq	%r8, (%rsi)
	movq	%rcx, (%rdx)
	jmp	.L3
.L7:
	subq	$1, %r9
.L2:
	testq	%r9, %r9
	jle	.L8
	movl	$0, %eax
	jmp	.L5
.L8:
	ret

bubble_p:
	leaq	-8(%rdi,%rsi,8), %rsi
	jmp	.L10
	.p2align 5
.L11:
	addq	$8, %rax
.L13:
	cmpq	%rax, %rsi
	je	.L15
	movq	(%rax), %rcx
	movq	8(%rax), %rdx
	cmpq	%rdx, %rcx
	jle	.L11
	movq	%rcx, 8(%rax)
	movq	%rdx, (%rax)
	jmp	.L11
.L15:
	subq	$8, %rsi
.L10:
	cmpq	%rdi, %rsi
	je	.L16
	movq	%rdi, %rax
	jmp	.L13
.L16:
	ret

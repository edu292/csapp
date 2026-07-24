.global bubble_p_swap_one_cmov
bubble_p_swap_one_cmov:
	leaq	-8(%rdi,%rsi,8), %rsi
	jmp	.L10
.L11:
	addq	$8, %rax
.L13:
	cmpq	%rax, %rsi
	je	.L15
	movq	(%rax), %rcx
	movq	8(%rax), %rdx
    movq	%rcx, %r8
    xorq	%rdx, %r8
    movq	$0, %r9
    cmpq	%rdx, %rcx
    cmovle	%r9, %r8
	xorq	%r8, %rcx
	xorq	%r8, %rdx
    movq	%rdx, 8(%rax)
    movq	%rcx, (%rax)
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


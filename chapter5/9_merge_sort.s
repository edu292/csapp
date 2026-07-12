merge_move:
	movq	%rdi, %r11
	movq	%rsi, %r10
	movl	$0, %eax
	movl	$0, %esi
	movl	$0, %edi
	jmp	.L18
.L19:
	addq	$1, %rsi
	movq	%r9, %r8
.L20:
	movq	%r8, (%rdx,%rax,8)
	leaq	1(%rax), %rax
.L18:
	cmpq	%rsi, %rdi
	movq	%rsi, %r8
	cmovge	%rdi, %r8
	cmpq	%rcx, %r8
	jge	.L22
	movq	(%r11,%rdi,8), %r8
	movq	(%r10,%rsi,8), %r9
	cmpq	%r9, %r8
	jge	.L19
	addq	$1, %rdi
	jmp	.L20
.L23:
	movq	(%r11,%rdi,8), %r8
	movq	%r8, (%rdx,%rax,8)
	leaq	1(%rax), %rax
	leaq	1(%rdi), %rdi
.L22:
	cmpq	%rcx, %rdi
	jl	.L23
	jmp	.L24
.L25:
	movq	(%r10,%rsi,8), %rdi
	movq	%rdi, (%rdx,%rax,8)
	leaq	1(%rax), %rax
	leaq	1(%rsi), %rsi
.L24:
	cmpq	%rcx, %rsi
	jl	.L25
	ret

mergesort_move:
	leaq	-40(%rsp), %rsp
	movq	%r12, 16(%rsp)
	movq	%rdi, %r12
	cmpq	$1, %rsi
	movq	%rbx, (%rsp)
	movq	%rbp, 8(%rsp)
	movq	%r13, 24(%rsp)
	movq	%r14, 32(%rsp)
	movq	%rsi, %r13
	movq	%rsi, %rbp
	shrq	$63, %rbp
	addq	%rsi, %rbp
	sarq	%rbp
	movl	$8, %edx
	movq	%rbp, %rsi
	call	array_copy
	movq	%rax, %r14
	subq	%rbp, %r13
	leaq	(%r12,%rbp,8), %rdi
	movl	$8, %edx
	movq	%r13, %rsi
	call	array_copy
	movq	%rax, %rbx
	movq	%r13, %rsi
	movq	%rax, %rdi
	call	mergesort_move
	movq	%rax, %r13
	movq	%rbp, %rsi
	movq	%r14, %rdi
	call	mergesort_move
	movq	%rax, %rdi
	movq	%rbp, %rcx
	movq	%r12, %rdx
	movq	%r13, %rsi
	call	merge_move
	movq	%r14, %rdi
	call	free@PLT
	movq	%rbx, %rdi
	call	free@PLT
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	24(%rsp), %r13
	movq	32(%rsp), %r14
.L27:
	movq	%r12, %rax
	movq	16(%rsp), %r12
	leaq	40(%rsp), %rsp
	ret

merge_jump:
	movq	%rdi, %r11
	movq	%rsi, %r10
	movl	$0, %eax
	movl	$0, %esi
	movl	$0, %edi
	jmp	.L30
.L31:
	addq	$1, %rsi
	movq	%r8, (%rdx,%rax,8)
	leaq	1(%rax), %rax
.L30:
	cmpq	%rsi, %rdi
	movq	%rsi, %r8
	cmovge	%rdi, %r8
	cmpq	%rcx, %r8
	jge	.L34
	movq	(%r11,%rdi,8), %r9
	movq	(%r10,%rsi,8), %r8
	cmpq	%r8, %r9
	jge	.L31
	addq	$1, %rdi
	movq	%r9, (%rdx,%rax,8)
	leaq	1(%rax), %rax
	jmp	.L30
.L35:
	movq	(%r11,%rdi,8), %r8
	movq	%r8, (%rdx,%rax,8)
	leaq	1(%rax), %rax
	leaq	1(%rdi), %rdi
.L34:
	cmpq	%rcx, %rdi
	jl	.L35
	jmp	.L36
.L37:
	movq	(%r10,%rsi,8), %rdi
	movq	%rdi, (%rdx,%rax,8)
	leaq	1(%rax), %rax
	leaq	1(%rsi), %rsi
.L36:
	cmpq	%rcx, %rsi
	jl	.L37
	ret

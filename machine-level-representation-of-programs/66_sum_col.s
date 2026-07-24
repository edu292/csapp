sum_col:
	testq	%rdi, %rdi
	jg	.L7
	xorl	%ecx, %ecx
	movq	%rcx, %rax
	ret
.L7:
	leaq	1(,%rdi,4), %r8
	leaq	(%rsi,%rdx,8), %rdx
	xorl	%ecx, %ecx
	xorl	%eax, %eax
	salq	$3, %r8
	leaq	(%rdi,%rdi,2), %rsi
.L3:
	addq	$1, %rax
	addq	(%rdx), %rcx
	addq	%r8, %rdx
	cmpq	%rsi, %rax
	jne	.L3
	movq	%rcx, %rax
	ret

loop_while2:
	testq	%rsi, %rsi
	jle	.L4
	movq	%rsi, %rax
.L3:
	imulq	%rdi, %rax
	subq	%rdi, %rsi
	testq	%rsi, %rsi
	jg	.L3
	ret
.L4:
	movq	%rsi, %rax
	ret

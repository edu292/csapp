decode2:
	subq	%rdx, %rsi
	imulq	%rsi, %rdi
	sall	$7, %esi
	sarb	$7, %sil
	movsbq	%sil, %rsi
	movq	%rdi, %rax
	xorq	%rsi, %rax
	ret

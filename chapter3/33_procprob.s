procprob:
	movslq	%edi, %rdi
	addq	%rdi, (%rdx)
	addw	%si, (%rcx)
	movl	$6, %eax
	ret

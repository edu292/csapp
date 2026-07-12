test_two:
	movl	$64, %edx
	movl	$0, %eax
.L3:
	addq	%rax, %rax
	movq	%rdi, %rcx
	andl	$1, %ecx
	orq	%rcx, %rax
	shrw	%di
	subl	$1, %edx
	testw	%dx, %dx
	jne	.L3
	ret

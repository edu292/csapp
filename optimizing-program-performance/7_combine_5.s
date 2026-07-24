combine5:
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdi, %r13
	movq	%rsi, %r12
	call	vec_length
	movq	%rax, %rbp
	leaq	-4(%rax), %rbx
	movq	%r13, %rdi
	call	get_vec_start
	vmovsd	.LC0(%rip), %xmm0
	movl	$0, %edx
	jmp	.L4
.L5:
	vmulsd	(%rax,%rdx,8), %xmm0, %xmm0
	vmulsd	8(%rax,%rdx,8), %xmm0, %xmm0
	vmulsd	16(%rax,%rdx,8), %xmm0, %xmm0
	vmulsd	24(%rax,%rdx,8), %xmm0, %xmm0
	vmulsd	32(%rax,%rdx,8), %xmm0, %xmm0
	addq	$5, %rdx
.L4:
	cmpq	%rbx, %rdx
	jl	.L5
	jmp	.L6
.L7:
	vmulsd	(%rax,%rdx,8), %xmm0, %xmm0
	addq	$1, %rdx
.L6:
	cmpq	%rbp, %rdx
	jl	.L7
	vmovsd	%xmm0, (%r12)
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	ret

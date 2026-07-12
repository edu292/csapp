loop_while:
	movl	$0, %eax
	jmp	.L2
.L3:
	addl	%edi, %eax
	addl	%esi, %eax
	subl	$1, %edi
.L2:
	cmpw	%si, %di
	jg	.L3
	ret

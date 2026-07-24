cond:
	testw	%di, %di
	je	.L1
	cmpw	%di, (%rsi)
	jge	.L1
	movw	%di, (%rsi)
.L1:
	ret
cond_goto:
	testw	%di, %di
	je	.L3
	cmpw	%di, (%rsi)
	jge	.L3
	movw	%di, (%rsi)
.L3:
	ret

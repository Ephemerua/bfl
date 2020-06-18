
.global bfl_read, bfl_close, bfl_exit, bfl_wait4, bfl_write, bfl_fork, bfl_shmat, bfl_open

bfl_read:
	movq $0, %rax
	syscall
	ret

bfl_open:
	movq $2, %rax
	syscall
	ret

bfl_write:
	movq $1, %rax
	syscall
	ret

bfl_shmat:
	movq $30, %rax
	xorq %rsi, %rsi
	syscall
	ret

bfl_wait4:
	movq $61, %rax
	xchgq %rcx, %r10
	syscall
	ret

bfl_exit:
	mov $60, %rax
	syscall
	ret

bfl_close:
	movq $3, %rax
	syscall
	ret

bfl_fork:
	movq $57, %rax
	syscall
	ret



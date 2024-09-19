	.global _start
	.hidden _start
	.type _start, @function
_start:
	xor %ebp, %ebp
	mov %rsp, %rdi
	call ldso_main

	.size _start, . - _start

	.global _start
	.type _start, @function
_start:
	xor	%ebp, %ebp
	mov	%rdx, %r9
	pop	%rsi
	mov	%rsp, %rdx
	and	$0xfffffffffffffff0, %rsp
	push	%rax
	push	%rsp
	lea	__libc_csu_fini(%rip),%r8
	lea	__libc_csu_init(%rip),%rcx
	lea	main(%rip),%rdi
	callq 	__libc_start_main
	.size _start, . - _start

	.global __libc_csu_init
	.type __libc_csu_init, @function
__libc_csu_init:
	/* FIXME: missing code here */
	ret
	.size __libc_csu_init, . - __libc_csu_init

	.global __libc_csu_fini
	.type __libc_csu_fini, @function
__libc_csu_fini:
	ret
	.size __libc_csu_fini, . - __libc_csu_fini

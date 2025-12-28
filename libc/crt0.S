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
    hlt                   # safety net (never reached)
    .size _start, . - _start

    .section .text
    .global __libc_csu_init
    .type __libc_csu_init, @function
__libc_csu_init:
    # Call all constructors in .init_array
    # __init_array_start and __init_array_end are linker-provided symbols
    lea     __init_array_start(%rip), %rsi
    lea     __init_array_end(%rip), %rdx
1:  cmp     %rsi, %rdx
    jge     2f                      # done if reached end
    mov     (%rsi), %rax
    call    *%rax
    add     $8, %rsi
    jmp     1b
2:  ret
    .size __libc_csu_init, . - __libc_csu_init

    .global __libc_csu_fini
    .type __libc_csu_fini, @function
__libc_csu_fini:
    ret
    .size __libc_csu_fini, . - __libc_csu_fini
#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

static inline i64 syscall6(u64 syscall_nr, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6)
{
	i64 ret;

	asm volatile ("mov %[arg4], %%r10\n"
		      "mov %[arg5], %%r8\n"
		      "mov %[arg6], %%r9\n"
		      "syscall\n"
		      : "=a"(ret)
		      : "a"(syscall_nr), "D"(arg1), "S"(arg2), "d"(arg3),
		      [arg4]"r"(arg4), [arg5]"r"(arg5), [arg6]"r"(arg6)
		      : "memory", "r10", "r8", "r9", "rcx", "r11");

	return ret;
}

static inline i64 syscall5(u64 syscall_nr, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	i64 ret;

	asm volatile ("mov %[arg4], %%r10\n"
			"mov %[arg5], %%r8\n"
			"syscall\n"
			: "=a"(ret)
			: "a"(syscall_nr), "D"(arg1), "S"(arg2), "d"(arg3),
			[arg4]"r"(arg4), [arg5]"r"(arg5)
			: "memory", "r10", "r8", "rcx", "r11");

	return ret;
}

static inline i64 syscall4(u64 syscall_nr, u64 arg1, u64 arg2, u64 arg3, u64 arg4)
{
	i64 ret;

	asm volatile ("mov %[arg4], %%r10\n"
			"syscall\n"
			: "=a"(ret)
			: "a"(syscall_nr), "D"(arg1), "S"(arg2), "d"(arg3),
			[arg4]"r"(arg4)
			: "memory", "r10", "rcx", "r11");

	return ret;
}

static inline i64 syscall3(u64 syscall_nr, u64 arg1, u64 arg2, u64 arg3)
{
	i64 ret;

	asm volatile ("syscall\n"
			: "=a"(ret)
			: "a"(syscall_nr), "D"(arg1), "S"(arg2), "d"(arg3)
			: "memory", "rcx", "r11");

	return ret;
}

static inline i64 syscall2(u64 syscall_nr, u64 arg1, u64 arg2)
{
	i64 ret;

	asm volatile ("syscall\n"
			: "=a"(ret)
			: "a"(syscall_nr), "D"(arg1), "S"(arg2)
			: "memory", "rcx", "r11");

	return ret;
}

static inline i64 syscall1(u64 syscall_nr, u64 arg1)
{
	i64 ret;

	asm volatile ("syscall\n"
			: "=a"(ret)
			: "a"(syscall_nr), "D"(arg1)
			: "memory", "rcx", "r11");

	return ret;
}

#endif /* !SYSCALL_H */

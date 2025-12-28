#include "unistd.h"
#include "stdio.h"


typedef long i64;
typedef unsigned long u64;
#define __NR_write 1


static inline i64 call3(u64 syscall_nr, u64 arg1, u64 arg2, u64 arg3)
{
	i64 ret;

	asm volatile ("syscall\n"
			: "=a"(ret)
			: "a"(syscall_nr), "D"(arg1), "S"(arg2), "d"(arg3)
			: "memory", "rcx", "r11");

	return ret;
}

i64 sys_write(int fd, const void *buf, size_t len)
{
	return call3(__NR_write, fd, (u64)buf, len);
}


void __libc_start_main(int (*main)(int, char **, char **),
		       int argc, char **argv,
		       void *csu_init, void *csu_fini, void *dl_fini)
{
	// dl_fini is never defined in crt0.S so we won't use it here
	(void) dl_fini;

	const char msg[] = "Hello world via syscall!\n";
    // Using a local version of write so I can use it even without if I fail the relocation
	sys_write(1, msg, sizeof(msg)-1);

	if (csu_init)
        ((void (*)(void))csu_init)();

    char **envp = argv + argc + 1;

    int ret = main(argc, argv, envp);

	sys_write(1, "ok\n", 3);

	if (csu_fini)
        ((void (*)(void))csu_fini)();
	sys_write(1, "ok\n", 3);

    _exit(ret);
}

#include <asm/unistd.h>
#include <stddef.h>

#include "types.h"
#include "syscall.h"

#include "unistd.h"

void _exit(int rc)
{
	syscall1(__NR_exit, rc);
}

i64 write(int fd, const void *buf, size_t len)
{
	return syscall3(__NR_write, fd, (u64)buf, len);
}


i64 writev(int fd, const struct iovec *iov, int iovcnt)
{
	return syscall3(__NR_writev, fd, (u64)iov, iovcnt);
}

int open(const char *file, int flags, ...)
{
	register int mode asm("%rdx");
	return syscall3(__NR_open, (u64)file, flags, mode);
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, i64 offset)
{
	i64 rc = syscall6(__NR_mmap, (u64)addr, len, prot, flags, fd, offset);

	/* weird return value for mmap */
	if (rc < 0 && rc > -4096) {
		return MAP_FAILED;
	}

	return (void *)rc;
}

int munmap(void *addr, size_t len)
{
	return syscall2(__NR_munmap, (u64)addr, len);
}

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, ...)
{
	register u64 new_address asm("%r8");
	i64 rc = syscall5(__NR_mremap, (u64)old_address, old_size, new_size, flags, new_address);

	/* weird return value for mmap */
	if (rc < 0 && rc > -4096) {
		return MAP_FAILED;
	}

	return (void *)rc;
}

#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>

#include "types.h"

void _exit(int rc);
i64 write(int fd, const void *buf, size_t len);

struct iovec;

i64 writev(int fd, const struct iovec *iov, int iovcnt);

#ifndef O_RDONLY
    #define O_RDONLY 0
#endif

int open(const char *file, int flags, ...);
int close(int fd);

#ifndef MAP_FAILED
    #define MAP_FAILED ((void *)-1)
#endif

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */

#define MAP_SHARED	    0x01		/* Share changes.  */
#define MAP_PRIVATE     0x02		/* Changes are private.  */

#define MAP_ANONYMOUS	0x20        /* Don't use a file.  */
#define MAP_FIXED	    0x10		/* Interpret addr exactly.  */

void *mmap(void *addr, size_t len, int prot, int flags, int fd, i64 offset);
int munmap(void *addr, size_t len);
void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, ...);

ssize_t read(int fd, void *buf, size_t count);

#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

long lseek(int fd, long offset, int whence);

#endif /* !UNISTD_H */

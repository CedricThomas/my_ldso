#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>

#include "types.h"

void _exit(int rc);
i64 write(int fd, const void *buf, size_t len);

struct iovec;

i64 writev(int fd, const struct iovec *iov, int iovcnt);

int open(const char *file, int flags, ...);


#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

void *mmap(void *addr, size_t len, int prot, int flags, int fd, i64 offset);
int munmap(void *addr, size_t len);
void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, ...);

#endif /* !UNISTD_H */

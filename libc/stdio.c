#include "compiler.h"
#include "string.h"
#include "unistd.h"

#include <linux/uio.h>

void puts(const char *s)
{
#if 0
	size_t len = strlen(s);

	write(1, s, len);
	write(1, "\n", 1);
#else
	struct iovec iovec[] = {
		{ (void *)s, strlen(s) },
		{ "\n", 1},
	};

	writev(1, iovec, array_size(iovec));
#endif
}

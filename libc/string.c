#include <stddef.h>

size_t strlen(const char *s)
{
	return __builtin_strlen(s);
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;
	for (; len < maxlen; len++)
		if (!s[len])
			return len;
	return len;
}

void *memset(void *s, int c, size_t n)
{
	return __builtin_memset(s, c, n);
}

void *memcpy(void *dest, void *src, size_t n)
{
	return __builtin_memcpy(dest, src, n);
}

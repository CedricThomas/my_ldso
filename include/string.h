#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, void *src, size_t n);

#endif /* !STRING_H */

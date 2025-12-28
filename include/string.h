#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
size_t strlen_delim(const char *s, char delim);

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);

#endif /* !STRING_H */

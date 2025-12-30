#include <stddef.h>
#include "stdlib.h"

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t maxlen)
{
    size_t len = 0;
    for (; len < maxlen; len++)
        if (!s[len])
            break;
    return len;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n--)
        *d++ = *s++;

    return dest;
}
int strncmp(const char *s1, const char *s2, size_t n)
{
    size_t i = 0;

    while (i < n && s1[i] && s1[i] == s2[i]) {
        i++;
    }

    if (i == n)
        return 0;

    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

size_t strlen_delim(const char *s, char delim)
{
    size_t i = 0;

    if (!s)
        return 0;

    while (s[i] && s[i] != delim)
        i++;

    return i;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i])
            return (int)p1[i] - (int)p2[i];
    }
    return 0;
}
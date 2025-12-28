#include "stdlib.h"
#include "string.h"


char *strdup(const char *s)
{
    if (!s)
        return NULL;

    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (!dup)
        return NULL;

    memcpy(dup, (void *)s, len);
    return dup;
}

char *strndup(const char *s, size_t n) {
    if (!s)
        return NULL;

    size_t len = 0;
    while (len < n && s[len])
        len++;

    char *dup = malloc(len + 1);
    if (!dup)
        return NULL;

    for (size_t i = 0; i < len; i++)
        dup[i] = s[i];

    dup[len] = '\0';
    return dup;
}

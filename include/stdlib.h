#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

char *strdup(const char *s);
char *strndup(const char *s, size_t n);

#endif /* !STDLIB_H */

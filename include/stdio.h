#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

void puts(const char *s);

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);

#endif /* !STDIO_H */

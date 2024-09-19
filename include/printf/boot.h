#ifndef PRINTF_BOOT_H
#define PRINTF_BOOT_H

/* Support file for linux printf implementation */

#include <stdarg.h>
#include <stddef.h>

#include "ctype.h"
#include "string.h"
#include "unistd.h"

static inline void puts(const char *s)
{
	write(1, s, strlen(s));
}

#endif /* !PRINTF_BOOT_H */

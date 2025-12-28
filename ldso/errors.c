#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

const char *program_name = NULL;

void exit_with_error(const char *msg) {
    if (program_name)
        dprintf(2, "%s: %s\n", program_name, msg);
    else
        dprintf(2, "Error: %s\n", msg);
    _exit(1);
}

void exit_with_errorf(const char *fmt, ...) {
    char printf_buf[1024];
	va_list args;

    va_start(args, fmt);
	vsprintf(printf_buf, fmt, args);
	va_end(args);

    exit_with_error(printf_buf);
}

void set_program_name(const char *value) {
    program_name = value;
}
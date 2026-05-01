# Task 5: Extend printf for 64-bit and larger buffers

## Problem
The current `printf` is adapted from the 1991 Linux kernel. It has two issues:
1. The comment says "does not include 64-bit support" — on x86-64, `%lx` / `%x` works because `long` is 64-bit, but `long long` (`%llx`) is not handled.
2. The fixed 1024-byte buffer silently truncates long output strings.

## Files
- `libc/printf.c` — `printf()`, `dprintf()`, `sprintf()`, `vsprintf()`
- `include/printf/boot.h` — header (if needed for va_list)

## Changes
1. In `vsprintf()`, when `qualifier == 'l'` and the format specifier is 'x'/'X'/'d'/'u'/'o'/'i':
   - Already handled via `va_arg(args, unsigned long)` which is 64-bit on x86-64.
   - Add support for `ll` qualifier: check for consecutive 'l' chars, use `va_arg(args, unsigned long long)`.
   
2. Increase the fixed buffer from 1024 to 4096 in `printf()` and `dprintf()`.

3. Add a safety check: if `vsprintf` returns a length >= buffer size, print a warning to stderr before truncating.

```c
int printf(const char *fmt, ...) {
    char printf_buf[4096];
    va_list args;
    int printed;

    va_start(args, fmt);
    printed = vsprintf(printf_buf, fmt, args);
    va_end(args);

    if (printed >= sizeof(printf_buf)) {
        dprintf(2, "[printf] output truncated (%d chars)\n", printed);
    }

    puts(printf_buf);
    return printed;
}
```

4. Apply the same buffer increase and truncation warning to `dprintf()`.

## Verification
- Build and run existing tests — must still pass.
- Test with `%llx` format: `printf("addr: %llx\n", (unsigned long long)0xdeadbeef12345678)`.

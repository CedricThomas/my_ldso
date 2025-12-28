#include "unistd.h"
#include "stdio.h"

int file_exists(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;

    close(fd);
    return 1;
}

int resolve(char *result, size_t result_size,
            char **paths, const char *name)
{
    if (!result || result_size == 0)
        return 1;

    for (int i = 0; paths[i]; i++) {
        char *out = result;
        size_t left = result_size - 1;

        const char *p = paths[i];
        const char *n = name;

        /* copy path */
        while (*p && left) {
            *out++ = *p++;
            left--;
        }

        if (!left)
            continue;

        /* add '/' if needed */
        if (out != result && out[-1] != '/') {
            *out++ = '/';
            left--;
        }

        if (!left)
            continue;

        /* copy name */
        while (*n && left) {
            *out++ = *n++;
            left--;
        }

        *out = '\0';

        if (file_exists(result))
            return 0;
    }

    result[0] = '\0';
    return 1;
}

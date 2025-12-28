#include "ldso.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"

int file_exists(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;

    close(fd);
    return 1;
}

size_t count_path_segments(const char *path) {
    if (!path || !*path)
        return 0;

    size_t count = 1; // at least one
    for (size_t i = 0; path[i]; i++) {
        if (path[i] == ':')
            count++;
    }
    return count;
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

static size_t split_path_into_tab(const char *path, char **tab)
{
    if (!path || !tab) return 0;

    size_t count = 0;
    const char *start = path;

    while (*start) {
        size_t len = strlen_delim(start, ':');

        tab[count++] = strndup(start, len);
        start += len;
        if (*start == ':') start++;
    }

    tab[count] = NULL; /* NULL terminate */
    return count;
}

char **build_search_paths(dso_t *obj)
{
    if (!obj) return NULL;

    size_t total = 0;

    /* Step 1: RPATH (if no RUNPATH) */
    if (obj->rpath && !obj->runpath)
        total += count_path_segments(obj->rpath);

    /* Step 2: LD_LIBRARY_PATH */
    if (obj->ld_library_path)
        total += count_path_segments(obj->ld_library_path);

    /* Step 3: RUNPATH */
    if (obj->runpath)
        total += count_path_segments(obj->runpath);

    /* Step 4: cache (ignored here) */
    /* Step 5: default paths */
    total += 2;

    /* Allocate array */
    char **tab = malloc((total + 1) * sizeof(char *)); // +1 for final NULL
    if (!tab) return NULL;

    size_t offset = 0;

    /* Fill tab */
    if (obj->rpath && !obj->runpath)
        offset += split_path_into_tab(obj->rpath, tab + offset);

    if (obj->ld_library_path)
        offset += split_path_into_tab(obj->ld_library_path, tab + offset);

    if (obj->runpath)
        offset += split_path_into_tab(obj->runpath, tab + offset);

    tab[offset++] = strdup("/lib");
    tab[offset++] = strdup("/usr/lib");

    tab[offset] = NULL;

    return tab;
}

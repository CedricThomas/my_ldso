#include "ldso.h"
#include "string.h"
#include "types.h"
#include "stdio.h"

#include <link.h>
#include <stdint.h>
#include "stdio.h"

void print_auxv(ElfW(auxv_t) *auxv)
{
}

int check_ld_show_auxv(char **env)
{
    const char *key = "LD_SHOW_AUXV=";
    size_t key_len = strlen(key);

    for (; *env; env++) {
        if (strncmp(*env, key, key_len) == 0)
            return 1;
    }
    return 0;
}

#include "ldso.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

char *get_env_var(char **env, char *var) {
    char **copy = env;
    size_t var_len = strlen(var);

    for (; *copy; copy++) {
        if (strncmp(*copy, var, var_len) == 0 && (*copy)[var_len] == '=') {
            return *copy + var_len + 1;
        }
            
    }
    return NULL;
}

void free_tab(char **tab)
{
    if (!tab)
        return;

    for (size_t i = 0; tab[i]; i++)
        free(tab[i]);

    free(tab);
}
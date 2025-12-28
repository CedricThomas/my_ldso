#include "ldso.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

char *get_env_var_from_dso(dso_t *obj, char *var) {
    char **env = obj->env;
    size_t var_len = strlen(var);

    for (; *env; env++) {
        if (strncmp(*env, var, var_len) == 0 && (*env)[var_len] == '=') {
            return *env + var_len + 1;
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
#include "ldso.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h" 
#include "types.h"

int check_ld_trace_loaded_objects(char **env)
{
    const char *key = "LD_TRACE_LOADED_OBJECTS=";
    size_t key_len = strlen(key);

    for (; *env; env++) {
        if (strncmp(*env, key, key_len) == 0)
            return 1;
    }
    return 0;
}

void print_loaded_objects(dso_t *obj) {
    // stack-allocated strings
    char s0[] = "./";
    char s1[] = "/lib64/";
    char s2[] = "/lib/";
    char s3[] = "/usr/lib64/";
    char s4[] = "/usr/lib/";

    // intializing the default paths on the stack to avoid crashes 
    char *search_paths[] = { s0, s1, s2, s3, s4, NULL };
    char path[456];

    for (size_t i = 0; i < obj->needed_size; i++) {
        int found =  resolve(path, 456, search_paths, obj->needed[i]);
        printf("\t%s", obj->needed[i]);
        if (!found) {
            printf(" => %s", path);
        }
        // FIXME: we should use the loaded lib address
        printf(" (%p)\n", obj);
    }
}

int load_dso(dso_t *obj, ElfW(Addr) base, ElfW(Dyn) *dyn, char **envp)
{
    dyn_info_t info = scan_dynamic(dyn);

    memset(obj, 0, sizeof(obj));
    obj->base = base;
    obj->dynamic = dyn;
    obj->env = envp;
    obj->strtab = (const char *)(info.strtab);
    if (info.needed_size) {
        obj->needed = malloc(info.needed_size * sizeof(char *));
        if (!obj->needed) {
            return 1;
        }
        obj->needed_size = info.needed_size;
    }

    size_t needed_index = 0;
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
        case DT_NEEDED:
            obj->needed[needed_index] = obj->strtab + dyn->d_un.d_val;
            needed_index++;
            break;
        }
    }

    return 0;
}

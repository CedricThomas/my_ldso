#include <limits.h>
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

int load_dso(dso_t *obj, ElfW(Addr) base, ElfW(Dyn) *dyn)
{
    dyn_info_t info = scan_dynamic(dyn);

    memset(obj, 0, sizeof(obj));
    obj->base = base;
    obj->dynamic = dyn;
    obj->dynstr = (const char *)(info.dynstr);
    if (info.rpath_offset) {
        obj->rpath = obj->dynstr + info.rpath_offset; 
    } else if (info.runpath_offset) {
        obj->runpath = obj->dynstr + info.runpath_offset; 
    }
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
            obj->needed[needed_index] = obj->dynstr + dyn->d_un.d_val;
            needed_index++;
            break;
        }
    }

    return 0;
}

void print_loaded_objects(dso_t *obj, char **env) {
    char **search_paths = build_search_paths(obj, env);
    if (!search_paths)
        exit_with_error("cannot build libraries search path");

    for (size_t i = 0; i < obj->needed_size; i++) {
        char path[PATH_MAX];
        int found = resolve(path, PATH_MAX, search_paths, obj->needed[i]);

        printf("\t%s", obj->needed[i]);

        if (!found) {
            printf(" => %s", path);
        } else {
            printf(" => not found");
        }

         /* FIXME: lookup actual dso_t of resolved lib */
        ElfW(Addr) addr = found ? obj->base : 0;
        if (addr)
            printf(" (%p)", (void *)addr);

        printf("\n");
    }

    free_tab(search_paths);
}
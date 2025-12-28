#include "ldso.h"
#include "string.h"
#include "types.h"
#include "stdio.h"

void print_auxv(ElfW(auxv_t) *auxv)
{
    for (; auxv->a_type != AT_NULL; auxv++) {
        switch (auxv->a_type) {
        case AT_SYSINFO_EHDR: printf("AT_SYSINFO_EHDR:      0x%lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_MINSIGSTKSZ:  printf("AT_MINSIGSTKSZ:       %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_HWCAP:        printf("AT_HWCAP:             %lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_PAGESZ:       printf("AT_PAGESZ:            %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_CLKTCK:       printf("AT_CLKTCK:            %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_PHDR:         printf("AT_PHDR:              0x%lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_PHENT:        printf("AT_PHENT:             %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_PHNUM:        printf("AT_PHNUM:             %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_BASE:         printf("AT_BASE:              0x%lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_FLAGS:        printf("AT_FLAGS:             0x%lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_ENTRY:        printf("AT_ENTRY:             0x%lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_UID:          printf("AT_UID:               %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_EUID:         printf("AT_EUID:              %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_GID:          printf("AT_GID:               %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_EGID:         printf("AT_EGID:              %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_SECURE:       printf("AT_SECURE:            %lu\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_RANDOM:       printf("AT_RANDOM:            0x%lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_HWCAP2:       printf("AT_HWCAP2:            %lx\n", (unsigned long)auxv->a_un.a_val); break;
        case AT_EXECFN:       printf("AT_EXECFN:            %s\n", (char *)auxv->a_un.a_val); break;
        case AT_PLATFORM:     printf("AT_PLATFORM:          %s\n", (char *)auxv->a_un.a_val); break;
        default:             printf("AT_??? (0x%lx): 0x%lx\n", (unsigned long)auxv->a_type, (unsigned long)auxv->a_un.a_val); break;
        }
    }
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

void load_auxv_info(auxv_info_t *auxv_info, ElfW(auxv_t) *auxv)
{
    for (; auxv->a_type != AT_NULL; auxv++) {
        switch (auxv->a_type) {
        case AT_PHDR:   auxv_info->phdr  = auxv->a_un.a_val; break;
        case AT_PHENT:  auxv_info->phent = auxv->a_un.a_val; break;
        case AT_PHNUM:  auxv_info->phnum = auxv->a_un.a_val; break;
        case AT_BASE:   auxv_info->base  = auxv->a_un.a_val; break;
        case AT_ENTRY:  auxv_info->entry = auxv->a_un.a_val; break;
        }
    }
}

ElfW(auxv_t) *find_auxv(char **envp)
{
    while (*envp != NULL)
        envp++;
    envp++;

    return (ElfW(auxv_t) *)envp;
}

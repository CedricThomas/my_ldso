#include "ldso.h"
#include "string.h" 
#include "types.h"


ElfW(Dyn) *find_dynamic(auxv_info_t *auxv_info)
{
    ElfW(Phdr) *ph = (ElfW(Phdr) *)auxv_info->phdr;

    for (int i = 0; i < auxv_info->phnum; i++) {
        if (ph[i].p_type == PT_DYNAMIC)
            return (ElfW(Dyn) *)(ph[i].p_vaddr);
    }
    return NULL;
}

dyn_info_t scan_dynamic(Elf64_Dyn *dyn)
{
    dyn_info_t info = {0};

    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
        case DT_NEEDED:
            info.needed_size++;
            break;
        case DT_STRTAB:
            info.strtab = dyn->d_un.d_ptr;
            break;
        }
    }
    return info;
}

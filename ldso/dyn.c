#include "ldso.h"
#include "string.h" 
#include "types.h"


ElfW(Dyn) *find_dynamic_in_auxv(auxv_info_t *auxv_info)
{
    ElfW(Phdr) *ph = (ElfW(Phdr) *)auxv_info->phdr;

    for (int i = 0; i < auxv_info->phnum; i++) {
        if (ph[i].p_type == PT_DYNAMIC)
            return (ElfW(Dyn) *)(ph[i].p_vaddr);
    }
    return NULL;
}

void parse_dynamic_reloc_info(dso_t *dso, relocation_dyn_info_t *info) {
    memset(info, 0, sizeof(*info));
    if (!dso->dynamic) return;

    ElfW(Addr) dt_symtab = 0;
    ElfW(Addr) dt_strtab = 0;

    for (ElfW(Dyn) *dyn = dso->dynamic; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {

            case DT_RELA:
                info->rela = (ElfW(Rela)*)dso_resolve_ptr(dso, dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                info->rela_sz = dyn->d_un.d_val;
                break;

            case DT_REL:
                info->rel = (ElfW(Rel)*)dso_resolve_ptr(dso, dyn->d_un.d_ptr);
                break;
            case DT_RELSZ:
                info->rel_sz = dyn->d_un.d_val;
                break;

            case DT_JMPREL:
                info->jmprel = (ElfW(Rela)*)dso_resolve_ptr(dso, dyn->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                info->jmprel_sz = dyn->d_un.d_val;
                break;
            case DT_PLTREL:
                info->plt_is_rela = (dyn->d_un.d_val == DT_RELA);
                break;

            case DT_SYMTAB:
                dt_symtab = dyn->d_un.d_ptr;
                break;
            case DT_SYMENT:
                info->syment = dyn->d_un.d_val;
                break;

            case DT_STRTAB:
                dt_strtab = dyn->d_un.d_ptr;
                break;

            default:
                break;
        }
    }

    if (dt_symtab && dt_strtab && info->syment) {
        info->dynsym = (ElfW(Sym)*)dso_resolve_ptr(dso, dt_symtab);
        info->dynstr = (const char*)dso_resolve_ptr(dso, dt_strtab);
        info->dynsym_sz = (const char*)info->dynstr - (const char*)info->dynsym; // quick hack
    }
}

// This method doesn't take the type of binary DSO it is loading the infos from so the dynstr is raw  
dso_dyn_info_t parse_dynamic_dso_info(Elf64_Dyn *dyn)
{
    dso_dyn_info_t info = {0};

    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
        case DT_NEEDED:
            info.needed_size++;
            break;
        case DT_STRTAB:
            info.dynstr = dyn->d_un.d_ptr;
            break;
        case DT_RUNPATH:
            info.runpath_offset = dyn->d_un.d_val;
            break;
        case DT_RPATH:
            info.rpath_offset = dyn->d_un.d_val;
            break;
        }
    }
    return info;
}

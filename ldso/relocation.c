#include "ldso.h"
#include "string.h"
#include "stdio.h"

static ElfW(Addr) resolve_symbol(const char *name, linked_list_t *map) {
    const size_t MAX_SYMS = 4096; // safety cap

    for (linked_node_t *n = map->head; n; n = n->next) {
        dso_t *dso = &n->data;

        relocation_dyn_info_t info;
        parse_dynamic_reloc_info(dso, &info);

        if (!info.dynsym || !info.dynstr || !info.syment)
            continue;

        size_t sym_count = info.dynsym_sz / info.syment;
        if (sym_count > MAX_SYMS)
            sym_count = MAX_SYMS;

        for (size_t i = 0; i < sym_count; i++) {
            ElfW(Sym) *s = &info.dynsym[i];

            // skip local symbols
            if (ELF64_ST_BIND(s->st_info) == STB_LOCAL)
                continue;

            // skip undefined symbols
            if (s->st_shndx == SHN_UNDEF)
                continue;

            const char *sym_name = info.dynstr + s->st_name;
            if (strcmp(sym_name, name) == 0)
                return (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
        }
    }

    return 0;
}

static void apply_rela_table(
    dso_t *dso,
    relocation_dyn_info_t *info,
    ElfW(Rela) *rela,
    size_t size,
    linked_list_t *link_map
) {
    size_t count = size / sizeof(ElfW(Rela));
    
    for (size_t i = 0; i < count; i++) {
        ElfW(Rela) *r = &rela[i];
        ElfW(Addr) *where = (ElfW(Addr)*)dso_resolve_ptr(dso, r->r_offset);

        uint32_t type = ELF64_R_TYPE(r->r_info);
        uint32_t sym  = ELF64_R_SYM(r->r_info);

        switch (type) {

        case R_X86_64_RELATIVE:
            *where = dso->base + r->r_addend;
            break;

        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT: {
            ElfW(Sym) *s = &info->dynsym[sym];
            const char *name = dso->dynstr + s->st_name;

            ElfW(Addr) addr = resolve_symbol(name, link_map);
            if (!addr) {
                exit_with_errorf("cannot resolve symbol %s", name);
            }

            *where = addr + r->r_addend;
            break;
        }

        default:
            dprintf(2, "unsupported relocation");
            /* relocation non supportée */
            break;
        }
    }
}

static void apply_rel_table(
    dso_t *dso,
    relocation_dyn_info_t *info,
    ElfW(Rel) *rel,
    size_t size,
    linked_list_t *link_map
) {
    size_t count = size / sizeof(ElfW(Rel));

    for (size_t i = 0; i < count; i++) {
        ElfW(Rel) *r = &rel[i];

        ElfW(Addr) *where =
            (ElfW(Addr)*)dso_resolve_ptr(dso, r->r_offset);

        uint32_t type = ELF64_R_TYPE(r->r_info);
        uint32_t sym  = ELF64_R_SYM(r->r_info);

        switch (type) {

        case R_X86_64_RELATIVE:
            *where += dso->base;
            break;

        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT: {
            ElfW(Sym) *s = &info->dynsym[sym];
            const char *name = dso->dynstr + s->st_name;

            ElfW(Addr) addr = resolve_symbol(name, link_map);
            *where = addr;
            break;
        }

        default:
            break;
        }
    }
}

void relocate_dso(dso_t *dso, linked_list_t *link_map)
{
    relocation_dyn_info_t info;

    parse_dynamic_reloc_info(dso, &info);

    /* Static relocations */
    if (info.rela && info.rela_sz) {
        apply_rela_table(dso, &info, info.rela, info.rela_sz, link_map);
    }

    if (info.rel && info.rel_sz) {
        apply_rel_table(dso, &info, info.rel, info.rel_sz, link_map);
    }

    /* PLT / GOT relocations */
    if (info.jmprel && info.jmprel_sz) {
        if (info.plt_is_rela)
            apply_rela_table(dso, &info, info.jmprel, info.jmprel_sz, link_map);
        else
            apply_rel_table(dso, &info,
                (ElfW(Rel)*)info.jmprel, info.jmprel_sz, link_map);
    }
}

#include "ldso.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"

#ifndef DT_GNU_HASH
#define DT_GNU_HASH 0x6ffffef5
#endif

static uint32_t elf_hash_str(const char *name)
{
    uint32_t h = 0, g;
    for (; *name; name++) {
        h = (h << 4) + *name;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

/* Look up name in a GNU hash table.  Returns symbol index or UINT32_MAX.
 * The GNU hash table bloom filter size formula varies between toolchain
 * versions, making portability difficult. We skip it and rely on the
 * ELF hash or linear scan fallback for symbol resolution.
 */
static uint32_t gnu_hash_lookup(const uint32_t *hash, ElfW(Sym) *dynsym,
                                const char *dynstr, const char *name)
{
    (void)hash; (void)dynsym; (void)dynstr; (void)name;
    return UINT32_MAX;
}

/* Look up name in an ELF (SYSV) hash table.  Returns symbol index or UINT32_MAX. */
static uint32_t elf_hash_lookup(const uint32_t *hash, ElfW(Sym) *dynsym,
                                const char *dynstr, const char *name)
{
    uint32_t nbuckets = hash[0];
    const uint32_t *buckets = hash + 1;
    const uint32_t *chain   = hash + 1 + nbuckets;

    uint32_t h = elf_hash_str(name);

    /* Walk the chain. Each chain entry: low bit = terminator,
     * bits 1-24 = 24-bit hash, bits 25-31 = unused (or next pointer).
     * Actually: the chain IS the next-index array. chain[idx] encodes:
     *   low bit = 1 means end of chain (rest is hash value)
     *   low bit = 0 means chain[idx] >> 1 is the next index */
    for (uint32_t idx = buckets[h % nbuckets]; idx != 0; idx = chain[idx] >> 1) {
        if ((chain[idx] & 0xffffff) == (h & 0xffffff) &&
            strcmp(dynstr + (dynsym + idx)->st_name, name) == 0)
            return idx;
    }
    return UINT32_MAX;
}

static ElfW(Addr) resolve_symbol(const char *name, linked_list_t *map) {
    const size_t MAX_SYMS = 4096; /* safety cap */

    for (linked_node_t *n = map->head; n; n = n->next) {
        dso_t *dso = &n->data;

        relocation_dyn_info_t info;
        parse_dynamic_reloc_info(dso, &info);

        if (!info.dynsym || !info.dynstr || !info.syment)
            continue;

        uint32_t sym_idx = UINT32_MAX;

        /* Try GNU hash table first (preferred) */
        if (info.gnu_hash) {
            sym_idx = gnu_hash_lookup(info.gnu_hash, info.dynsym,
                                      info.dynstr, name);
        }

        /* Fallback to ELF (SYSV) hash table */
        if (sym_idx == UINT32_MAX && info.elf_hash) {
            sym_idx = elf_hash_lookup(info.elf_hash, info.dynsym,
                                      info.dynstr, name);
        }

        /* Hash table match found — return address */
        if (sym_idx != UINT32_MAX) {
            ElfW(Sym) *s = &info.dynsym[sym_idx];
            if (s->st_shndx != SHN_UNDEF)
                return (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            /* Symbol is undefined in this DSO, continue to next DSO */
            continue;
        }

        /* Hash table not available or didn't find symbol — linear scan */
        size_t sym_count = info.dynsym_sz / info.syment;
        if (sym_count > MAX_SYMS)
            sym_count = MAX_SYMS;

        for (size_t i = 0; i < sym_count; i++) {
            ElfW(Sym) *s = &info.dynsym[i];

            /* skip local symbols */
            if (ELF64_ST_BIND(s->st_info) == STB_LOCAL)
                continue;

            /* skip undefined symbols */
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

        case R_X86_64_PC32: {
            ElfW(Sym) *s = &info->dynsym[sym];
            ElfW(Addr) loc = (s->st_shndx == SHN_UNDEF) ? 0 : (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            *where = loc + r->r_addend - (ElfW(Addr))where;
            break;
        }

        case R_X86_64_64: {
            ElfW(Sym) *s = &info->dynsym[sym];
            ElfW(Addr) loc = (s->st_shndx == SHN_UNDEF) ? 0 : (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            *where = loc + r->r_addend;
            break;
        }

        case R_X86_64_32: {
            ElfW(Sym) *s = &info->dynsym[sym];
            ElfW(Addr) loc = (s->st_shndx == SHN_UNDEF) ? 0 : (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            *(uint32_t *)where = (uint32_t)(loc + r->r_addend);
            break;
        }

        case R_X86_64_32S: {
            ElfW(Sym) *s = &info->dynsym[sym];
            ElfW(Addr) loc = (s->st_shndx == SHN_UNDEF) ? 0 : (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            *(int32_t *)where = (int32_t)(loc + r->r_addend);
            break;
        }

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
            exit_with_errorf("unsupported relocation type %u at %p", type, where);
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

        case R_X86_64_PC32: {
            ElfW(Sym) *s = &info->dynsym[sym];
            ElfW(Addr) loc = (s->st_shndx == SHN_UNDEF) ? 0 : (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            *where = loc + *(int32_t *)where - (ElfW(Addr))where;
            break;
        }

        case R_X86_64_32: {
            ElfW(Sym) *s = &info->dynsym[sym];
            ElfW(Addr) loc = (s->st_shndx == SHN_UNDEF) ? 0 : (ElfW(Addr))dso_resolve_ptr(dso, s->st_value);
            *(uint32_t *)where = (uint32_t)(loc + *(int32_t *)where);
            break;
        }

        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT: {
            ElfW(Sym) *s = &info->dynsym[sym];
            const char *name = dso->dynstr + s->st_name;

            ElfW(Addr) addr = resolve_symbol(name, link_map);
            *where = addr;
            break;
        }

        default:
            exit_with_errorf("unsupported relocation type %u at %p", type, where);
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

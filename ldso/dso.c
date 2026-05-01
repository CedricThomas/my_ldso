#include <limits.h>
#include "ldso.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h" 
#include "unistd.h" 
#include "types.h"

int check_ld_trace_loaded_objects(char **env) {
    const char *key = "LD_TRACE_LOADED_OBJECTS=";
    size_t key_len = strlen(key);

    for (; *env; env++) {
        if (strncmp(*env, key, key_len) == 0)
            return 1;
    }
    return 0;
}

void print_link_map(linked_list_t *map) {
    if (!map) return;

    linked_list_for_each(map, iter) {
        dso_t *obj = &iter->data;
        
        if (obj->origin == DSO_KERNEL_MAPPED) {
            continue;
        }

        if (obj->path) {
            printf("%s => %s (0x%lx)\n",
                   iter->data.name ? iter->data.name : "(unknown)",
                   obj->path,
                   obj->base);
        } else {
            printf("%s => not found\n",
                   iter->data.name ? iter->data.name : "(unknown)");
        }
    }
}

static void load_dso_from_dynamic(dso_t *obj, ElfW(Addr) base, ElfW(Dyn) *dyn) {
    dso_dyn_info_t info = parse_dynamic_dso_info(dyn);

    obj->base = base;
    obj->ehdr = (ElfW(Ehdr) *)base;
    obj->dynamic = dyn;

    /* Compute bias */
    ElfW(Phdr) *phdrs = (ElfW(Phdr) *)((char *)base + obj->ehdr->e_phoff);
    ElfW(Addr) min_vaddr = (ElfW(Addr))-1;
    for (ElfW(Half) i = 0; i < obj->ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD && phdrs[i].p_vaddr < min_vaddr)
            min_vaddr = phdrs[i].p_vaddr;
    }
    obj->bias = base - min_vaddr;

    /* Dynamic strings */
    obj->dynstr = dso_resolve_ptr(obj, info.dynstr);

    if (info.rpath_offset)
        obj->rpath = obj->dynstr + info.rpath_offset;
    else if (info.runpath_offset)
        obj->runpath = obj->dynstr + info.runpath_offset;
    
    /* DT_NEEDED */
    if (info.needed_size) {
        obj->needed = malloc(info.needed_size * sizeof(char *));
        if (!obj->needed)
            exit_with_error("out of memory");

        obj->needed_size = info.needed_size;

        size_t idx = 0;
        for (ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; d++) {
            if (d->d_tag == DT_NEEDED) {
                obj->needed[idx++] = obj->dynstr + d->d_un.d_val;
            }
        }
    }
}

void load_dso_from_auxv(dso_t *obj, auxv_info_t *auxv, char *path, char *name) {
    ElfW(Dyn) *dyn = find_dynamic_in_auxv(auxv);
    if (!dyn)
        exit_with_error("invalid dynamic section");

    /* Compute exec_base: the ELF header is at file offset 0, the phdr
     * table at file offset e_phoff (= sizeof(ElfW(Ehdr)) in practice).
     * Since AT_PHDR is the mapped address of the phdr table, the ELF
     * header (and thus the EXEC base) is at: AT_PHDR - sizeof(Ehdr). */
    ElfW(Addr) exec_base = auxv->phdr - sizeof(ElfW(Ehdr));
    ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)exec_base;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0)
        exec_base = auxv->phdr - ehdr->e_phoff;

    /* For ET_EXEC, phdr p_vaddr IS the absolute mapped address
     * (kernel maps at preferred addresses). So dyn is already absolute. */
    ElfW(Addr) dyn_addr = (ElfW(Addr))dyn;

    memset(obj, 0, sizeof(dso_t));
    obj->origin = DSO_KERNEL_MAPPED;
    obj->path = path;
    obj->name = name;
    load_dso_from_dynamic(obj, exec_base, (ElfW(Dyn) *)dyn_addr);
}

void load_dso_from_path(dso_t *obj, const char *path, const char *name) {
    obj->path   = path;
    obj->name   = name;
    obj->origin = DSO_LOADER_MAPPED;

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        exit_with_error("failed to open DSO");

    /* Read ELF header */
    ElfW(Ehdr) ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
        exit_with_error("failed to read ELF header");

    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0)
        exit_with_error("not an ELF");

    if (ehdr.e_phnum == 0)
        exit_with_error("ELF has no program headers");

    /* Read program headers */
    size_t phdrs_size = ehdr.e_phnum * sizeof(ElfW(Phdr));
    ElfW(Phdr) *phdrs = malloc(phdrs_size);
    if (!phdrs)
        exit_with_error("out of memory");

    if (lseek(fd, ehdr.e_phoff, SEEK_SET) < 0 ||
        read(fd, phdrs, phdrs_size) != (ssize_t)phdrs_size)
        exit_with_error("failed to read program headers");

    /* Compute address span */
    ElfW(Addr) min_vaddr = (ElfW(Addr))-1;
    ElfW(Addr) max_vaddr = 0;

    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD)
            continue;

        if (phdrs[i].p_memsz == 0)
            continue;

        if (phdrs[i].p_vaddr < min_vaddr)
            min_vaddr = phdrs[i].p_vaddr;

        if (phdrs[i].p_vaddr + phdrs[i].p_memsz > max_vaddr)
            max_vaddr = phdrs[i].p_vaddr + phdrs[i].p_memsz;
    }

    if (min_vaddr >= max_vaddr)
        exit_with_error("invalid PT_LOAD layout");

    min_vaddr = PAGE_DOWN(min_vaddr);
    max_vaddr = PAGE_UP(max_vaddr);

    size_t map_size = max_vaddr - min_vaddr;

    /* For EXEC binaries (min_vaddr != 0), map at preferred addresses.
     * For shared libraries (min_vaddr == 0), reserve address space first.
     * In both cases, base = ELF header address so obj->ehdr = base works. */
    ElfW(Addr) base;
    if (min_vaddr != 0) {
        /* EXEC: use preferred addresses directly */
        base = min_vaddr;
    } else {
        /* Shared lib: let kernel choose address */
        void *mapping = mmap(NULL, map_size, PROT_NONE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mapping == MAP_FAILED)
            exit_with_error("failed to reserve address space");
        base = (ElfW(Addr))mapping;
    }

    /* Map PT_LOAD segments */
    for (int i = 0; i < ehdr.e_phnum; i++) {
        ElfW(Phdr) *p = &phdrs[i];

        if (p->p_type != PT_LOAD || p->p_memsz == 0)
            continue;

        ElfW(Addr) seg_vaddr = p->p_vaddr;
        ElfW(Addr) seg_start = PAGE_DOWN(seg_vaddr);
        ElfW(Addr) seg_end   = PAGE_UP(seg_vaddr + p->p_memsz);

        ElfW(Off) file_offset = PAGE_DOWN(p->p_offset);

        int prot = 0;
        if (p->p_flags & PF_R) prot |= PROT_READ;
        if (p->p_flags & PF_W) prot |= PROT_WRITE;
        if (p->p_flags & PF_X) prot |= PROT_EXEC;

        /* EXEC maps at preferred addresses, shared at base+vaddr */
        ElfW(Addr) seg_map = (min_vaddr != 0) ? seg_start : (base + seg_start);

        /* Map segment in three steps:
         * 1) Map the entire segment as anonymous (within PROT_NONE reservation)
         * 2) If file data exists, overlay file-backed mapping
         * 3) If p_filesz < p_memsz, zero the .bss gap
         *
         * For non-page-aligned p_filesz, the file mmap reads extra bytes
         * beyond p_filesz into .bss. We fix this by zeroing .bss after mapping.
         */
        ElfW(Addr) seg_start_mapped = (min_vaddr != 0) ? seg_start : (base + seg_start);
        ElfW(Addr) seg_end_mapped   = (min_vaddr != 0) ? seg_end   : (base + seg_end);

        /* Step 1: anonymous mapping for the whole segment */
        if (mmap((void *)seg_start_mapped,
                 seg_end_mapped - seg_start_mapped,
                 prot,
                 MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
                 -1,
                 0) == MAP_FAILED)
            exit_with_error("PT_LOAD anon mmap failed");

        /* Step 2: overlay file-backed portion */
        if (p->p_filesz > 0) {
            ElfW(Addr) fsize = PAGE_UP(p->p_filesz);
            if (fsize > seg_end_mapped - seg_start_mapped)
                fsize = seg_end_mapped - seg_start_mapped;
            if (mmap((void *)seg_start_mapped,
                     fsize,
                     prot,
                     MAP_PRIVATE | MAP_FIXED,
                     fd,
                     file_offset) == MAP_FAILED)
                exit_with_error("PT_LOAD file mmap failed");

            /* Step 3: zero .bss gap (file data may have overwritten it) */
            if (p->p_memsz > p->p_filesz) {
                ElfW(Addr) bss_start = seg_start_mapped + p->p_filesz;
                size_t bss_len = p->p_memsz - p->p_filesz;
                memset((void *)bss_start, 0, bss_len);
            }
        }
    }

    /* Locate PT_DYNAMIC */
    ElfW(Dyn) *dynamic = NULL;
    ElfW(Addr) bias = base - min_vaddr;

    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_DYNAMIC) {
            dynamic = (ElfW(Dyn) *)(bias + phdrs[i].p_vaddr);
            break;
        }
    }

    if (!dynamic)
        exit_with_error("DSO has no PT_DYNAMIC");

    close(fd);
    free(phdrs);

    /* Continue from dynamic section */
    load_dso_from_dynamic(obj, base, dynamic);
}

static int dso_match(data_t *a, data_t *b)
{
    if (a->path && b->path &&
        strcmp(a->path, b->path) == 0)
        return 1;

    if (a->name && b->name &&
        strcmp(a->name, b->name) == 0)
        return 1;

    return 0;
}

void free_dso(dso_t *obj)
{
    if (!obj)
        return;

    if (obj->needed) {
        free(obj->needed);
        obj->needed = NULL;
    }

    /* Only free path/name for loader-mapped DSOs (strdup'd in
     * resolve_dependencies_recursive). Kernel-mapped main binary
     * points to argv[0] — must not be freed. */
    if (obj->origin == DSO_LOADER_MAPPED) {
        if (obj->path) {
            free((void *)obj->path);
            obj->path = NULL;
        }
        if (obj->name) {
            free((void *)obj->name);
            obj->name = NULL;
        }
    }
}

void resolve_dependencies_recursive(
    linked_list_t *map,
    dso_t *obj,
    char **env
) {
    char **search_paths = build_search_paths(obj, env);
    if (!search_paths)
        exit_with_error("cannot build libraries search path");

    for (size_t i = 0; i < obj->needed_size; i++) {
        data_t lib_data = {0};
        char resolved_path[PATH_MAX];
        int found = resolve(resolved_path, PATH_MAX, search_paths, obj->needed[i]);
        if (found) {
            printf("cannot find library %s\n", obj->needed[i]);
            exit_with_error("library not found");
        }
        lib_data.path = strdup(resolved_path);
        lib_data.name = strdup(obj->needed[i]);

        if (linked_list_search(map, &lib_data, dso_match)) {
            free((void *)lib_data.path);
            free((void *)lib_data.name);
            continue; /* already in link map */
        }

        /* Load the DSO */
        load_dso_from_path(&lib_data, lib_data.path, lib_data.name);

        if (!linked_list_append(map, lib_data)) {
            exit_with_error("cannot insert dependency");
        }

        /* Recurse */
        resolve_dependencies_recursive(map, &lib_data, env);
    }

    free_tab(search_paths);
}

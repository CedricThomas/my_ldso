#ifndef LDSO_H
#define LDSO_H

#include <link.h>
#include <elf.h>


#define PAGE_SIZE 4096
#define PAGE_DOWN(x) ((x) & ~(PAGE_SIZE - 1))
#define PAGE_UP(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))


struct auxv_info {
    ElfW(Addr) phdr;
    ElfW(Half) phent;
    ElfW(Half) phnum;
    ElfW(Addr) base;
    ElfW(Addr) entry;
};

typedef struct auxv_info auxv_info_t;

struct dso_dyn_info {
    size_t needed_size;
    // Raw dynstr pointer without bias translation
    ElfW(Addr) dynstr;
    size_t rpath_offset;
    size_t runpath_offset;

};

typedef struct dso_dyn_info dso_dyn_info_t;

typedef enum {
    DSO_KERNEL_MAPPED,   // binaire principal
    DSO_LOADER_MAPPED    // libs chargées par toi
} dso_origin_t;

struct dso {
    const char *name;           /* DT_NEEDED string or basename */
    const char *path;           /* resolved full path on disk */
    ElfW(Addr) base;            /* load address */
    ElfW(Addr) bias;            /* recalibrage pour les ET_DYN */
    ElfW(Ehdr) *ehdr;           /* ELF header mappé en mémoire */
    ElfW(Dyn) *dynamic;         /* PT_DYNAMIC */
    const char *dynstr;         /* DT_STRTAB */
    const char *rpath;          /* DT_RPATH */
    const char *runpath;        /* DT_RUNPATH */
    const char **needed;        /* DT_NEEDED strings */
    size_t needed_size;

    dso_origin_t origin;
};

typedef struct dso dso_t;


static inline void *dso_resolve_ptr(dso_t *obj, ElfW(Addr) elf_addr)
{
    if (!elf_addr)
        return (void *)0;

    return (void *)(obj->bias + elf_addr);
}

typedef dso_t data_t;

/* Node in the linked list */
typedef struct linked_node {
    data_t data;
    struct linked_node *next;
    struct linked_node *prev;
} linked_node_t;

/* List head */
typedef struct {
    linked_node_t *head;
    linked_node_t *tail;
    size_t size;
} linked_list_t;

/* dynamic segment info needed for a relocation */
typedef struct relocation_dyn_info {
    /* relocation tables */
    ElfW(Rela) *rela;
    size_t      rela_sz;

    ElfW(Rel)  *rel;
    size_t      rel_sz;

    ElfW(Rela) *jmprel;
    size_t      jmprel_sz;

    /* symbols + strings */
    ElfW(Sym)  *dynsym;
    size_t      syment;     /* size of a symbol entry */
    size_t      dynsym_sz;  /* size of dynsym in bytes */
    const char *dynstr;     /* string table */
    
    /* misc */
    int         plt_is_rela; /* DT_PLTREL == DT_RELA ? */
} relocation_dyn_info_t;

// linked_list.c
void linked_list_init(linked_list_t *list);
linked_node_t* linked_list_append(linked_list_t *list, data_t data);
linked_node_t* linked_list_prepend(linked_list_t *list, data_t data);
linked_node_t* linked_list_insert_after(linked_list_t *list, linked_node_t *node, data_t data);
linked_node_t* linked_list_insert_before(linked_list_t *list, linked_node_t *node, data_t data);
void linked_list_delete(linked_list_t *list, linked_node_t *node);
data_t linked_list_pop_front(linked_list_t *list);
data_t linked_list_pop_back(linked_list_t *list);
linked_node_t *linked_list_search(linked_list_t *list, data_t *needle, int (*test)(data_t *, data_t *));
void linked_list_clear(linked_list_t *list);
void linked_list_free(linked_list_t *list);
static inline size_t linked_list_size(linked_list_t *list) { return list->size; }

#define linked_list_for_each(list, iter) \
    for (linked_node_t *iter = (list)->head; iter != NULL; iter = iter->next)

#define linked_list_for_each_reverse(list, iter) \
    for (linked_node_t *iter = (list)->tail; iter != NULL; iter = iter->prev)


// auxv.c
void print_auxv(ElfW(auxv_t) *auxv);
int check_ld_show_auxv(char **env);
ElfW(auxv_t) *find_auxv(char **envp);
void load_auxv_info(auxv_info_t *auxv_info, ElfW(auxv_t) *auxv);

// dyn.c
ElfW(Dyn) *find_dynamic_in_auxv(auxv_info_t *auxv_info);
dso_dyn_info_t parse_dynamic_dso_info(ElfW(Dyn) *dyn);
void parse_dynamic_reloc_info(dso_t *dso, relocation_dyn_info_t *info);

// dso.c
int check_ld_trace_loaded_objects(char **env);
void print_link_map(linked_list_t *map);
void load_dso_from_auxv(dso_t *obj, auxv_info_t *auxv, char *path, char *name);
void load_dso_from_path(dso_t *obj, const char *path, const char *name);
void resolve_dependencies_recursive(linked_list_t *map, dso_t *obj, char **env);

// lib_path_search.c
char **build_search_paths(dso_t *obj, char **env);
int file_exists(const char *path);
int resolve(char *result, size_t result_sz, char **paths, const char *name);
size_t count_path_segments(const char *path);

// utils.c
char *get_env_var(char **env, char *var);
void free_tab(char **tab);


// errors.c
void exit_with_error(const char *msg);
void exit_with_errorf(const char *fmt, ...);
void set_program_name(const char *value);

// relocation.c
void relocate_dso(dso_t *dso, linked_list_t *link_map);

#endif /* !LDSO_H */

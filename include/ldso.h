#ifndef LDSO_H
#define LDSO_H

#include <link.h>
#include <elf.h>

struct auxv_info {
    ElfW(Addr) phdr;
    ElfW(Half) phent;
    ElfW(Half) phnum;
    ElfW(Addr) base;
    ElfW(Addr) entry;
};

typedef struct auxv_info auxv_info_t;

struct dyn_info {
    size_t needed_size;
    ElfW(Addr) strtab;
};

typedef struct dyn_info dyn_info_t;

struct dso {
    char **env;
    ElfW(Addr) base;          /* load address */
    ElfW(Dyn) *dynamic;       /* PT_DYNAMIC */

    const char *strtab;       /* DT_STRTAB */
    const char **needed;      /* DT_NEEDED strings */
    size_t needed_size;
};

typedef struct dso dso_t;

// auxv.c
void print_auxv(ElfW(auxv_t) *auxv);
int check_ld_show_auxv(char **env);
ElfW(auxv_t) *find_auxv(char **envp);
void load_auxv_info(auxv_info_t *auxv_info, ElfW(auxv_t) *auxv);

// dyn.c
ElfW(Dyn) *find_dynamic(auxv_info_t *auxv_info);
dyn_info_t scan_dynamic(ElfW(Dyn) *dyn);

// dso.c
int load_dso(dso_t *obj, ElfW(Addr) base, ElfW(Dyn) *dyn, char **env);
int check_ld_trace_loaded_objects(char **env);

void print_loaded_objects(dso_t *obj);

// utils.c
int file_exists(const char *path);
int resolve(char *result, size_t result_sz, char **paths, const char *name);

// errors.c
void exit_with_error(const char *msg);
void exit_with_errorf(const char *fmt, ...);
void set_program_name(const char *value);

#endif /* !LDSO_H */

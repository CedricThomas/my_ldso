#ifndef LDSO_H
#define LDSO_H

#include <link.h>
#include <elf.h>

void print_auxv(ElfW(auxv_t) *auxv);
int check_ld_show_auxv(char **env);

#endif /* !LDSO_H */

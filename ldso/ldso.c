#include <stdbool.h>
#include <elf.h>
#include <stddef.h>
#include <sys/auxv.h>
#include <asm/mman.h>
#include <asm-generic/fcntl.h>

#include "ldso.h"
#include "types.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

static inline void jmp_to_usercode(u64 entry, u64 stack)
{
    asm volatile ("mov %[stack], %%rsp\n"
                  "push %[entry]\n"
                  "ret" :: [entry]"rm"(entry), [stack]"rm"(stack));
}

void ldso_main(u64 *stack)
{
    int argc = *stack;
    char **argv = (void *)&stack[1];
    char **envp = argv + argc + 1;
    ElfW(auxv_t) *auxv = find_auxv(envp);

    set_program_name(argv[0]);
    // If LD_SHOW_AUXV is set, printing the auxiliary vector before execution
    if (check_ld_show_auxv(envp)){
        print_auxv(auxv);
    }

    auxv_info_t auxv_info;
    load_auxv_info(&auxv_info, auxv);

    ElfW(Dyn) *dyn = find_dynamic(&auxv_info);
    if (dyn == NULL) {
        exit_with_error("invalid dynamic section");
    }

    dso_t obj;
    if (load_dso(&obj, auxv_info.base, dyn)) {
        exit_with_error("invalid dso object");
    }

    // TODO: Load libraries
    
    // If LD_TRACE_LOADED_OBJECTS is set, print the loaded objects statuses
    if (check_ld_trace_loaded_objects(envp)) {
        print_loaded_objects(&obj, envp);
        _exit(0);
    }

    if (auxv_info.base != 0) {
        jmp_to_usercode(auxv_info.entry, (u64)stack);
    }

    if (argc <= 1) {
        exit_with_error("missing program name\n");
    }

    exit_with_error("direct loading not implemented yet\n");
}

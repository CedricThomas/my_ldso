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

void jmp_to_usercode(u64 entry, u64 stack)
{
    asm volatile (
        "mov %0, %%rsp\n"
        "xor %%ebp, %%ebp\n"   /* CRITICAL */
        "jmp *%1\n"
        :
        : "r"(stack), "r"(entry)
    );
    __builtin_unreachable();
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

    dso_t bin;
    auxv_info_t auxv_info = {0};
    load_auxv_info(&auxv_info, auxv);

    /* Direct mode: no interpreter (AT_BASE == 0) and extra args present */
    int direct_mode = (auxv_info.base == 0 && argc > 1);
    if (direct_mode) {
        argv[0] = argv[1];
        load_dso_from_path(&bin, argv[0], argv[0]);
    } else {
        load_dso_from_auxv(&bin, &auxv_info, argv[0], argv[0]);
    }

    linked_list_t dependencies;
    linked_list_init(&dependencies);
    linked_list_append(&dependencies, bin);
    resolve_dependencies_recursive(
        &dependencies,
        &bin,
        envp
    );

    // If LD_TRACE_LOADED_OBJECTS is set, print the loaded objects statuses
    if (check_ld_trace_loaded_objects(envp)) {
        print_link_map(&dependencies);
        _exit(0);
    }

    linked_list_for_each(&dependencies, n) {
        relocate_dso(&n->data, &dependencies);
    }

    if (direct_mode) {
        /* After argv[0] = argv[1], stack[2] is already the target name.
         * Just fix argc and shift the NULL terminator left by 1. */
        u64 new_argc = argc - 1;
        /* Move NULL from position argc+2 to argc+1 */
        stack[1 + new_argc + 1] = stack[1 + argc + 1];  /* shift NULL left */
        stack[1] = new_argc;
        jmp_to_usercode((u64)dso_resolve_ptr(&bin, bin.ehdr->e_entry), (u64)&stack[1]);
    } else {
        jmp_to_usercode(auxv_info.entry, (u64)stack);
    }
}

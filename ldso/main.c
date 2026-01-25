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

    auxv_info_t auxv_info;
    load_auxv_info(&auxv_info, auxv);

    dso_t bin;

    load_dso_from_auxv(&bin, &auxv_info,  argv[0], argv[0]);

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

    if (auxv_info.base != 0) {
        jmp_to_usercode(auxv_info.entry, (u64)stack);
    }

    if (argc <= 1) {
        exit_with_error("missing program name\n");
    }

    exit_with_error("direct loading not implemented yet\n");
}


// void ldso_main(u64 *stack)
// {

//     int argc = *stack;
//     char **argv = (void *)&stack[1];
//     char **envp = argv + argc + 1;

//     set_program_name(argv[0]);

//     if (argc <= 1) {
//         exit_with_error("missing program name\n");
//     }

//     dso_t bin;

//     load_dso_from_path(&bin, argv[1],  argv[1]);

//     linked_list_t dependencies;
//     linked_list_init(&dependencies);
//     linked_list_append(&dependencies, bin);
//     resolve_dependencies_recursive(
//         &dependencies,
//         &bin,
//         envp
//     );

//     // If LD_TRACE_LOADED_OBJECTS is set, print the loaded objects statuses
//     if (check_ld_trace_loaded_objects(envp)) {
//         print_link_map(&dependencies);
//         _exit(0);
//     }
    
//     linked_list_for_each(&dependencies, n) {
//         relocate_dso(&n->data, &dependencies);
//     }

//     jmp_to_usercode((u64)dso_resolve_ptr(&bin, bin.ehdr->e_entry), (u64)stack);

//     exit_with_error("direct loading not implemented yet\n");
// }
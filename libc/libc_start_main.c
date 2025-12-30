#include "unistd.h"
#include "stdio.h"

void __libc_start_main(int (*main)(int, char **, char **),
		       int argc, char **argv,
		       void *csu_init, void *csu_fini, void *dl_fini)
{
	// dl_fini is never defined in crt0.S so we won't use it here
	(void) dl_fini;

    // Using a local version of write so I can use it even without if I fail the relocation

	if (csu_init)
        ((void (*)(void))csu_init)();

    char **envp = argv + argc + 1;

    int ret = main(argc, argv, envp);

	if (csu_fini)
        ((void (*)(void))csu_fini)();

    _exit(ret);
}

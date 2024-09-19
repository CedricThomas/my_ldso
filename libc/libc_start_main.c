#include "unistd.h"

void __libc_start_main(int (*main)(int, char **, char **),
		       int argc, char **argv,
		       void *csu_init, void *csu_fini, void *dl_fini)
{
	(void)csu_init;
	(void)csu_fini;
	(void)dl_fini;

	/* FIXME: missing code */

	char **envp = argv + argc + 1;
	_exit(main(argc, argv, envp));
}

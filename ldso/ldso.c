#include <stdbool.h>
#include <elf.h>
#include <link.h>
#include <stddef.h>
#include <sys/auxv.h>
#include <asm/mman.h>
#include <asm-generic/fcntl.h>

#include "types.h"
#include "unistd.h"

ElfW(auxv_t) *get_auxv_entry(ElfW(auxv_t) *auxv, u32 type)
{
	for (; auxv->a_type != AT_NULL; auxv++)
		if (auxv->a_type == type)
			return auxv;
	return NULL;
}

ElfW(auxv_t) *find_auxv(char **envp)
{
	while (*envp != NULL)
		envp++;
	envp++;

	return (ElfW(auxv_t) *)envp;
}

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

	u64 entry = get_auxv_entry(auxv, AT_ENTRY)->a_un.a_val;

	jmp_to_usercode(entry, (u64)stack);
}

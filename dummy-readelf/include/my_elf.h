#include <stdint.h>

/* From elf.h */
/* Type of addresses. */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* from link.h */
/* We use this macro to refer to ELF types independent of the native wordsize.
`ElfW(TYPE)' is used in place of `Elf32_TYPE' or `Elf64_TYPE'. */
#define ElfW(type) _ElfW (Elf, __ELF_NATIVE_CLASS, type)
#define _ElfW(e,w,t) _ElfW_1 (e, w, _##t)

#define _ElfW_1(e,w,t) e##w##t

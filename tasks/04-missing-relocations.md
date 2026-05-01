# Task 4: Add Missing x86-64 Relocation Types

## Problem
Only `R_X86_64_RELATIVE`, `R_X86_64_GLOB_DAT`, and `R_X86_64_JUMP_SLOT` are handled. Missing relocation types are silently skipped (or print a message for Rela), which can cause subtle runtime crashes.

## Files
- `ldso/relocation.c` — `apply_rela_table()`, `apply_rel_table()`

## Changes
1. Handle `R_X86_64_PC32` (PC-relative 32-bit):
```c
case R_X86_64_PC32: {
    ElfW(Sym) *s = &info->dynsym[sym];
    uint64_t loc = (s->st_shndx == SHN_UNDEF) ? 0 : dso_resolve_ptr(dso, s->st_value);
    *where = loc + r->r_addend - (ElfW(Addr))where;
    break;
}
```

2. Handle `R_X86_64_64` (direct 64-bit):
```c
case R_X86_64_64: {
    ElfW(Sym) *s = &info->dynsym[sym];
    uint64_t loc = (s->st_shndx == SHN_UNDEF) ? 0 : dso_resolve_ptr(dso, s->st_value);
    *where = loc + r->r_addend;
    break;
}
```

3. Handle `R_X86_64_32` (direct 32-bit):
```c
case R_X86_64_32: {
    ElfW(Sym) *s = &info->dynsym[sym];
    uint64_t loc = (s->st_shndx == SHN_UNDEF) ? 0 : dso_resolve_ptr(dso, s->st_value);
    *(uint32_t *)where = (uint32_t)(loc + r->r_addend);
    break;
}
```

4. Handle `R_X86_64_32S` (signed 32-bit):
```c
case R_X86_64_32S: {
    ElfW(Sym) *s = &info->dynsym[sym];
    int64_t loc = (s->st_shndx == SHN_UNDEF) ? 0 : (int64_t)dso_resolve_ptr(dso, s->st_value);
    *(int32_t *)where = (int32_t)(loc + r->r_addend);
    break;
}
```

5. In `apply_rel_table()`, handle `R_X86_64_PC32` and `R_X86_64_32` similarly (without addend — Rel uses the target location for addend).

6. Make unknown relocation types **fatal** instead of silent — call `exit_with_errorf("unsupported relocation type %u at %p", type, where)`.

## Verification
- Build and run existing tests — must still pass.
- The new types cover what PIC executables and shared libs commonly need.

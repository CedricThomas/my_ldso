# Task 2: Uniform Bias in dso_resolve_ptr

## Problem
`dso_resolve_ptr` special-cases `DSO_KERNEL_MAPPED` by returning the raw vaddr, which only works because the kernel mapped the binary at preferred addresses. This is fragile and inconsistent with the bias model used for loader-mapped DSOs.

## Files
- `include/ldso.h` — `dso_resolve_ptr()` inline function
- `ldso/dso.c` — `load_dso_from_auxv()`, `load_dso_from_dynamic()`

## Changes
1. Always compute and store `obj->bias` (done in `load_dso_from_dynamic` already).
2. Change `dso_resolve_ptr` to **always** use `obj->bias + elf_addr` regardless of origin:

```c
static inline void *dso_resolve_ptr(dso_t *obj, ElfW(Addr) elf_addr) {
    if (!elf_addr)
        return (void *)0;
    return (void *)(obj->bias + elf_addr);
}
```

3. For `DSO_KERNEL_MAPPED`, ensure `bias` is computed correctly in `load_dso_from_dynamic` (it already is — `bias = base - min_vaddr`).
4. Verify `find_dynamic_in_auxv()` in `dyn.c` works correctly — it currently returns `(ElfW(Dyn) *)(ph[i].p_vaddr)` (raw vaddr). This is fine because auxv gives us kernel-mapped addresses that are already correct for kernel-mapped binaries. But after uniform bias, the caller should use `dso_resolve_ptr` instead.

## Verification
- Build and run `./ld.so ./test-standalone` — must still work.
- Verify `LD_TRACE_LOADED_OBJECTS=1 ./ld.so ./test-standalone` prints correct addresses.

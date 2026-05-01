# Task 3: Hash-Based Symbol Lookup (DT_GNU_HASH / DT_HASH)

## Problem
`resolve_symbol()` in `relocation.c` does a linear O(n*m) scan across all loaded DSOs and all their symbols. For any non-trivial program, this is slow.

## Files
- `ldso/relocation.c` — `resolve_symbol()`
- `include/ldso.h` — `dso_t` struct, `relocation_dyn_info_t`
- `ldso/dyn.c` — `parse_dynamic_reloc_info()`

## Changes
1. In `parse_dynamic_reloc_info()`, also parse `DT_HASH` (0x4) and `DT_GNU_HASH` (0x6ffffef5).
2. Add hash table pointers to `relocation_dyn_info_t`:
```c
typedef struct relocation_dyn_info {
    // ... existing fields ...
    
    /* GNU hash table (preferred) */
    const uint32_t *gnu_hash;
    size_t gnu_hash_sz;
    
    /* ELF hash table (fallback) */
    const uint32_t *elf_hash;
} relocation_dyn_info_t;
```

3. Implement `gnu_hash_lookup(const uint32_t *hash, size_t hash_sz, ElfW(Sym) *dynsym, const char *name)` returning the symbol index or UINT32_MAX.
4. Implement `elf_hash_lookup(const uint32_t *hash, ElfW(Sym) *dynsym, const char *name)` returning the symbol index or -1.
5. Rewrite `resolve_symbol()` to:
   - For each DSO, try gnu_hash_lookup first (if available), then elf_hash_lookup (if available).
   - On hash match, verify the symbol name with `strcmp` (hash collision check).
   - Only fall back to linear scan if no hash table is present.

6. Define `DT_GNU_HASH` locally if not in headers: `#define DT_GNU_HASH 0x6ffffef5`

## Verification
- Build and run `./ld.so ./test-standalone` — must still resolve all symbols correctly.
- The linear scan is kept as a fallback for robustness.

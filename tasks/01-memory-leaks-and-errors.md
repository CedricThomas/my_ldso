# Task 1: Fix Memory Leaks and Error Messages in Dependency Resolution

## Problem
`resolve_dependencies_recursive` leaks memory and has misleading error handling.

## Issues
1. `lib_data.path = strdup(resolved_path)` is called when `resolve()` fails — `resolved_path` is an empty string `""`. This creates a useless strdup and then tries to load an empty path.
2. Both `lib_data.path` and `lib_data.name` are always `strdup`'d but **never freed** anywhere.
3. When `resolve()` fails (`found == 1`), we still `strdup` the name and add a broken entry to the link map.
4. `obj->needed` (malloc'd in `load_dso_from_dynamic`) is **never freed**.

## Files
- `ldso/dso.c` — `resolve_dependencies_recursive()`, `load_dso_from_dynamic()`

## Changes
1. When `resolve()` fails, print a clear error: `cannot find library <name>` and call `exit_with_error`.
2. When `resolve()` succeeds, strdup the path. When it fails, don't strdup.
3. Add a cleanup function `free_dso(dso_t *obj)` that frees `obj->needed` (the array and each entry), `obj->path`, `obj->name` (if they were strdup'd).
4. In `resolve_dependencies_recursive`, free `lib_data.path` and `lib_data.name` after appending to the list (the list owns copies via `data_t` copy — or redesign so the list stores pointers).
5. Export `free_dso` in `ldso.h` and call it during cleanup (or at least in `ldso_main` before `jmp_to_usercode`).

## Expected result
- No memory leaks from strdup/malloc in the dependency resolution path.
- Clear "library not found" error instead of "failed to open DSO".

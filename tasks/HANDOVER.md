# Handover: Orchestrate ld.so Quality Fixes

## Context
We're improving a custom x86-64 dynamic linker (ld.so) written from scratch. 5 focused tasks are defined in `tasks/`. Fix them using subagents.

## Task dependency graph

```
01-memory-leaks-and-errors    (independent)
02-uniform-bias               (independent)
04-missing-relocations        (independent)
05-printf-64bit               (independent)
   ↓
03-hash-symbol-lookup         (after 02 — both touch ldso.h)
```

## Execution plan

**Wave 1** (parallel, 4 agents): tasks 01, 02, 04, 05
**Wave 2** (1 agent): task 03 (after wave 1 commits)

## Rules for each subagent

- Read the full task file in `tasks/<name>.md`
- Read all mentioned source files **before** coding
- Make focused changes — don't rewrite unrelated code
- After implementing, run `make` to verify it compiles
- If tests exist (`make test` or `./run_tests.sh`), run them
- Commit with a clear message: `fix: <what>` referencing the task number

## Rules for you (main agent)

1. Spawn wave 1 as 4 **parallel** foreground spawns using `tasks[]`
2. After all 4 complete, review the diffs, fix any merge conflicts, commit wave 1
3. Spawn wave 2 (task 03) as a single foreground spawn
4. After task 03 completes, do a final `make test`
5. Commit everything with a focused commit per task

## Subagent briefs

**Agent 1** (task 01): "Fix memory leaks and error messages in `ldso/dso.c`. Read `tasks/01-memory-leaks-and-errors.md` for details. Fix `resolve_dependencies_recursive` strdup leak, add `free_dso` cleanup, improve error messaging. Read `ldso/dso.c` and `include/ldso.h` first. Compile with `make` after changes."

**Agent 2** (task 02): "Unify address resolution in `dso_resolve_ptr` to always use bias. Read `tasks/02-uniform-bias.md` for details. Change the inline function in `include/ldso.h` and verify `load_dso_from_auxv` still works. Read `include/ldso.h` and `ldso/dso.c` first. Compile with `make` after changes."

**Agent 3** (task 04): "Add missing x86-64 relocation types (PC32, 64, 32, 32S) and make unknown types fatal. Read `tasks/04-missing-relocations.md` for details. Read `ldso/relocation.c` and `include/ldso.h` first. Compile with `make` after changes."

**Agent 4** (task 05): "Extend printf for 64-bit (ll qualifier) and increase buffer from 1024 to 4096. Read `tasks/05-printf-64bit.md` for details. Read `libc/printf.c` and `include/printf/boot.h` first. Compile with `make` after changes."

## After wave 1

- `git add -A && git commit -m "fix: address quality issues (leaks, bias, relocations, printf)"`
- Verify `make clean && make` succeeds
- Then spawn agent for task 03

## Agent 5** (task 03): "Implement hash-based symbol lookup (DT_GNU_HASH / DT_HASH). Read `tasks/03-hash-symbol-lookup.md` for details. This depends on wave 1 being committed. Read `ldso/relocation.c`, `ldso/dyn.c`, and `include/ldso.h` first. Add gnu_hash_lookup and elf_hash_lookup functions. Keep linear scan as fallback. Compile with `make` after changes."

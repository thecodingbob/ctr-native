---
name: memory-model
description: Work safely with CTR's resident layouts, MEMPACK arena, scratchpad, VRAM, overlays, pointer repair, and packed GPU links.
---

# Memory model

Use this skill for allocations, layouts, pointer conversions, scratchpad, checkpoints, and overlay data.

1. Read `docs/MEMORY_MODEL.md`, then `docs/DATA_SECTIONS.md` and `docs/OVERLAYS.md` as needed. `include/regionsEXE.h` maps resident `rdata`, `data`, `sdata_static`, and `bss`; `include/namespace_Main.h` defines the game tracker. Some layouts and offsets are part of game data contracts; inspect consumers before changing fields.
2. `platform/native_memory.c` owns the 8 MiB host MEMPACK backing buffer and 1 KiB scratchpad. `game/MEMPACK.c` owns allocator lifecycle: low allocations grow via `MEMPACK_AllocMem`, high allocations via `MEMPACK_AllocHighMem`, and bookmarks/reset operations alter lifetimes. Trace callers and existing pack boundaries rather than treating allocations as independent `malloc`s.
3. `Platform_RepairResidentPointers` reconnects native aliases into static game storage on initialization/restore. Check `platform/native_checkpoint.c`, `native_savestate.c`, and `native_state.c` when adding persistent or pointer-bearing state.
4. Use `CTR_SCRATCHPAD_PTR` from `include/ctr_scratchpad.h` for retail scratchpad offsets; shared offsets may be overwritten by nested helpers. Check a helper's call chain and offset lifetimes, particularly in collision, camera, and the 1P–4P geometry renderers.
5. GPU primitive tags carry a 24-bit link and 8-bit length, not a 24-bit CPU pointer. Preserve packet shape and route links through `platform/native_gpu_links.c` / `include/platform/native_gpu_links.h`. For VRAM CPU/GPU ownership and lazy synchronization, see the rendering skill.

The native binary is currently 32-bit by `CMakeLists.txt`; avoid silently assuming a pointer cast or layout will work on a wider host. Existing retail-specific branches are historical implementation details, not a mandate to retain PS1 limits when redesigning a subsystem.

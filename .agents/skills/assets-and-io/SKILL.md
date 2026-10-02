---
name: assets-and-io
description: Trace disc image and extracted-asset loading, CD facade, audio, input, and memory-card storage on native hosts.
---

# Assets and platform I/O

Use this skill for host-facing files, disc reads, audio/input integration, or saves.

1. `main.c` initializes paths and validates assets before starting the platform and game. `platform/native_assets.c` resolves assets relative to the executable base, indexes extracted files, and handles case-insensitive host paths; `platform/native_disc_image.c` reads the raw `assets/ctr-u.bin` image. Extracted assets override disc contents; see `README.md` Running for layout and requirements.
2. The game-facing loader is in `game/CDSYS.c` and `game/LOAD/`; CD/sector emulation is in `platform/native_cd.c`. Trace the entire path before changing read timing or buffer ownership. Disc data and raw XA/STR sectors have different formats.
3. Input mappings and pad snapshots are in `platform/native_input.c`, `platform/native_libpad.c`, and `game/GAMEPAD.c`. Audio is in `game/HOWL/` and `platform/native_audio.c` / `native_libspu.c`. Card semantics are in `game/MEMCARD/` with native storage in `platform/native_memcard.c` and `native_memcard_adapter.c`.
4. Keep OS/SDL and host filesystem logic in `platform/` with interfaces in `include/platform/` or `include/platform.h`; keep game rules in `game/`. Preserve local saves and asset overrides during debugging.

When testing file changes, try both disc fallback and an extracted override where relevant. For input/replay-sensitive changes, consult the debugging skill and `docs/REPLAYS.md`.

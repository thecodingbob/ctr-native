# CTR Native Expanded

A native PC port of Crash Team Racing (PS1, 1999) with extra additions. It builds on [CTR-tools/ctr-native](https://github.com/CTR-tools/ctr-native) and adds optional quality-of-life and customization features.

## Philosophy

- **No byte budget.** Game source lives in `game/` as our own copies. Edit freely.
- **No PSX toolchain.** Targets Windows and Linux with SDL3. No MIPS compiler needed.
- **Clean platform layer.** `main.c` owns process startup; host details stay in `platform/native_*`.
- **No build system nonsense.** Just `build.bat` / `build.sh`.
- **Fully static build.** Single executable, zero dependencies. SDL3 is compiled from vendored source and linked statically.

## Directory Layout

```
ctr_native/
  main.c              Entrypoint and native platform boundary
  platform/           Native-owned audio, input, memcard, CD, and PSX facade glue
  build-msvc.bat      Windows build (MSVC x86)
  build.bat           Windows build (MinGW i686)
  build.sh            Linux build
  CMakePresets.json   Shared CLion/command-line CMake configurations
  README.md           This file
  game/               Our copies of all decompiled game source (943 files)
    game_unity.h      Ordered unity include chain for all game source files
  include/            Project headers (structs, globals, declarations, platform facade)
  externals/
    SDL/              SDL3 source (static build)
```

## Prerequisites

### Windows

The recommended native Windows toolchain is MSVC x86:

1. Install Visual Studio 2022 or Visual Studio Build Tools 2022.
2. Select the **Desktop development with C++** workload and a current Windows SDK.
3. Ensure CMake 3.20 or newer is on `PATH` (standalone or the Visual Studio C++ CMake tools component).
4. Run `build-msvc.bat`, or select the `windows-msvc-x86` CMake preset in CLion.

The existing MinGW i686 build remains supported:

1. Install [MSYS2](https://www.msys2.org/).
2. In an MSYS2 terminal:
   ```
   pacman -Syu
   pacman -S --needed git mingw-w64-i686-gcc mingw-w64-i686-cmake mingw-w64-i686-make
   ```
   If the update asks you to close the terminal, reopen MSYS2 and run the install command.
3. Add `C:\msys64\mingw32\bin` to your system PATH
4. Open a new Command Prompt or PowerShell and run `build.bat`.

That's it. SDL3 is compiled from vendored source -- no separate install needed.

### Linux (Debian/Ubuntu)

```
sudo apt install gcc-multilib
sudo apt install libx11-dev libxext-dev libgl1-mesa-dev libasound2-dev libudev-dev libdbus-1-dev
```

## Building

```
build-msvc.bat       # Windows, MSVC x86 (recommended)
build.bat            # Windows, MinGW i686
chmod +x build.sh
./build.sh           # Linux
```

The shared CMake presets can also be used directly or selected as CLion CMake profiles:

```
cmake --preset windows-msvc-x86
cmake --build --preset windows-msvc-x86-debug
ctest --preset windows-msvc-x86-debug
```

First build compiles SDL3 from source. This is cached as a static library in the selected build directory.

Output:

- MSVC: `build-msvc-x86/Release/ctr_native.exe`
- MinGW: `build/ctr_native.exe`
- Linux: `build/ctr_native`

### Clean build

```
rmdir /s /q build    # Windows: delete cached libraries
build.bat            # Windows: rebuild everything

rmdir /s /q build-msvc-x86
build-msvc.bat       # Windows MSVC: rebuild everything

rm -rf build/        # Linux: delete cached libraries
./build.sh           # Linux: rebuild everything
```

## Running

### Normal Setup

If you downloaded a release build, you only need two things for normal play:

1. The game executable:
   - `ctr_native.exe` on Windows
   - `ctr_native` on Linux
2. Your own NTSC-U retail CTR disc image, named (put in directory called `assets`):
   - `assets/ctr-u.bin`

Example:

```
CTR-Native/
  ctr_native.exe
  assets/
    ctr-u.bin
```

Then run `ctr_native.exe`.

The disc image must be the common single-track raw PSX BIN layout: MODE2/2352 sectors, with the data track starting at byte 0. A cooked 2048-byte `.iso` does not preserve the XA/STR sector data needed for audio and video playback.

For development builds run from `build/`, put the same `assets/ctr-u.bin` next to the source tree:

```
ctr-native/
  build/
    ctr_native.exe
  assets/
    ctr-u.bin
```

### Extracted Asset Override

You do not need extracted assets for normal play.

Extracted files are still supported for development, modding, and debugging. If present, they override files from `ctr-u.bin`.

Extracted-asset override structure:

```
CTR-Native/
  ctr_native.exe
  assets/
    BIGFILE.BIG
    SOUNDS/KART.HWL
    TEST.STR
    XA/
      ENG.XNF
      ENG/EXTRA/S00.XA ... S05.XA
      ENG/GAME/S00.XA ... S20.XA
      MUSIC/S00.XA ... S01.XA
```

The full extracted asset list is:

- `BIGFILE.BIG`
- `SOUNDS/KART.HWL`
- `TEST.STR`
- `XA/ENG.XNF`
- `XA/ENG/EXTRA/S00.XA` through `S05.XA`
- `XA/ENG/GAME/S00.XA` through `S20.XA`
- `XA/MUSIC/S00.XA` through `S01.XA`

## Configuration

All settings can be changed at runtime from the in-game config menu
(Options → Config). This is the recommended way — changes take effect
immediately and are saved automatically.

For reference, the settings are persisted to `build/config.ini` (standard INI
format). An example template is at `default_config.ini` in the project root.

### General

| Key                               | Values                                                   | Default   | Description                                     |
|-----------------------------------|----------------------------------------------------------|-----------|-------------------------------------------------|
| `skip_intro`                      | `true` / `false`                                         | `false`   | Skip boot intros and go straight into main menu |
| `mask_mode`                       | `Normal` / `Random` / `Inverted` / `All Uka` / `All Aku` | `Normal`  | Mask assignment mode                            |
| `show_reserves_meter`             | `true` / `false`                                         | `false`   | Show the in-game reserves meter                 |
| `allow_oxide_station_multiplayer` | `true` / `false`                                         | `false`   | Allow selecting Oxide Station in multiplayer    |

### Adventure

| Key                         | Values             | Default | Description                                                                                      |
|-----------------------------|--------------------|---------|--------------------------------------------------------------------------------------------------|
| `skip_hints`                | `true` / `false`   | `false` | Skip mask hints in adventure mode                                                                |
| `extended_character_select` | `true` / `false`   | `false` | Use the classic screen to select any unlocked character when starting a new Adventure            |
| `save_anywhere`             | `true` / `false`   | `false` | Allows to save/load anywhere in the adventure mode hubs by pressing select or the equivalent key |
| `unlock_all_gates`          | `true` / `false`   | `false` | Opens all adventure wood gates bypassing the key requirements                                    |
| `unlock_all_portals`        | `true` / `false`   | `false` | Unlock all warp pads and boss garages                                                            |

### Vehicle

| Key                         | Values               | Default | Description                                                                                                              |
|-----------------------------|----------------------|---------|--------------------------------------------------------------------------------------------------------------------------|
| `speed_stat_multiplier`     | `10`–`200` (percent) | `100`  | Kart top-speed multiplier                                                                                                |
| `gravity_stat_multiplier`   | `10`–`300` (percent) | `100`  | Kart gravity multiplier. The lower the value, the more you'll spend in the air when jumping or dropping.                 |
| `turn_stat_multiplier`      | `10`–`400` (percent) | `100`  | Kart turn-rate multiplier. Higher values makes turning easier.                                                           |
| `jump_stat_multiplier`      | `10`–`300` (percent) | `100`  | Kart jump-height multiplier                                                                                              |
| `turbo_reserves_multiplier` | `0`–`400`  (percent) | `100`  | Reserve accumulation multiplier. The higher the value, the more reserve your kart will get when executing a turbo drift. |

### Weapons

| Key                                | Values                                                   | Default  | Description                           |
|------------------------------------|----------------------------------------------------------|----------|---------------------------------------|
| `missile_speed_multiplier`         | `20`–`500` (percent)                                     | `100`    | Missile speed multiplier              |
| `bomb_speed_multiplier`            | `20`–`300` (percent)                                     | `100`    | Bomb speed multiplier                 |
| `warpball_speed_multiplier`        | `20`–`300` (percent)                                     | `100`    | Warpball speed multiplier             |
| `bomb_explosion_radius_multiplier` | `50`–`600` (percent)                                     | `100`    | Bomb explosion radius multiplier      |
| `tnt_explosion_radius_multiplier`  | `50`–`600` (percent)                                     | `100`    | TNT/Nitro explosion radius multiplier |
| `mask_protects_from_damage`        | `true` / `false`                                         | `true`   | Mask makes driver immune to damage    |
| `mask_damages_others`              | `true` / `false`                                         | `true`   | Mask damages other drivers on contact |
| `mask_persists_after_oob`          | `true` / `false`                                         | `false`  | Mask stays active after falling OOB   |
| `mask_duration_multiplier`         | `20`–`250` (percent)                                     | `100`    | Mask weapon duration multiplier       |
| `mask_extra_speed_multiplier`      | `0`–`300` (percent)                                      | `100`    | Mask extra speed multiplier           |
| `clock_duration_multiplier`        | `20`–`250` (percent)                                     | `100`    | Clock slowdown duration multiplier    |
| `allow_weapons_during_clock`       | `true` / `false`                                         | `false`  | Allow weapon usage while clocked       |

### Graphics

| Key                        | Values  | Default | Description                                                             |
|----------------------------|---------|---------|-------------------------------------------------------------------------|
| `increase_draw_distance`   | `true` / `false` | `false` | Renders farther objects and uses higher poly models at higher distances |
| `disable_split_screen_lod` | `true` / `false` | `false` | Disable supported visual quality reductions in split-screen              |
| `fullscreen`               | `true` / `false` | `false` | Borderless fullscreen on startup                                        |
| `aspect_ratio`             | `4:3`, `16:9`, `16:10`, `21:9` | `4:3`   | Display aspect ratio                                                    |
| `dithering`                | `true` / `false` | `true`  | Enable color dithering                                                  |
| `render_scale`             | `Original`, `2X`, `3X`, `4X`, `Native` | `Native` | Internal render resolution; Native follows the presentation viewport |
| `smooth_scaling`           | `true` / `false` | `true`  | Use linear filtering when presenting scaled output                      |
| `texture_filtering`        | `true` / `false` | `false` | Enable bilinear filtering for PSX textures                              |

## Bug Replays

Internal builds can record a small bug report folder. See `docs/REPLAYS.md`.

## Architecture

```
main.c (entrypoint)
  |
  +-- platform/native_* (platform shell, audio, input, memcard, CD, renderer, PSX facade glue)
  |
  +-- game/game_unity.h
        |
        +-- game/ (all decompiled game source)
              |
              +-- include/ (headers: structs, globals, declarations)
```

- `CTR_NATIVE` is defined for native host/platform-specific code
- First-party native code targets portable C17 with compiler extensions disabled
- The default build uses 32-bit mode while remaining PSX address-shaped data and host-pointer contracts are audited. GPU primitive links are bridged through 24-bit native tokens; see `docs/MEMORY_MODEL.md`.

## Roadmap

- Clean up `game/` copies strip byte budget hacks and route platform-specific code through `CTR_NATIVE`
- Keep reducing 32-bit host-pointer assumptions in PSX-shaped data, and keep pruning inherited compatibility code now owned in `include/` and `platform/`.

## Credits

- [CTR-ModSDK](https://github.com/CTR-tools/CTR-ModSDK) — the decompilation project this is built on
- [PsyCross](https://github.com/OpenDriver2/PsyCross) — original PS1 compatibility code from which parts of CTR Native's owned platform layer and PsyQ facade headers are derived
- [SDL3](https://github.com/libsdl-org/SDL) — cross-platform multimedia
- [ctr-native-ap](https://github.com/dowlle/ctr-native-ap) by dowlle — render-scale, scaling-filter, and texture-filtering options
- Crash Team Racing is a trademark of Sony Computer Entertainment / Naughty Dog

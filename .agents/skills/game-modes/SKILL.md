---
name: game-modes
description: Change race, battle, adventure, time trial, menu, cup, player-count, or overlay transition behavior without missing mode gates.
---

# Game modes and transitions

Use this skill when a feature applies only in particular game modes or player counts.

1. Read `include/namespace_Main.h` first: `GameMode1` is a bitfield (`BATTLE_MODE`, `ARCADE_MODE`, `TIME_TRIAL`, `ADVENTURE_MODE`, `ADVENTURE_ARENA`, `RELIC_RACE`, `CRYSTAL_CHALLENGE`, `MAIN_MENU`, `END_OF_RACE`, etc.), not a single-choice enum. `GameMode2`, `GameRenderFlag`, and `GameHudFlag` are separate flag sets. The live state and current/next player counts are on `struct GameTracker` in that header, accessed via `sdata->gGT`.
2. Follow mode setup and frame transitions through `game/MAIN/MainGameStart.c`, `MainFrame.c`, `MainGameEnd.c`, `MainRaceTrack.c`, and `game/LOAD/LOAD_Level.c` / `LOAD_TenStages.c`. `game/230/` owns menu selection, `game/231/` race/battle gameplay, and `game/232/` adventure hub behavior.
3. Overlay indices remain meaningful even though native builds link all overlays: see `game/LOAD/LOAD_Overlays.c` and `docs/OVERLAYS.md`. Changing 1P–4P loads/selects different level renderers; region-3 transitions reset overlay-owned data and invoke callbacks.
4. For a new gate, enumerate combinations explicitly: player count (including 1P vs split-screen), race vs battle/crystal, adventure vs arcade/time trial/cup, start/end-of-race, pause/loading, and menu/cutscene. Do not assume absence of one flag uniquely identifies another mode. Reuse named masks/constants from `namespace_Main.h`.
5. Check `game/UI/UI_RenderFrame.c`, `game/MAIN/MainFrame_RenderFrame.c`, and end-event overlays for presentation effects of gameplay changes. For a new configurable behavior, also use the configuration skill.

Test the affected mode plus at least one adjacent mode and relevant player counts; state transitions often reveal bugs that an isolated race does not.

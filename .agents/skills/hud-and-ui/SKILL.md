---
name: hud-and-ui
description: Add or change HUD elements, menu screens, primitives, race overlays, and viewport-specific UI behavior.
---

# HUD and UI

Use this skill for heads-up display, 2D drawing, or menus.

1. Start with `game/MAIN/MainFrame_RenderFrame.c` (`RenderAllHUD`, `RenderDispEnv_UI`) and `game/UI/UI_RenderFrame.c` for racing, adventure hub, crystal challenge, and multiplayer HUD paths. Menu code is chiefly in `game/230/`; the configurable Options menu is `game/230/MM_ConfigMenu.c`.
2. Put drawing routines in the relevant `game/UI/` file and declare public game functions in `include/functions.h`. HUD positions come from the per-player `data.hudStructPtr` layouts; use `UI_HUD_SLOT_*` and `UI_HUD_SLOT_COUNT` from `include/namespace_UI.h` rather than numeric offsets.
3. Emit graphics through the existing primitive allocation/ordering-table path (`GetPrimMem`, `AddPrimitive`, `gGT->pushBuffer_UI.ptrOT` or the appropriate per-player push buffer). Follow neighboring routines for ordering, clipping, scale, and primitive lifetime; do not bypass the rendering pipeline with direct GL drawing.
4. Explicitly decide visibility in 1P and 2P–4P, battle and crystal modes, start/end-of-race, adventure hub, pause, and menu states. Use `gGT->hudFlags` and mode bits from `include/namespace_Main.h`. `UI_RenderFrame_Racing` and `UI_RenderFrame_CrystChall` have distinct call sites.
5. For an adjustable HUD feature, follow the runtime-configuration skill. The existing reserves meter is drawn from `game/UI/UI_Meter.c` and gated by `g_config.showReservesMeter` in the HUD paths.

Visually verify placement, layering, and visibility in each affected viewport/mode; check configuration-enabled and default-disabled behavior where applicable.

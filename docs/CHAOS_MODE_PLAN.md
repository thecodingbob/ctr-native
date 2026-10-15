# Chaos Mode — Implementation Plan

Plan only, no code written. Phases are sized to be separately buildable and
reviewable.

## 1. What is being built

A configuration option exposes Chaos mode. Starting an Adventure file asks
whether to arm it; answering yes opens a menu with an effect duration, a list of
effects to toggle, an **effect resolution** (the trigger condition) and a trigger
interval. Once armed, every Adventure race that is not the hub periodically
activates one enabled effect, which deactivates itself when its timer expires.

| Question             | Decision                                                                                              |
|----------------------|-------------------------------------------------------------------------------------------------------|
| Prompt placement     | Title/adventure-start flow in `game/230`, after the save slot is bound. See §3                        |
| Trigger condition    | `Random` / `Pickup` / `Interval` / `External`                                                         |
| External provider    | File-drop watch on `chaos/inbox/`, polled once per frame. §6 Phase 6                                  |
| Settings storage     | Inside the memory-card save, in `struct MemcardExtension`. Master switch stays in `config.ini`        |
| Trigger cadence      | Fourth menu row, shown when resolution is `Interval` or `Random`                                      |
| Who is affected      | Human players only. Effect table carries a scope field so bot effects need no structural change       |
| Which levels         | Every Adventure race type — normal, boss, relic, crystal, gem cup. Never the hub, never the main menu |
| Outside Adventure    | No. The gate is one pure predicate with the restriction on a single line. §4.5                        |
| Pickup trigger       | Contact, not grant                                                                                    |
| Re-prompt in the hub | No. Settings are written at creation and read back on load                                            |

## 2. Architecture

```
config.ini ──► g_config.chaosMode              (CFG_BOOL, "Adventure", default false)
                     │  gates the prompt only
                     ▼
title flow          MM_MenuProc_Main → MM_MenuProc_NewLoad → profile select
                     │  sdata->advProfileIndex bound
                     ▼
                Chaos_Prompt          YES/NO over the title flow
                     │  YES
                     ▼
                Chaos_SettingsMenu    duration / effect toggles / resolution / interval
                     │  commits into sdata->chaosSettings
                     ▼
save file  ◄──►  MemcardExtension.chaos[slot]     packed, per-slot, CRC-covered
                     │  SelectProfile_{Load,Save}ChaosSettings
                     ▼
                sdata->chaosSettings    live copy, in the SDATA checkpoint region
                     │
                     ▼
race (231)       Chaos_ArmRace()      at MainGameStart.c:17
                     ▼
                Chaos_Update()        at MainFrame.c:285
                     │                 ├─ ChaosGate_IsRaceEligible()      pure
                     │                 ├─ resolve trigger, pick an effect
                     │                 └─ tick remainingMS, deactivate at 0
                     ▼
                per-effect predicates  ChaosEffect_AppliesToDriver()  pure
```

Four layers, each testable alone:

| Layer      | Files                                            | Depends on                        |
|------------|--------------------------------------------------|-----------------------------------|
| Pure logic | `include/chaos_effect.h`, `include/chaos_gate.h` | base types only — no game headers |
| Engine     | `game/231/RB_Chaos.c`                            | `gGT`, `sdata`                    |
| Menus      | `game/230/`                                      | retail menu pipeline              |
| Host IO    | `platform/native_chaos_ipc.c`                    | filesystem only                   |

`chaos_effect.h` must stay dependency-free: `include/namespace_Memcard.h`
includes it for `CHAOS_EFFECT_COUNT`.

## 3. Prompt placement

Adventure start chain:

```
MM_MenuProc_Main      case LNG_ADVENTURE      game/230/MM_MenuFlow.c:178
 └─ MM_MENU_ADVENTURE NEW / LOAD               game/230/D230.c:134-148
     └─ MM_MenuProc_NewLoad                    game/230/MM_MenuFlow.c:652
         ├─ NEW  → MM_EXIT_ROUTE_ADV_NEW      game/230/MM_Title.c:100
         └─ LOAD → MM_EXIT_ROUTE_ADV_LOAD     game/230/MM_Title.c:134
             └─ profile select                game/SelectProfile.c
                 └─ SelectProfile_FinalizeAdventure
                                              game/SelectProfile.c:1432
```

`SelectProfile_FinalizeAdventure` is where both branches converge. `ADV_NEW`
(`:1460-1475`) and `ADV_LOAD` (`:1477-1501`) each bind
`sdata->advProfileIndex = menu->rowSelected;` then hand off with
`sdata->ptrDesiredMenu = QueueLoadTrack_GetMenuPtr();`.

**Insertion:** route to the chaos prompt before that hand-off in both branches,
when `g_config.chaosMode` is on. Factor the two hand-off lines into a helper so
the prompt's funcPtr can re-enter them without duplicating the queueing code.

The hub green load/save menu (`SELECT_PROFILE_SCREEN_GREEN_LOAD_SAVE`,
`:1437-1458`) is a separate entry into a loaded adventure and does not re-prompt.

Consequence: chaos cannot be armed mid-session. Re-arming would need a hub menu
entry — feasible (§7), not currently planned.

## 4. Data structures

### 4.1 Persisted, per-slot

Mirrors `struct AdvMultiplayerSettings` (4 bytes, `namespace_Memcard.h:619-625`).
In `struct MemcardExtension` rather than `struct MemcardProfile`, because the
profile size is baked into the on-disk magic: `GAMEPROG.c:244` writes
`header[1] = sizeof(struct MemcardProfile)` and `RefreshCard.c:557` matches the
literal `0x1600ffee`. Growing the profile invalidates existing saves.

```c
// Chaos setup for one Adventure slot. All-zero reads as "no chaos", which is
// what a save written before this feature existed reports.
struct ChaosSettings
{
	u16 durationSeconds;              // how long one activation lasts
	u16 triggerIntervalSeconds;       // cadence for Random and Interval
	u8 trigger;                       // enum ChaosTrigger
    bool effects[CHAOS_EFFECT_COUNT]; // one entry per effect, in enum order
};
```

**Sizing.** At 21 effects the struct is 26 bytes and `struct MemcardData` is
`0x1600 + 0x10 + 4 * 26 == 0x1678`. Bump `MEMCARD_FILE_DATA_SIZE`
(`namespace_Memcard.h:97`) to a round value covering it and use that constant for
the RAM buffer instead of the bare literal — `sdata->memcardBytes[0x1680]`
(`regionsEXE.h:3952`) and its GCC twin (`game/zGlobal_SDATA.c:5`). The adapter
already I/Os through the constant (`platform/native_memcard.c:784`, `:806`), as
does `SELECT_PROFILE_ADV_SAVE_BYTES` (`namespace_SelectProfile.h:40`). The
assertion at `namespace_Memcard.h:659` then catches the next batch of effects
landing without a resize.

The extension is already zero-initialised by `GAMEPROG_InitFullMemcard`
(`GAMEPROG.c:238`) and already CRC-covered (`MEMCARD_Checksum.c:29-32`).

### 4.2 Live copy — `sdata`

Appended after `advMultiplayer`, the current last member
(`regionsEXE.h:4093`):

```c
	// Chaos setup of the profile in use. Mirrored from the card extension on
	// load and written back on save.
	struct ChaosSettings chaosSettings;

	// Chaos session state. Armed by the title prompt, consumed by the race.
	int chaosArmed;
```

`sizeof(sdata_static)` is what `NATIVE_CHECKPOINT_REGION_SDATA` captures
(`platform/native_checkpoint.c:203-204`), so both get savestate and quick-state
coverage for free.

### 4.3 Dedicated RNG — `sdata->chaosRng`

`u32 chaosRng;` in the same block. Must not draw from `MixRNG_Scramble()` or
`RngDeadCoed()`: both are compared every recorded frame by
`NativeReplayScheduler_FrameInfoMatches` (`native_replay_scheduler.c:1282-1290`),
so one extra draw diverges the verifier on the next frame. Optionally add it to
`MainReplayScheduler_FrameInfo` (`MainMain.c:43-48`) so chaos divergence is
detected in replays too.

`rand()` / `PSX_BIOS_GetRandSeed()` is the alternative but is shared with crate
explosions and the item reel (`RB_Crate.c:139`, `:436`, `UI_Weapon.c:113`).

### 4.4 Per-race state — file-static

Active effect, remaining time and interval countdown live in file-statics in
`RB_Chaos.c`, following `MainFrame_Tow.c:29`. Nothing else reads them and they
are re-initialised every level load. Reset from `Chaos_ArmRace()`.

Not `D231`: `OVR231_InitData` runs only on an overlay *change*.

### 4.5 Race gate — pure predicate

Kept header-only so widening the scope later is an argument change, not an
archaeology exercise. `RB_Chaos.c` becomes a thin caller.

```c
// include/chaos_gate.h
enum ChaosGateScope
{
	CHAOS_GATE_ADVENTURE_ONLY,
	CHAOS_GATE_ANY_RACE,
};

int ChaosGate_IsRaceEligible(int gameMode1, int overlayIndex, int lightsGone, enum ChaosGateScope scope);
```

```c
int ChaosGate_IsRaceEligible(int gameMode1, int overlayIndex, int lightsGone, enum ChaosGateScope scope)
{
	if ((gameMode1 & (ADVENTURE_ARENA | MAIN_MENU | GAME_CUTSCENE | PAUSE_ALL | LOADING | END_OF_RACE)) != 0)
	{
		return 0;
	}
	if (overlayIndex != OVERLAY_INDEX_RACING_OR_BATTLE) { return 0; }
	if (!lightsGone) { return 0; }
	// Only Adventure. Delete this branch to widen the scope.
	if ((scope == CHAOS_GATE_ADVENTURE_ONLY) && ((gameMode1 & ADVENTURE_MODE) == 0)) { return 0; }
	return 1;
}
```

`ADVENTURE_MODE` is set at `MM_MenuFlow.c:185` and cleared only at `:160` and
`MM_Title.c:171`. `LOAD_TenStages.c:129` clears `GAME_CUTSCENE | END_OF_RACE |
ADVENTURE_ARENA | MAIN_MENU` on level change but leaves it, so the bit survives
the hub→race hand-off and covers every Adventure race type at once —
`RELIC_RACE` (`:182` in `MainGameEnd.c` masks the three together),
`CRYSTAL_CHALLENGE`, `ADVENTURE_CUP`, `ADVENTURE_BOSS`.

Battle arenas are correctly out of scope: not reachable from a hub, and
`MM_MenuFlow.c:160` clears `ADVENTURE_MODE` when entering Battle from the menu.

The one test needing live state, `sdata->ptrActiveMenu != NULL`, stays in
`RB_Chaos.c`.

### 4.6 Effect scope — pure test

```c
enum ChaosEffectScope
{
	CHAOS_SCOPE_HUMANS = 0, // every effect in the first cut
	CHAOS_SCOPE_ALL,        // future: humans and bots, e.g. HYPER_BOTS
};

int ChaosEffect_AppliesToDriver(enum ChaosEffectScope scope, int driverID, int numHumanPlayers);
```

Pass `driver->driverID` and `gGT->numPlyrCurrGame`. Bots are already numbered
above the human slots, the same convention `LOAD_Assets.c:58-61` and
`AH_WarpPad.c:504` rely on, so no new "is this a bot" flag is needed.

## 5. Effect catalogue

Grouped by what the effect changes, which determines the hook site and the side
effects to reason about. No effect needs a teardown hook: each completes on its
own or is simply not triggered again.

| Tier | What it changes                                             | Undo contract                                      | Count |
|------|-------------------------------------------------------------|----------------------------------------------------|-------|
| A    | driving constants and input, evaluated continuously         | implicit; only `const_*` need a snapshot           | 8     |
| B    | items, wumpa, the economy                                   | implicit for withholding; the reel resolves itself | 5     |
| C    | things the game does *to* the driver, and control takeovers | implicit                                           | 3     |
| D    | deferred                                                    | —                                                  | 5     |

### Tier A — per-frame modifiers

| Id                  | Effect                 | Scope  | Hook                                 | Mechanism                                                                                                                     |
|---------------------|------------------------|--------|--------------------------------------|-------------------------------------------------------------------------------------------------------------------------------|
| `INVERTED_STEERING` | Steering mirrored      | humans | `game/Vehicle/VehPhysProc.c:1338`    | mirror `stickLX` about the active centre                                                                                      |
| `NO_HOPPING`        | No hop, no power slide | humans | `VehPhysProc.c:877`, `:885`, `:2358` | force `RAIN_CLOUD_EFFECT_JUMP_LOCKOUT`; all three sites test it                                                               |
| `WOBBLE_STEERING`   | Steering jitters       | humans | after `VehPhysProc.c:1411`           | offset `steerStrength` before `storeSteerState`                                                                               |
| `LOW_GRAVITY`       | Karts float            | humans | `game/MAIN/MainFrame.c:285`          | write `driver->const_Gravity` (`namespace_Vehicle.h:1366`)                                                                    |
| `HEAVY_KARTS`       | Karts drop like anvils | humans | same                                 | same field, raised                                                                                                            |
| `SPEED_CAPPED`      | Top speed limited      | humans | same                                 | write `driver->const_Speed_ClassStat` (`namespace_Vehicle.h:1399`)                                                            |
| `SUPER_SPEED`       | Extreme speed          | humans | same                                 | raise `const_Speed_ClassStat` **and** `const_AccelSpeed_ClassStat` + `const_Accel_ClassStat`, or the top speed is unreachable |
| `NO_RESERVES`       | Boost meter empty      | humans | same                                 | zero `reserves` **and** `turbo_outsideTimer`, mirroring `VehPhysProc.c:918`                                                   |

`INVERTED_STEERING`: no d-pad handling needed. `GAMEPAD_ProcessSticks_ResolveAxis`
(`GAMEPAD.c:428-446`, called `:606-607`) already folds `BTN_LEFT`/`BTN_RIGHT`
into `pad->stickLX`. The block at `GAMEPAD.c:645-666` is the reverse conversion
(stick → d-pad) for menu code and runs afterwards. `:1338` is the single ingress
and `:1413-1448` the single resolution point. Mirror about
`ptrgamepad->rwd != NULL ? ptrgamepad->rwd->gamepadCenter :
VEH_PHYS_PROC_STICK_CENTER`, not a hardcoded `0x80` (`VehPhysJoystick.c:103-110`).

Top speed: `const_TerminalVelocity` is **not** a forward cap. It clamps the
kart-local vertical velocity at `VehPhysForce.c:355`; writing it shortens jumps
only. The lever is `const_Speed_ClassStat`, read every frame at
`VehPhysGeneral.c:1793`, feeding `targetBaseSpeed` → `gasTargetSpeed` →
`baseSpeed`/`fireSpeed` → the forward clamp at `VehPhysForce.c:297`. Never write
`baseSpeed` or `fireSpeed`; both are overwritten next frame.

Ceiling: `driver->speed` is hard-clamped to `0x6400` (25600) every frame at
`VehPhysForce.c:899,906-908`, and the base-speed source at
`VehPhysGeneral.c:1764,1864-1867`. `SUPER_SPEED` cannot exceed ~1.95× base
without raising those too — a separate deliberate change.
`g_config.speedMultiplier` scales `const_Speed_ClassStat` only, at birth
(`VehBirth.c:667-668`); no per-frame conflict.

`SPEED_CAPPED` and `SUPER_SPEED` are mutually exclusive — one field, two
directions. `Chaos_Pick` must refuse to co-select them.

`NO_RESERVES`: `UI_DrawSlideMeter` reads `turbo_MeterRoomLeft`, not `reserves`
(`UI_Meter.c:286-343`), so the HUD is unaffected. Zeroing `reserves` alone
desyncs the `FREEZE_RESERVES_ON_TURBO_PAD` refill at `VehFire.c:342-357`; every
retail damage path zeroes both (`VehPickState.c:300-301`). Note it also *lifts*
the speed cap (`VehPhysGeneral.c:1818-1833`) — decide whether that suits.

### Tier B — item and economy modifiers

| Id                | Effect                       | Scope  | Hook                               | Mechanism                                                          |
|-------------------|------------------------------|--------|------------------------------------|--------------------------------------------------------------------|
| `NO_ITEMS`        | Crates give nothing          | humans | `game/231/RB_Crate.c:192`          | clear `driver->heldItemID` after the reel                          |
| `PUNISHING_ITEMS` | Every crate gives TNT/beaker | humans | `game/231/RB_Crate.c:192`          | overwrite with a fixed `HELD_ITEM_*` (`namespace_Vehicle.h:641`)   |
| `WUMPA_DRAIN`     | Fruit drains                 | humans | `Chaos_Update` timer tick          | `RB_Player_ModifyWumpa(driver, -1)` (`RB_Player.c:141`)            |
| `CLOCK_CURSE`     | N. Tropy clock               | humans | `Chaos_Update` timer tick          | set `clockReceive`, reusing `VehPickupItem.c:813`                  |
| `CLEAR_BOOSTS`    | Pickups rob reserves         | humans | `game/Vehicle/VehPhysForce.c:1005` | zero `reserves` and `turbo_outsideTimer` after `VehFire_Increment` |

`VehPhysForce.c:1005` is the only turbo-pad grant site.

### Tier C — things done to the driver

| Id                    | Effect                            | Scope  | Hook                             |
|-----------------------|-----------------------------------|--------|----------------------------------|
| `ANXIOUS_MASK`        | The mask picks you up off road    | humans | `game/COLL.c:1491`–`:1667`       |
| `HARD_CRATES`         | Breaking a crate damages the kart | humans | `RB_Crate.c:123`, `:293`, `:420` |
| `BOT_TAKES_THE_WHEEL` | Players become CPU controlled     | humans | `game/MAIN/MainFrame.c:285`      |

**`ANXIOUS_MASK`** — the OOB rescue, not the mask weapon. Today the Aku/Uka is
only used when genuinely out of bounds, below the BSP floor, or on the kill
plane. This widens that.

One flag bit does it. `DRIVER_COLL_FLAG_MASK_GRAB_REQUEST = 0x1` of
`driver->collisionFlags` (`namespace_Vehicle.h:618-625`) is set from five places
(`VehPhysForce.c:969`, `COLL.c:1437`, `:1463` via
`COLL_FIXED_PlayerSearch_CheckMaskGrabProgress`, `:1664`, `:2376`) and consumed
in exactly one, `COLL.c:1667-1671`, which calls `VehStuckProc_MaskGrab_Init`
directly.

Hook inside `COLL_FIXED_PlayerSearch` between `COLL.c:1491` and `:1667`: in that
window `currentTerrain`, `terrainMeta1/2`, `quadBlockHeight`,
`currBlockTouching`, `lastValid` and `DRIVER_COLL_FLAG_GROUNDED` are all
current-frame and the request is consumed the same frame. Earlier reads last
frame's terrain; later misses the consumer.

Predicate:

```c
if (Chaos_IsActive(CHAOS_EFFECT_ANXIOUS_MASK) &&
    (d->collisionFlags & (DRIVER_COLL_FLAG_MASK_GRAB_REQUEST | DRIVER_COLL_FLAG_GROUNDED)) == DRIVER_COLL_FLAG_GROUNDED &&
    d->currentTerrain != TERRAIN_NONE &&
    d->terrainMeta2->speedMultiplier != VEH_PHYS_PROC_TERRAIN_SPEED_NEUTRAL &&
    d->kartState == KS_NORMAL && d->lastValid != NULL && d->speed < ceiling &&
    cooldown[d->driverID] <= 0)
{
	d->collisionFlags |= DRIVER_COLL_FLAG_MASK_GRAB_REQUEST;
	cooldown[d->driverID] = CHAOS_MASK_RESCUE_COOLDOWN_MS;
}
```

Off-road test is the game's own: `terrainMeta2->speedMultiplier != 0x100`
(`VehPhysProc.c:51`, applied `:1312-1322`), covering grass, dirt, mud, snow,
slow-grass, slow-dirt without per-level terrain ids. Pair with
`currentTerrain != TERRAIN_NONE` — airborne also reads as neutral.

Requirements found while researching this:

- **No airborne precondition.** `VehStuckProc_MaskGrab_Init:679` selects the
  falling tumble only when `posCurr.y > quadBlockHeight + 0x8000`; grounded takes
  the benign branch, so the sequence is safe while driving. Do not fake the
  height to get the crash variant — it plays a fall animation with no fall and
  snaps `posPrev` (`VehStuckProc.c:509-526`).
- **No cooldown exists.** `collisionFlags` is zeroed wholesale each frame at
  `VehPhysForce.c:950`; `numTimesMaskGrab` is a write-only statistic, never
  compare against it. The game's own floor is
  `VEH_STUCK_MASK_GRAB_INITIAL_TIMER = 1440` ms plus the rev-engine descent,
  ~5 s — not a guarantee. Keep a per-driver ms cooldown on the chaos side,
  decremented by `gGT->elapsedTimeMS`. `struct Driver` is growable, but
  `DRIVER_NTSC_RETAIL_SIZE` sizes the mempack bucket at `BOTS.c:3042`, so
  extending it changes allocation.
- **The rescue is a checkpoint rollback.** `VehStuckProc_MaskGrab_FindDestPos`
  resolves `lastValid` to `ptr_restart_points[lastValid->checkpointIndex]`
  (`VehStuckProc.c:235`) and overwrites `posCurr` (`:288-294`). On a grass verge
  that is potentially the whole checkpoint backwards. If the effect should feel
  like "put me back on the road", the stock path cannot do it — that needs its
  own `MaskGrab_Update` variant snapping to the nearest valid track quad.
  Decide before Phase 5.
- `MaskGrab_Init` does not zero `speed`, so gate on a speed ceiling or the kart
  is yanked sideways at full throttle.
- Each grab also: wipes turbo (`:639-641`), spawns then despawns a mask
  (`:637`, `RevEngine_Update:969`), locks the mask-grab camera ~4.8 s
  (`CAM.c:1320-1330`), and costs −2 wumpa on repeats (`:651`) since the previous
  grab consumed the mask that would have suppressed it.
- No existing option duplicates this: `maskPersistsAfterOOB` is read only at
  `VehStuckProc.c:967` and controls whether the mask *item* survives a rescue,
  not how often one happens.

**`HARD_CRATES`.** Hook the top of the `if (crateInst->scale.x == 0x1000)` block
in each of the three `RB_Crate.c` collide handlers — they are separate functions
with separate early returns, so factor a shared helper. Resolve the victim with
the switch each already has: `sps->Input1.modelID == DYNAMIC_PLAYER` gives
`collidingTh->object`, else `((struct TrackerWeapon *)collidingTh->object)->driverParent`.
Then `RB_Hazard_HurtDriver(victim, 2, attacker, 0)` (`RB_Hazard.c:4`) — the
funnel every real weapon uses. Type 2 (`BLAST`) routes to
`VehStuckProc_Tumble_Init` and the `BAKED_GTE_MATRIX_BLASTED` tumble, and is
re-entry guarded. Add `victim->damageColorTimer = 0x1e` and the `pushBuffer`
fade triple from `RB_Burst.c:115-117`; `RB_Hazard_HurtDriver` alone gives no HUD
feedback.

Cautions: `struct Driver *driver;` is left uninitialised at `RB_Crate.c:279`
when the hitter is unrecognised — null-check. `RB_Burst_CollLevInst`
(`RB_Burst.c:174-192`) reaches the same crates with the *weapon's* thread, not a
driver's. `VehStuckProc.c:1459` draws `MixRNG_Scramble()`, shifting
`sdata->randomNumber`. The wumpa penalty at `:1449` goes through
`RB_Player_ModifyWumpa`, which is mask-suppressed, so `HARD_CRATES` composes
with `ANXIOUS_MASK`.

`VEH_PICK_DAMAGE_*` / `VEH_PICK_REASON_*` are file-local to `VehPickState.c`,
included at `game_unity.h:228`, after `MainFrame.c` at `:122`. Hoist into a
header or declare chaos-local constants.

**`BOT_TAKES_THE_WHEEL`.** Takeover is free: `BOTS_Driver_Convert` (`BOTS.c:3065`)
already drives a live player, called on a real human at `PlayLevel.c:291`, and
`BOTS_ThTick_Drive` branches on `DYNAMIC_PLAYER` throughout (`:982`, `:2007`,
`:2074`, `:2677`, `:2694`). Hand-back does not exist —
`include/functions.h:1327-1336` is the whole `BOTS_*` surface, and the only
places that clear `ACTION_BOT` are lifecycle resets (`VehBirth.c:494`,
`GhostReplay.c:85`). Add `void BOTS_Driver_Decompare(struct Driver *d)` to
`game/BOTS.c` and call it from `Chaos_Deactivate`.

Snapshot before converting:

| Snapshot            | Why                                                                                                                                                                                  |
|---------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `distanceDriven`    | `UI_RaceEnd_GetDriverClock` (`BOTS.c:3073` → `UI_RaceFlow.c:41-58`) overwrites it with a derived average speed — destructive                                                         |
| `timeElapsedInRace` | same function reads it after freezing the clock                                                                                                                                      |
| `actionsFlagSet`    | `BOTS.c:3141` clears `ACTION_JUMP_BUTTON_HELD` and `ACTION_ACCEL_PREVENTION`; the AI sets `ACTION_TURBO_INPUT_LATCH` at `:1745`, which survives in `VEH_PHYS_PROC_ACTION_CARRY_MASK` |

`ACTION_RACE_TIMER_FROZEN` is the sharp one — `UI_RaceFlow.c:51` is its only
writer and **nothing ever clears it**. Without an explicit `&= ~`, the race clock
stays frozen at the takeover instant and final time and average speed are wrong
for the rest of the race. Snapshot/restore is safer than editing
`UI_RaceEnd_GetDriverClock`: it leaves retail code alone and the rescale is lossy.

The inverse:

- `d->instSelf->thread->funcThTick = NULL;` — alone re-enables the 13-stage loop
  (`MainFrame.c:310-313` skips any thread with non-null `funcThTick`); precedent
  at `AH_SaveObj.c:46`. `funcPtrs` was never cleared, only ignored, so it comes
  back intact — provided the driver never entered a `VehStuckProc_*` state while
  converted, worth asserting.
- `LIST_RemoveMember(&sdata->navBotList[botPath], &d->botData.item);` — inverse of
  `LIST_AddFront` at `BOTS.c:3133`. Without it other bots keep treating the kart
  as traffic (`BOTS.c:1095-1125`) and the savestate relocator
  (`native_checkpoint.c:1531-1533`) walks a human's `botData` as a list node.
- `memset(&d->botData, 0, sizeof(struct BotData));` — matching `BOTS.c:3101`.
- `BOTS_SetRotation(d, 0);` — as the convert path does at `BOTS.c:3135`.
  `botData.ai_rotY_608` drives `rotCurr.w` (`BOTS.c:2684`), which the camera
  reads; without this the camera snaps on hand-back.
- Restore `speed`, `speedApprox` and the `xSpeed`/`ySpeed`/`zSpeed` triple. The AI
  writes them directly (`BOTS.c:2527-2548`). `baseSpeed` and `fireSpeed` self-heal
  — `VehPhysProc.c:1325`, `:1288`/`:1305` recompute both from `gasTargetSpeed`.

Recovering automatically once `ACTION_BOT` clears: `BOTS_UpdateGlobals`
(`BOTS.c:466-493`) reclassification, camera L2 gate (`CAM.c:1841`), controller
rumble (`GAMEPAD.c:949`-`1044`), engine audio ceiling (`HOWL_Engine.c:241-253`).

Converting also changes, while active, 49 `ACTION_BOT` read sites at once:
lap checkpoint source (`VehLap.c:39`), damage dispatch to `BOTS_ChangeState`
(`RB_Hazard.c:8`), kart-to-kart bounce maths (`VehPhysCrash.c:329,338,419`), this
kart becoming a weapon target (`PickupBots.c:71`), and it becoming every other
bot's catch-up reference (`BOTS.c:475` — in 1P, its own target).

Two costs: `BOTS.c:1043` draws `RngDeadCoed(&sdata->advRng)` once per conversion,
compared every frame by the replay verifier; and `BOTS_Driver_Convert`
early-returns at `BOTS.c:3097` on levels with no usable nav path (Nitro Court,
`BOTS.c:145-151`), leaving a half-converted driver with a zeroed `botData` and no
`funcThTick` that freezes. Skip the effect there.

### Tier D — deferred

`HYPER_BOTS` is the worked example for the scope field: a `CHAOS_SCOPE_ALL`
effect raising every bot's speed class, touching no human driver. Plus
`BLIND_KART`, `GIANT_KARTS`, `TINY_KARTS`, `ITEM_RAIN`.

Total: 8 + 5 + 3 = 16 in the first cut, 21 with Tier D.

### Adding an effect later

One enum value, one table row, one implementation, one scope value if not
`HUMANS`, one tier. Nothing else moves — not the save format, not a size budget.
The IPC provider addresses effects by slug, so adding one does not break
existing providers.

## 6. Phase plan

### Phase 0 — branch and baseline

Topic branch off `master`. Confirm a clean build (`build.bat` / `build-msvc.bat`)
and green `ctest --test-dir build --output-on-failure`. Record the baseline check
count.

### Phase 1 — pure effect table, gate, tests

1. `include/chaos_effect.h` — header-only, dependency-free. `ChaosEffectId`
   enum, `CHAOS_EFFECT_COUNT` derived from its last member, `ChaosEffectScope`,
   label table, slug table, `ChaosEffect_FromSlug`, `ChaosEffect_FromIndex`,
   `ChaosEffect_IsEnabled`, `ChaosEffect_AppliesToDriver`,
   `ChaosEffect_Pick(const struct ChaosSettings *, u32 *rngState)`.
2. `include/chaos_gate.h` — `ChaosGateScope`, `ChaosGate_IsRaceEligible`.
3. `include/functions.h` — engine declarations.
4. `tests/test_chaos_effect.c`: every slug and index round-trips; unknown slug
   resolves to the none sentinel; picking with nothing enabled returns none;
   picking from a settings struct with one effect enabled always returns it
   (fails if the counting loop becomes a modulo); deterministic for a fixed seed;
   not biased to low ids over a large sample; `ChaosEffect_IsEnabled` reads the
   entry at the effect's own index (fails on an off-by-one).
5. `tests/test_chaos_gate.c`: hub, main menu, cutscene, pause, load and results
   each rejected; lights-not-gone rejected; non-racing overlay rejected; an
   Adventure race with each of `RELIC_RACE`, `CRYSTAL_CHALLENGE`, `ADVENTURE_CUP`
   and `ADVENTURE_BOSS` set accepted; the same race accepted under
   `CHAOS_GATE_ANY_RACE` without `ADVENTURE_MODE` (fails if the Adventure bit is
   hard-coded into the shared mask).
6. Also in the effect suite: `ChaosEffect_AppliesToDriver` matches every id below
   the human count and none at or above; id 0 with 0 humans matches nothing; a
   negative id matches nothing.
7. Also: `Chaos_Pick` never returns `SPEED_CAPPED` while `SUPER_SPEED` is active,
   nor the reverse.
8. Register in `tests/CMakeLists.txt` with `ctr_add_test_suite`.

### Phase 2 — configuration gate

1. `include/platform/native_config.h`: `bool chaosMode;` and
   `bool chaosExternalCommands;`, plus a `ChaosTrigger` enum.
2. `platform/native_config.c`: defaults in `g_config` (`false` both); two
   `ConfigEntry` rows inside the contiguous `Adventure` run (`:168-209`) —
   `chaos_mode` and `chaos_external_commands`, both `CFG_BOOL`.
3. `default_config.ini`: rows under `[Adventure]` with `;` comments. Check key
   names against `g_configEntries`, not the template — the template drifts.
4. `README.md`: rows in the `### Adventure` table plus a link to
   `docs/features/CHAOS.md`.
5. Optionally hide `chaos_external_commands` unless `chaos_mode` is on, via the
   pointer-identity switch in `NativeConfig_ApplyDependencies` (`:524-549`).

### Phase 3 — per-slot persistence

1. `include/chaos_effect.h` first, so `CHAOS_EFFECT_COUNT` exists.
2. `include/namespace_Memcard.h`: include `chaos_effect.h`, add
   `struct ChaosSettings` and the `MemcardExtension` field.
3. `include/regionsEXE.h`: `chaosSettings`, `chaosArmed`, `chaosRng` after
   `advMultiplayer`.
4. `game/GAMEPROG.c`: `GAMEPROG_ResetChaosSettings` beside
   `GAMEPROG_ResetAdvMultiplayer` (`:334-338`), called from
   `GAMEPROG_NewGame_OnBoot` (`:329`).
5. `game/SelectProfile.c`: `SelectProfile_{Load,Save,Clear}ChaosSettings` beside
   their `AdvMultiplayer` twins (`:727-753`). Load from
   `SelectProfile_LoadAdvProfile` (`:762`); clear in the profile delete branch
   next to `:1193-1194`.
6. Resize `MEMCARD_FILE_DATA_SIZE` and the two `memcardBytes` arrays per §4.1.
7. `tests/memcard/test_chaos_settings.c` — the first memcard suite in the repo.
   Cover: all-zero reads as chaos-off with every effect disabled; round trip
   preserves every field including each individual toggle; a save without the
   feature loads as chaos-off; the delete path clears the slot; and a struct with
   *some* effects set round-trips without clearing its neighbours, which fails on
   a loop bound that assumes a fixed effect count. Use the
   `tests/controls/test_native_controls.c` include pattern; point
   `NativeAssets_GetBaseDir` at `CtrTestScratch_Open`, never the working
   directory.

### Phase 4 — prompt and settings menu

1. Prompt `struct RectMenu` + rows in overlay 230 data, `include/ovr_230.h`.
   `D230` has no `OFFSETOF` assertions, so appending is free. Do **not** add the
   menus to `D230.arrayMenuPtrs` (`:702`, `MM_MENU_RESET_COUNT = 9`): that feeds
   `MM_ResetAllMenus` (`MM_MenuFlow.c:704-728`), which only walks
   `ptrNextBox_InHierarchy` chains, and the chaos prompt is reached via
   `sdata->ptrDesiredMenu`. It needs its own reset on abort.
2. Prompt behaviour mirrors `MM_MenuProc_QuitConfirm` (`MM_MenuFlow.c:302-318`) —
   `RECTMENU_CLOSE_TRANSIENT`, `LNG_YES`/`LNG_NO`, cursor on NO,
   `rowSelected == -1` is back. Back and NO both continue unarmed.
3. Settings menu is a separate screen, not `MM_ConfigMenu`: the effect list is
   dynamic in length, `CFG_INT` always renders as a percentage
   (`MM_ConfigMenu.c:197-222`), `CFG_ENUM` cycling assumes values `0..n-1`
   (`:327`), and the section list does not scroll and is capped at 16
   (`:360-400`). Build rows per frame; draw with `DecalFont_DrawLineOT` and
   `CTR_Box_DrawClearBox` into `gGT->pushBuffer_UI.ptrOT`; reuse
   `CONFIG_ROW_START_Y` / `CONFIG_ROW_SPACING` / `CONFIG_LABEL_X` /
   `CONFIG_VALUE_X` / `CONFIG_MAX_VISIBLE_ROWS` from `MM_ConfigMenu.c:105-117`
   so the screens match. Scroll with the wrap-around from `:274-303` and the
   more-than-five-arrows affordance from `AH_HintMenu.c:162-191`.
4. Rows: **Duration** (left/right seconds), one per effect (X/circle),
   **Effect Resolution** (left/right cycles the four triggers), **Trigger
   Interval** (left/right seconds). The last row draws only when resolution is
   `Interval` or `Random`; otherwise it is hidden and the list closes the gap, so
   the cursor never lands on an invisible row.
5. If `External` is selected but `chaos_external_commands` is off, draw the row
   unavailable rather than silently doing nothing.
6. Commit on exit into `sdata->chaosSettings`. Register both menus for checkpoint
   relocation in `NativeCheckpoint_RelocateD230Pointers`
   (`native_checkpoint.c:1557`), the way `NativeCheckpoint_RelocateRectMenu`
   handles the `D232` menus (`:1649-1650`) — otherwise a quick state taken with
   the menu open restores a stale `rows` pointer.

### Phase 5 — engine and effects

1. `game/231/RB_Chaos.c`, added to the 231 block of `game/game_unity.h`
   (`:249-286`). Prefix everything `Chaos_` — file-local identifiers collide
   across unity includes.
2. `Chaos_ArmRace(gGT)` from `MainGameStart_Initialize`
   (`MainGameStart.c:17`, beside `gGT->trafficLightsTimer = 0xf00;`). Resets
   per-race state, snapshots `const_*` fields, no-ops when
   `!g_config.chaosMode || !sdata->chaosArmed`.
3. `Chaos_Update(gGT)` from `MainFrame_GameLogic` (`MainFrame.c:285`, beside
   `MainFrame_Tow_Update`) — inside the `PAUSE_ALL == 0` guard, after
   `elapsedEventTime` is finalised (`:212`) and before the `funcPtrs` stage loop
   (`:304`).
   ```c
   if (!sdata->chaosArmed) return;
   if (sdata->ptrActiveMenu != NULL) return;
   if (!ChaosGate_IsRaceEligible(gGT->gameMode1, gGT->overlayIndex_Threads,
                                 gGT->trafficLightsTimer < 1,
                                 CHAOS_GATE_ADVENTURE_ONLY)) return;
   ```
4. Trigger resolution: `Random` — after each effect expires wait a gap drawn from
   `sdata->chaosRng` in `[0.3, 1.0] × triggerIntervalSeconds`, then activate a
   random enabled effect. `Pickup` — at the contact sites below. `Interval` —
   every `triggerIntervalSeconds`. `External` — only from the IPC watcher.
5. Timer: `remainingMS -= gGT->elapsedTimeMS` (ms, matching
   `RB_MaskShieldCloud.c:184`), deactivate at `<= 0`.
6. Deactivation in the same tick, plus a belt-and-braces reset in
   `Chaos_ArmRace` and a clear in `MainGameEnd_Initialize`
   (`MainGameEnd.c:669`) so a stale effect cannot survive into the hub or the next
   race. Restore snapshotted `const_*`.
7. `BOT_TAKES_THE_WHEEL` needs the new `BOTS_Driver_Decompare` (§5, Tier C).
   Three easy-to-miss items: clearing `ACTION_RACE_TIMER_FROZEN`,
   `LIST_RemoveMember`, restoring `distanceDriven`. Skip the effect where
   `BOTS_Driver_Convert` early-returns (`BOTS.c:3097`).
8. Every per-frame site asks
   `ChaosEffect_AppliesToDriver(def->scope, driver->driverID, gGT->numPlyrCurrGame)`
   before touching a driver.
9. Effect hooks: Tier A per §5; `ANXIOUS_MASK` inside `COLL.c:1491`–`:1667`;
   `HARD_CRATES` at the three `RB_Crate.c` break sites; `CLEAR_BOOSTS` at
   `VehPhysForce.c:1005`; `NO_ITEMS` / `PUNISHING_ITEMS` at `RB_Crate.c:192`.

Pickup trigger sites (contact, verified as the single point per collectible):

| Collectible  | Site                                                     |
|--------------|----------------------------------------------------------|
| item crate   | `game/231/RB_Crate.c:192`                                |
| wumpa crate  | `game/UI/UI_RenderFrame.c:243` (shared with loose fruit) |
| time crate   | `game/231/RB_Crate.c:484`                                |
| crystal      | `game/UI/UI_RenderFrame.c:1059`                          |
| C/T/R letter | `game/231/RB_CtrLetter.c:30`                             |
| turbo pad    | `game/Vehicle/VehPhysForce.c:1005`                       |

There is no central "player X collected Y" dispatch — `RB_Pickup_SetCamera`
(`RB_Pickup.h:16`) is the only function every successful pickup calls and carries
no pickup type, so this is N call sites. `CollMoved_PlayerSearch_RunHitboxLInC`
(`COLL.c:2184`) catches every level instance including turtles and teeth gates,
and fires even when the handler declines the pickup.

Validation: drive a race with `Random`, confirm effects arrive and expire and the
driver returns to normal. For `ANXIOUS_MASK`: fires on grass, not on the racing
surface, not airborne, cooldown prevents a loop — and check where you are actually
put down, since §5 warns the stock path is a checkpoint rollback. Hub visit
freezes everything; pause freezes the timer; results screen does not tick. Bots
untouched; Battle arena out of scope. Split-screen 2P hits both humans. For
`BOT_TAKES_THE_WHEEL`: after expiry the race clock still runs, final time and
average speed are sane, rumble and L2 zoom work again, and `sdata->navBotList`
no longer holds the player's `botData` — take a quick state mid-takeover and load
it after expiry to prove the list survives. A `Random` run must replay
identically; `HARD_CRATES` and `BOT_TAKES_THE_WHEEL` will not (§7).

### Phase 6 — external provider IPC

1. `platform/native_chaos_ipc.c` + `include/platform/native_chaos_ipc.h`.
2. Poll once per frame from `MainMain.c` between `sdata->frameCounter++` (`:297`)
   and `GAMEPAD_ProcessAnyoneVars` (`:318`) — same slot as
   `NativeReplayScheduler_BeginFrame`, so a command lands on the frame it is
   read. Main thread only; no synchronisation needed, the only background thread
   in the project is SDL's audio device thread and this never touches it.
3. Throttle to every 6 frames via `SDL_GetPathInfo`; cap at one command per poll.
4. Protocol: inbox `<base>/chaos/inbox/`, outbox `<base>/chaos/outbox/`. Provider
   writes `<id>.cmd.tmp` and renames to `<id>.cmd`; skip names starting with `.`.
   First line is `effect <slug> [seconds]` (seconds overrides the slot duration
   for that activation), `status`, or `reset`. Game deletes the file after
   consuming and writes `<id>.reply`. Slugs are the stable names from
   `chaos_effect.h`, so adding an effect does not break providers.
5. Unknown slug → reply `error unknown-effect`, no crash, do not consume silently.
6. Disable the watcher during `--replay` playback, and document that a recording
   made while a provider was driving the game will not verify unless the provider
   replays the same commands in the same order.

### Phase 7 — documentation and polish

1. `docs/features/CHAOS.md` in house style: hook paragraph, then `##` sections as
   second-person tasks — enabling, arming, configuring, what each effect does,
   the provider protocol. Bold for on-screen labels, backticks for keys.
2. `README.md`: link from the `chaos_mode` row in the Adventure table and from
   the Configuration prose, matching how `CONTROLS.md` and `2P_ADVENTURE.md` are
   linked.
3. State the co-op interaction explicitly: every human driver is hit, bots are
   not, so in a 2P file both players are affected and the CPU field is not. Also
   state the level scope — every Adventure race type, never the hub.
4. Consider documenting the `MemcardExtension` mechanism itself; it is the second
   feature to use it and currently exists only as source comments.

## 7. Known landmines

| Risk                                         | Detail                                                                                                                              | Mitigation                                                                               |
|----------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------|
| `const_TerminalVelocity` is not top speed    | Clamps the kart-local **vertical** velocity at `VehPhysForce.c:355`; the name reads as a forward cap                                | Use `const_Speed_ClassStat` (§5, Tier A)                                                 |
| `SPEED_CAPPED` vs `SUPER_SPEED`              | One field, two directions; co-active, evaluation order decides                                                                      | `Chaos_Pick` refuses to co-select (Phase 1.7)                                            |
| `turbo_outsideTimer` travels with `reserves` | Zeroing only `reserves` desyncs `VehFire.c:342-357`                                                                                 | Always zero both                                                                         |
| Mask grabs have no cooldown                  | `collisionFlags` zeroed every frame at `VehPhysForce.c:950`; nothing rate-limits; `numTimesMaskGrab` is write-only                  | Chaos-side per-driver ms timer                                                           |
| Mask rescue is a checkpoint rollback         | `VehStuckProc.c:235` jumps to `ptr_restart_points[lastValid->checkpointIndex]` and overwrites `posCurr`                             | Decide in Phase 5 whether to accept or write a nearest-track variant                     |
| Effects that consume retail RNG              | `VehStuckProc.c:1459` and `RB_Crate.c:312` call `MixRNG_Scramble()`; `BOTS.c:1043` draws `advRng`                                   | `HARD_CRATES` and `BOT_TAKES_THE_WHEEL` will not replay cleanly; accepted and documented |
| `rowsAdvHub` growth                          | Shifts `playerIconAdvMap` and everything after it, so `OFFSETOF_DATA(playerIconAdvMap)` (`regionsEXE.h:4190`) and neighbours change | Churn only — a dozen assertion edits in one header. Not currently needed                 |
| RNG determinism                              | `MixRNG_Scramble()` and `RngDeadCoed()` are compared every recorded frame (`native_replay_scheduler.c:1282`)                        | Dedicated `sdata->chaosRng`                                                              |
| `D231` reset                                 | `OVR231_InitData` runs only on overlay *change*                                                                                     | Keep per-race state in file-statics                                                      |
| `D232` reset                                 | `OVR232_ResetRuntimeState` (`D232.c:300`) runs every hub load                                                                       | Menus may live there; settings must not                                                  |
| Config menu limits                           | 16-section cap, `CFG_INT` renders as percent, `CFG_ENUM` assumes `0..n-1`, section list does not scroll                             | Custom settings screen (Phase 4.3)                                                       |
| Memory-card corruption                       | Late-game high-Relic saves are a known retail risk and there is no memcard test suite                                               | Phase 3's suite is mandatory                                                             |
| Replay with a live provider                  | Commands are not reproducible from the replay file                                                                                  | Gate the watcher during playback                                                         |

## 8. Open questions

1. **Hub pause-menu access.** Chaos cannot be armed mid-session. The layout
   permits a hub entry (§7), it is just not planned.
2. **Battle arenas.** Out of scope today. When chaos widens past Adventure,
   `BATTLE_MODE` races come with it and need a battle-appropriate effect subset.
3. **`ANXIOUS_MASK` destination.** Accept the checkpoint rollback, or write a
   nearest-track variant of `VehStuckProc_MaskGrab_Update`? Changes the feel
   entirely.
4. **`HARD_CRATES` RNG.** Accept that it breaks replay verification, or
   snapshot-and-restore `sdata->randomNumber` around the tumble?

## 9. Estimated shape

| Phase | Touches                                                                                                    | Notes              |
|-------|------------------------------------------------------------------------------------------------------------|--------------------|
| 0     | —                                                                                                          | baseline           |
| 1     | `include/chaos_effect.h`, `include/chaos_gate.h`, `include/functions.h`, `tests/`                          | no game code       |
| 2     | `include/platform/native_config.h`, `platform/native_config.c`, `default_config.ini`, `README.md`          | smallest           |
| 3     | `include/namespace_Memcard.h`, `include/regionsEXE.h`, `game/GAMEPROG.c`, `game/SelectProfile.c`, `tests/` | save format        |
| 4     | `game/230/`, `include/ovr_230.h`, `platform/native_checkpoint.c`                                           | largest, pure UI   |
| 5     | `game/231/RB_Chaos.c`, `game/BOTS.c`, `game/game_unity.h`, `game/MAIN/*`, `game/Vehicle/*`, `game/COLL.c`  | most gameplay risk |
| 6     | `platform/native_chaos_ipc.c`, `game/MAIN/MainMain.c`                                                      | isolated           |
| 7     | `docs/`, `README.md`                                                                                       |                    |

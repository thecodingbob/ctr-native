# Chaos Mode — Remaining Implementation

## Status

Implemented: catalogue/selection helpers, race gate, creation flow, per-slot
persistence, save marker, pause inspection and runtime timers/Interval/Random scheduling.
Inverted Steering, No Hopping, Wobble Steering, Low Gravity, Heavy Karts,
Speed Capped, Super Speed, No Reserves, No Items, Wumpa Drain, Clock Curse,
Explosive Crates and Defective Boosts are
connected, with an activation-name overlay.
Pending: remaining effects, Pickup/External triggers, provider and player guide.
Save layouts and checkpoint ownership are documented in [CHAOS_STATE.md](CHAOS_STATE.md).

### Established rules

- `chaos_mode` gates the creation prompt only; `chaosSettings.armed` identifies
  the loaded file's mode. Loading never prompts.
- New resets to Regular with all catalogue effects selected. Flow: Mode →
  optional Players → Chaos setup when armed → character selection. Back retains
  choices. Defaults live in `namespace_Memcard.h`.
- Creation settings are immutable afterwards. Confirm requires a selected effect;
  otherwise it is gray and activation plays `MENU_SFX_LOCKED`.
- Cross/Circle activates choices/actions; Left/Right changes numeric sliders.
  Triangle returns from Effects, which has no Back row.
- Chaos Rules in Adventure pause reuses the setup UI read-only. Active Effects
  lists selected effects, not running activations, without ON/OFF or empty text.
- Effects target humans in all Adventure race types, never hubs or other modes.
- Pickup means contact. Activations accumulate with independent timers, including
repeats; later conflicting activations deactivate the previous conflicting effects.

## Runtime core

Owner: `game/231/RB_Chaos.c`, registered in `game/game_unity.h`, with declarations
in `include/functions.h`. Host IO stays in `platform/`.

The implemented core stores ordered activations and cadence in
`sdata->chaosRuntime`. Race initialization follows vehicle teleport; updates run
before driver stages and check eligibility, `armed`, menus and pending loads.
Pause freezes game-time timers. Expiry precedes scheduling; new entries receive
full duration and cadence retains overshoot. The four-entry queue rejects overflow
without eviction/refresh; rejected scheduled events consume their gap but no RNG.
Race end, retry teardown and level-load stage 0 clear the state.

For gameplay effects:

1. Add baselines, cooldowns, contact latches and takeover state to captured storage;
   relocate retained pointers. File-static mutable state alone is insufficient.
2. Restore Chaos-owned baseline constants **once per driver**, then apply active
   overrides in activation order. Restore even after the last entry expires.
   Per-entry baseline resets erase unrelated effects; activation snapshots can
   capture another effect's modified values.
3. Separate one-shot events, periodic pulses, continuous modifiers and ownership
   transitions. AI takeover acquires control once and hands back after the last
   relevant activation expires.
4. Extend reset hooks to undo temporary ownership before drivers are destroyed.
   Overlay changes alone are insufficient: 231 may stay loaded between races.

Capture effective gravity/speed/acceleration baselines after character/config
initialization. Preserve legitimate game changes outside Chaos-owned fields.
Use `sdata->chaosRng` for scheduling/selection, never retail RNG.

### Triggers

| Trigger | Behavior |
| --- | --- |
| Interval | One selected effect each configured interval |
| Random | One selected effect per randomized gap in `[0.3, 1.0] × interval`, including the first gap |
| Pickup | One activation per eligible human contact |
| External | Provider commands only |

None waits for earlier effects to expire. An empty selection yields no activation
and does not change `armed`.

Pickup hook candidates: crate collision handlers in `RB_Crate.c`,
`RB_Fruit_ThCollide`, `RB_Crystal_ThCollide`, `RB_CtrLetter.c`, and turbo terrain
handling in `VehPhysForce.c`. Validate contact and human identity before dispatch.
Weapon ownership is not a pickup contact. Wumpa/crystal increments in
`UI_RenderFrame.c` are delayed HUD awards, not trigger sites.
Debounce collectible lifecycles and turbo-pad traversal; sustained pad contact
must not activate every frame. `RB_Pickup_SetCamera` lacks the necessary context
for a central dispatcher; generic level collisions also include hazards.

## Effect hooks

All 16 effects have catalogue IDs/toggles. Thirteen are implemented; the
remaining rows are source leads to verify during implementation.

| Effects | Candidate mechanism |
| --- | --- |
| Inverted Steering — done | Mirror a local axis about the active controller center; D-pad already feeds stick resolution; duplicates do not cancel |
| No Hopping — done | Clear resolved manual hop latch/buffer; preserve drift input/states, forced jumps and rain-cloud ownership |
| Wobble Steering — done | Two-second sine wave at 75% of the controller range, clamped around its center; deactivates Inverted Steering and vice versa |
| Low Gravity / Heavy Karts — done | Derive effective gravity from the live constant at 1/4 or 3×, before terrain rules; deactivate each other |
| Speed Capped / Super Speed — done | Cap the forward target at half the live class speed / double resolved forward target and acceleration; retain retail hard limit and deactivate each other |
| No Reserves — done | Clear both fuel timers only in the existing driver hook; retail boost grants persist until the next application; allow new fuel after expiry |
| No Items — done | Empty inventory and cancel pending launches/roulette through one pre-item hook; keep already deployed effects and discard fresh rewards at the next check |
| Explosive Crates — done | Direct kart contact with item, Wumpa or time crates triggers an explosion and blast damage, bypassing protection; retain normal rewards |
| Wumpa Drain — done | Detect crossed seconds from each activation's elapsed time in the existing driver hook; independent duplicate pulses, stop at zero and preserve pending pickup awards |
| Clock Curse — done | One-off spin hit, clock audio and full 32-bit received-clock timer, bypassing protection; new activations overwrite earlier clocks |
| Defective Boosts — done | Invert the live fire-speed bonus while reserves remain; preserve normal fuel grants, accumulation and consumption for every boost source |
| Anxious Mask | Request rescue during current-frame collision processing, before mask-request consumption |
| Hard Crates | Shared damage helper across three crate handlers, validating victim and collision type |
| Bot Takes Wheel | Guarded `BOTS_Driver_Convert` plus explicit human-control hand-back |

Key implementation constraints:

- `const_TerminalVelocity` caps vertical velocity, not forward speed. Speed effects
  derive local targets/acceleration without overwriting constants. The existing
  `0x6400` target clamp limits Super Speed; slopes/collisions remain retail.
- Rescue needs captured per-driver cooldowns, grounded/off-road/state checks and
  valid restart data. Stock rescue returns to a checkpoint, changes camera/input,
  wipes turbo and can cost wumpa. Destination and speed ceiling remain open.
- Explosive Crates reuses the retail damage implementation with protection bypass;
  weapon hits retain ordinary crate behavior. Hard Crates remains separate.
- Takeover must handle missing nav paths without partial conversion. Audit clock
  freezing/`distanceDriven`, action latches, thread ownership, nav-list links,
  `botData`, rotation, velocity and damage states. Restore only owned state;
  stale whole-action/motion snapshots must not undo race progress. Checkpoint
  restoration must support takeover and subsequent hand-back.
- Hard Crates and Bot Takes Wheel can consume retail RNG. Their replay
  incompatibility is accepted; do not hide it by restoring retail RNG.

Deferred: Hyper Bots, Blind Kart, Giant Karts, Tiny Karts, Item Rain.
Append stable IDs/slugs; the current disk bitset supports 128 effects.

## External provider

Proposed `platform/native_chaos_ipc.c`: bounded main-thread polling of
`<base>/chaos/inbox/`, replies in `chaos/outbox/`, atomic `.tmp` → `.cmd` publication.
Proposed commands: `effect <slug> [seconds]`, `status`, `reset`.

Before implementation, define polling/frame order, command ordering, duration
bounds, disabled-effect rejection, ineligible-race handling, consumption and reset
semantics. Commands share the engine activation path and respect the saved
trigger/selection; they never edit creation settings.
Disable the watcher during replay playback. External command timing is not
currently recorded, so provider-driven replay verification is not guaranteed.

## Delivery and checks

1. Verify the playable effects in game: Interval/Random with only Inverted Steering,
   No Hopping, Wobble Steering, Low Gravity, Heavy Karts, Speed Capped, Super Speed
   No Reserves, No Items, Wumpa Drain, Clock Curse and Explosive Crates selected; check steering,
   jumps, speed/acceleration, reserve/item suppression, fruit drain cadence,
   sustained clock slowdown and the activation banner in 1P/2P.
2. Baseline/ordered modifier application, remaining effects and contact
   triggers, then External.
3. `docs/features/CHAOS.md` + README links: creation, scope, stacking, effects,
   pause inspection and provider/replay limitations.

Existing suites: `test_chaos_effect`, `test_chaos_gate`, `test_chaos_new_file`,
`test_chaos_setup`, `test_menu_strings`, `test_chaos_save`, `test_chaos_runtime`.
Runtime tests cover timers, ordering, overflow, cadence, gates, resident restore,
direct driver application, retained drifts and banner submissions. Effects apply
after retail hop-input resolution, before jump/steering consumption, with no
stored input copy.
Each activation is announced for three gameplay seconds, then docks to the right
at 50% of the small font. Separate borderless tables keep the newest banner at
the center baseline; older banners stack down in 1P and up in 2P, avoiding Final
Lap rows. Docked rows remain fixed behind lap statistics. The list shows only the
four latest distinct effects in both 1P and 2P, with a higher 2P position above
the lap counter. All messages disappear when the race ends. Pause freezes the
animation and hides the overlay.
Add modifier-restoration tests with the effects. Build with `cmake --build build -j`;
run focused CTest.

In-game checks: all Adventure race types, untouched bots, both 2P humans,
pause/retry/results/hub transitions, sustained pad contact, rescue destination,
takeover hand-back and mid-effect checkpoints. Binary save tests do not cover
full disk IO; resident snapshot tests do not cover full checkpoint serialization.
Pause viewer visual verification remains outstanding. Preserve the immediate
Effects → Settings redraw. The Confirm → character-select flash investigation
was intentionally closed; do not reintroduce its removed probes/workarounds.

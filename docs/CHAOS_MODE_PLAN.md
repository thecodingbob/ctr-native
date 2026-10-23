# Chaos Mode — Remaining Implementation

## Status

Implemented: catalogue/selection helpers, race gate, creation flow, per-slot
persistence, save marker, pause inspection and runtime timers/Interval/Random/Pickup/External triggers.
Inverted Steering, No Hopping, Wobble Steering, Low Gravity, Heavy Karts,
Speed Capped, Super Speed, No Reserves, No Items, Wumpa Drain, Clock Curse,
Explosive Crates, Defective Boosts, Anxious Mask, Hard Crates and Giant Karts are
connected, with an activation-name overlay.
The External file provider publishes the enabled-effect catalogue for voting apps.
Pending: player guide, external voting application and in-game final validation.
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
- Pickup means direct collection of crates, fruit, crystals and CTR letters, or
entry onto a normal/super turbo pad. Hard Crates must be consumed to count.
Each active effect has a unique entry; reactivation
replaces it and resets all timers. Conflicting activations deactivate the previous effect.

## Runtime core

Owner: `game/231/RB_Chaos.c`, registered in `game/game_unity.h`, with declarations
in `include/functions.h`. Host IO stays in `platform/`.

The implemented core stores ordered activations and cadence in
`sdata->chaosRuntime`. Race initialization follows vehicle teleport; updates run
before driver stages and check eligibility, `armed`, menus and pending loads.
Pause freezes game-time timers. Expiry precedes scheduling; new entries receive
full duration and cadence retains overshoot. The four-entry queue holds unique
effects; reactivation replaces the old entry even at capacity. A fifth unrelated
effect is rejected. Scheduled events consume their gap and effect-choice RNG,
including rejected requests.
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
| Pickup | One activation per eligible human collection or continuous turbo-pad traversal |
| External | Provider commands only |

None waits for earlier effects to expire. An empty selection yields no activation
and does not change `armed`.

Pickup hooks: `RB_CrateAny_Collide` dispatches after a full-size crate becomes
hidden on direct kart contact; `RB_Fruit_ThCollide`, `RB_Crystal_ThCollide` and
`RB_CtrLetter_ThCollide` dispatch after collection. Hidden collectibles and
intact Hard Crates cannot retrigger; regrown crates can. Weapon-owned rewards
do not count. `Chaos_OnPickup` gates human scope, race state, trigger mode and
configured duration before selecting an enabled effect.
`VehPhysForce_CollideDrivers` reports normal/super turbo-pad contact after the
retail boost. A per-human latch in captured Chaos runtime prevents repeated
activations until leaving all turbo-pad terrain; pause freezes the latch and
race reset clears it. Pending Wumpa/crystal HUD awards are not trigger sites.

## Effect hooks

All 16 effects have catalogue IDs/toggles and implementations.

| Effects | Candidate mechanism |
| --- | --- |
| Inverted Steering — done | Mirror a local axis about the active controller center; D-pad already feeds stick resolution; reactivation resets the lifetime |
| No Hopping — done | Clear resolved manual hop latch/buffer; preserve drift input/states, forced jumps and rain-cloud ownership |
| Wobble Steering — done | Two-second sine wave at 75% of the controller range, clamped around its center; deactivates Inverted Steering and vice versa |
| Low Gravity / Heavy Karts — done | Derive effective gravity from the live constant at 1/4 or 3×, before terrain rules; deactivate each other |
| Speed Capped / Super Speed — done | Cap the forward target at half the live class speed / double resolved forward target and acceleration; retain retail hard limit and deactivate each other |
| No Reserves — done | Clear both fuel timers only in the existing driver hook; retail boost grants persist until the next application; allow new fuel after expiry |
| No Items — done | Empty inventory and cancel pending launches/roulette through one pre-item hook; keep already deployed effects and discard fresh rewards at the next check |
| Explosive Crates — done | Direct kart contact with item, Wumpa or time crates triggers an explosion and blast damage, bypassing protection; retain normal rewards |
| Wumpa Drain — done | Detect crossed seconds from elapsed time in the existing driver hook; reactivation resets the pulse clock, stop at zero and preserve pending pickup awards |
| Clock Curse — done | One-off spin hit, clock audio and full 32-bit received-clock timer, bypassing protection; new activations overwrite earlier clocks |
| Defective Boosts — done | Invert the live fire-speed bonus while reserves remain; preserve normal fuel grants, accumulation and consumption for every boost source |
| Anxious Mask — done | Contact with slowing terrain requests retail rescue before checkpoint updates; existing rescue states prevent repeated requests |
| Hard Crates — done | Full-size item, Wumpa and time crates stay solid on direct human contact; override touch-only hitbox response with the retail solid impact without breaking the crate |
| Giant Karts — done | Double human kart models, tires and lateral collision radii; keep retail floor clearance, raise speed and jump force by 30%, squash smaller karts and hop past them |

Key implementation constraints:

- `const_TerminalVelocity` caps vertical velocity, not forward speed. Speed effects
  derive local targets/acceleration without overwriting constants. The existing
  `0x6400` target clamp limits Super Speed; slopes/collisions remain retail.
- Anxious Mask uses the physical contact terrain, leaving
  rescue eligibility to the retail consumer. Stock rescue returns to
  the last valid checkpoint, changes camera/input, wipes turbo and can cost wumpa.
- Explosive Crates reuses the retail damage implementation with protection bypass;
  weapon hits retain ordinary crate behavior. Hard Crates leaves crates intact
  and returns through the regular solid-hitbox impact path.
- Hard Crates can consume retail RNG when weapon hits break crates; its replay
  incompatibility is accepted. Do not hide it by restoring retail RNG.

Deferred: Hyper Bots, Blind Kart, Tiny Karts, Item Rain.
Append stable IDs/slugs; the current disk bitset supports 128 effects.

## External provider

### Agreed file-provider contract (2026-10-23)

- One fixed, extensionless text file: `<base>/chaos/command`. Its content is one
  stable effect slug, e.g. `low-gravity`, with optional surrounding whitespace.
  One file contains one command; there is no inbox queue, outbox, `status`,
  `reset`, filename-encoded command, or per-command duration override.
- At game initialization and in `Chaos_BeginRace()`, publish the effects available in the active
  save under `<base>/chaos/`, so an external application can discover the voting
  pool without duplicating the catalogue or guessing the saved selection.
  Available means implemented and enabled in that save, not currently running
  or immediately activatable. The next race initialization publishes its loaded
  selection. Do not rebuild the catalogue in frame updates or add profile/menu
  invalidation hooks. Publication failures are logged, without frame retries.
  Publish atomically via a temporary file and rename, including an empty list
  when no effects are enabled. The extensionless filename is `effects`, with
  one stable slug per line. Initializing with Chaos unarmed publishes an empty list; this is a
  catalogue, not a live readiness signal or proof that the process is running.
- The External trigger uses the saved `triggerIntervalSeconds` as a minimum gap
  between successful activations, not as a polling period. Wait one full interval
  from GO before attempting the first command. Each successful activation restarts
  this timer. Once it expires, periodically check for the command until one arrives.
- Count only eligible, active race time. Pause, menus, countdown, loading, hubs,
  and post-race states neither advance the timer nor consume the command.
  Each new race starts a fresh wait from GO. A pending file survives transitions
  and is valid for the next eligible race; do not delete it during race reset.
- The external producer waits until the command file is absent, writes and closes
  `command.tmp`, then atomically renames it to `command`. The game ignores the
  temporary file. Only one producer owns the slot; do not overwrite a pending file.
- When eligible and the interval has expired, read and delete the command file,
  then activate through the existing engine path. Deletion signals that the slot
  is available for another command, which can wait there during the next interval.
  If the file cannot be read or deleted, do not activate it; retain it for retry.
- Consume malformed, unknown, or disabled-effect requests too, logging the reason
  in the normal application log. Rejected commands do not restart the activation
  interval. Deletion acknowledges consumption, not successful execution.
- Respect the saved armed state, External trigger, effect selection, race gates,
  and saved effect duration. Never change creation settings. Duration and interval
  are independent; effects can overlap, and reactivation follows existing unique-ID
  refresh/replacement rules.
- Keep filesystem work in `platform/native_chaos_ipc.c` and game eligibility,
  timing, validation and activation in the game layer. Poll on the main thread
  after the Chaos timer update and before gameplay threads, so a new activation
  receives its full duration. Use a bounded command read and a modest polling
  cadence (100 ms once the interval expires).
- Disable the watcher during replay playback, preserving any pending file.
  External command timing is not recorded; provider-driven replay verification
  is not guaranteed.

Implemented in the single-file provider. The superseded multi-file batch/reply
prototype has been replaced. Build and all 23 CTest suites pass. Runtime tests
cover interval/gate/selection semantics and a filesystem suite exercises command
consumption, failed deletion, playback isolation and catalogue replacement.
See [CHAOS_EXTERNAL.md](CHAOS_EXTERNAL.md) for the producer contract and example.

Checks: initial wait from GO; gameplay-only timer; no-file polling after expiry;
one activation per interval; pending commands across pause/next race; atomic
publication; invalid/disabled requests consumed and logged without a cooldown;
read/delete failure without activation; saved duration/selection preserved;
overlap/reactivation; replay playback leaving the file untouched.
Also check initial catalogue publication, saved-selection filtering, empty lists,
atomic reads and replacement when switching Chaos saves.

### Intended external voting application

The intended producer presents three randomly selected available effects to
viewers, collects votes until its voting timer expires, then publishes the
winning effect through the single command slot. Voting, UI/stream overlay and
viewer integration belong to the external application, not the game.

The game still owns the gameplay-time activation interval; expiration of the
external voting timer does not bypass it. Exact alignment with pauses/countdown
would need a separate readiness/timing signal, which the minimal protocol does
not currently provide. The catalogue alone cannot synchronize those timers.
Voting policy still to decide: fewer than three enabled effects, ties/no votes,
repeat candidates between rounds, vote changes/one-vote-per-viewer, and when to
start the next round relative to command consumption. These are external-app
decisions, not additional game-side protocol requirements yet.

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

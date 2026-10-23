# Chaos implementation contracts

## Source map

| Responsibility                                | Owner                                                                            |
|-----------------------------------------------|----------------------------------------------------------------------------------|
| Stable effect IDs, slugs, scope and selection | `include/chaos_effect.h`                                                         |
| Save encoding and runtime layouts             | `include/chaos_save.h`, `include/namespace_Memcard.h`, `include/chaos_runtime.h` |
| Eligibility and runtime                       | `include/chaos_gate.h`, `game/231/RB_Chaos.c`                                    |
| Creation flow and shared settings screen      | `game/230/MM_NewLoad.c`, `game/230/MM_ChaosConfig.c`                             |
| Pause entry and HUD                           | `game/MAIN/MainFreeze_Chaos.c`, `game/UI/UI_Chaos.c`                             |
| External filesystem interface                 | `platform/native_chaos_ipc.c`                                                    |

Remaining work: [CHAOS_MODE_PLAN.md](CHAOS_MODE_PLAN.md).
External protocol: [CHAOS_EXTERNAL.md](CHAOS_EXTERNAL.md).

## Adventure saves

SLOTS uses native 32-bit little-endian layout. The retail profile and advertised
size remain `0x1600`; the co-op extension is unchanged.

| Payload offset | Contents                                     |
|----------------|----------------------------------------------|
| `0x1600`       | Four 4-byte `AdvMultiplayerSettings` entries |
| `0x1610`       | Four 24-byte `ChaosSettingsSaved` entries    |
| `0x1670`       | 14 unused bytes                              |
| `0x167e`       | Two-byte checksum                            |

Each V1 Chaos entry contains:

| Offset | Contents                                 |
|--------|------------------------------------------|
| `0x00` | `u16 durationSeconds`                    |
| `0x02` | `u16 triggerIntervalSeconds`             |
| `0x04` | `u8 trigger` (`ChaosTrigger`)            |
| `0x05` | `u8 armed`                               |
| `0x06` | 16-byte enabled-effect bitset            |
| `0x16` | `u16 format = 0x0143` (`'C'`, version 1) |

Effect ID `n` maps to bit `n % 8` of byte `n / 8`, least-significant bit first.
Append IDs; never renumber or reuse them. Slugs are stable external identifiers.
The live settings use `bool effects[CHAOS_EFFECT_COUNT]` and derived
`enabledCount`; encoding zeroes unused bits. Unknown bits are ignored on decode
and cleared on rewrite, so new effects start disabled in existing saves.

`armed` distinguishes Chaos from Regular even with an empty selection. Missing
or unsupported format tags decode to unarmed, empty settings. Normal save/load
preserves the selected slot's settings; deleting a slot zeroes them. Only V1 is
supported. Field/trigger encoding changes or growth beyond 128 effects require a
new format and migration; `namespace_Memcard.h` assertions pin the layout.

## Creation and inspection

- Every New starts Regular with all implemented effects selected. `chaos_mode`
  controls only the creation prompt; loaded files use `chaosSettings.armed`.
- Flow: Mode → optional Players → Chaos setup if armed → character selection.
  Back retains choices. Settings are immutable once the file is created.
- Confirm requires a selected effect; otherwise it is gray and plays the locked
  sound. This does not reinterpret an existing armed, empty file as Regular.
- Interval is shown for Random, Interval and External, hidden for Pickup.
- Armed Adventure files add **Chaos Rules** to hub/race/gem-cup pause menus.
  The shared setup screen is read-only, without Confirm. **Active Effects** lists
  enabled effects, not currently running activations, without ON/OFF controls.
- Cross/Circle activates actions; Left/Right edits creation values. Triangle
  returns one page; Start/Square exits the read-only view to pause.
- Custom menu strings live in `RECTMENU_Text.c`; `RECTMENU_GetString` handles
  retail LNG indices and custom IDs starting at `0x4000` for drawing and width.

## Runtime and checkpoints

`Chaos_BeginRace` runs after vehicle teleport in `MainGameStart_Initialize`.
`Chaos_Update` runs before gameplay threads, only after GO in eligible Adventure
races, with no active menu, debug menu or pending load. Pause freezes timers.
Race end, retry teardown and level-load stage 0 clear runtime before driver reuse;
settings and the dedicated `chaosRng` survive. Use that RNG for scheduling and
selection rather than retail RNG. Hard Crates' retail weapon interactions can
still consume retail RNG; do not conceal this by restoring it.

Activations are ordered, with at most four unique IDs. Reactivation removes the
old entry and appends a fresh one, resetting duration, phase, notice and one-off
behavior. Conflicting pairs replace each other: Inverted/Wobble Steering,
Low Gravity/Heavy Karts, Speed Capped/Super Speed. A fifth unrelated effect is
rejected; replacement remains possible at capacity. Expiry precedes scheduling,
so new entries receive their full duration.

| Trigger  | Scheduling                                                            |
|----------|-----------------------------------------------------------------------|
| Interval | One enabled effect per configured gap                                 |
| Random   | Each gap, including the first, is 30–100% of the configured interval  |
| Pickup   | One enabled effect per eligible direct collection or turbo-pad entry  |
| External | One file request after the minimum active-race interval; see protocol |

Interval/Random retain frame overshoot; rejected scheduled events still consume
their gap and selection RNG. External restarts a full gap only after success.
No trigger waits for earlier effects to expire.

Pickup hooks run after full-size crates, loose Wumpa, crystals or CTR letters are
consumed. Weapon-owned rewards, hidden/regrowing crates, intact Hard Crates, bots
and finished humans do not count. Normal/super turbo pads dispatch after the
retail boost: one captured latch per human prevents repeats until leaving all
pad terrain. Pause freezes latches; race reset clears them.

SDATA checkpoints capture settings, RNG, runtime, creation flow/menu navigation
and pause rows; D230 captures the ordinary chain menus. The checkpoint relocator
repairs standalone menu, pause-return and chain pointers. Immutable catalogue,
titles and row specifications need no snapshot. Do not re-enter setup after
restore: that resets restored navigation. Changed resident layouts reject old
checkpoints; this is separate from Adventure-save compatibility.

Host command/catalogue files are outside checkpoints. Catalogue publication runs
at game initialization and `Chaos_BeginRace`, not per frame. Playback performs no
provider file I/O; external request timing is not recorded.

## Effect integration

Effects target unfinished humans, not bots or hubs. Apply local modifiers in
activation order without overwriting shared input or snapshotting another
effect's modified constants. Persistent ownership/baselines, if introduced, must
be captured and released before driver destruction; overlay changes alone do not
guarantee teardown.

| Effect                     | Hook and constraints                                                                                                                                                                    |
|----------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Inverted Steering          | Mirror the resolved axis around the calibrated center.                                                                                                                                  |
| No Hopping                 | Clear resolved manual hop latch/buffer after input resolution; preserve drift, forced jumps and cloud ownership.                                                                        |
| Wobble Steering            | Two-second sine wave at 75% of controller range, clamped; phase uses captured activation elapsed time.                                                                                  |
| Low Gravity / Heavy Karts  | Derive gravity from the live constant at 1/4 or 3× before terrain multipliers; preserve jump impulse and terminal velocity.                                                             |
| Speed Capped / Super Speed | Half-class-speed ceiling / double forward target and positive acceleration; preserve reverse and the retail `0x6400` target limit.                                                      |
| No Reserves                | Clear both fuel timers in the driver-input hook; later retail grants survive until the next application.                                                                                |
| No Items                   | Clear inventory, pending launch and roulette before item processing; deployed effects remain active.                                                                                    |
| Wumpa Drain                | Remove one credited fruit per crossed game second, at most once per frame; preserve pending HUD awards and stop at zero. No pulse at activation or expiry.                              |
| Clock Curse                | One-off spin and clock audio bypass protection without consuming it; assign full duration to 32-bit `clockReceive`. Retail countdown owns it thereafter, even after a Chaos-only reset. |
| Defective Boosts           | Subtract twice the live fire-speed bonus while reserves remain, clamping the forward target to zero; retain ordinary fuel/fire bookkeeping.                                             |
| Anxious Mask               | Physical slowing terrain requests retail rescue before checkpoint progress; retail eligibility, destination and penalties remain authoritative.                                         |
| Explosive Crates           | Direct human contact with a consumed item/Wumpa/time crate creates the retail burst and protection-bypassing damage; retain rewards and weapon-hit behavior.                            |
| Hard Crates                | Full-size item/Wumpa/time crates return the solid-hitbox response before breaking or granting rewards; retain weapon/bot/regrowth behavior.                                             |
| Giant Karts                | Target `Driver.kartScale = 2`; double models, tires and lateral collision radii, retain ground clearance, add 30% forward speed/jump force. Restore target 1 on deactivation/reset.     |

Scale calculations belong in `VehPhysForce.c`; Chaos sets driver targets. Retail
interpolation owns body growth/squash/stretch. Giant-vs-smaller collisions use
retail squish damage with protection, skip normal bounce after a successful
squash and queue the forced hop; equal scales keep ordinary collision.

## HUD and audio

Each successful activation plays the preloaded double-gain transition swish and
starts a three-second announcement followed by 320 ms docking. Rejected requests
and redraws do neither. Race audio teardown releases level-local cached samples;
preloading avoids synchronous reads at activation.

The newest banner starts at the standard race-message baseline. Older banners
stack down in 1P and up in 2P, avoiding Final Lap. A separate right-hand table
shows at most four docked effects at 50% small-font scale, newest first. Docked
rows use UI OT slot 1 behind lap statistics; Final Lap is submitted after the HUD.
Pause freezes/hides announcements, menus hide them and all-human finish hides
them before results setup. Reset clears them.

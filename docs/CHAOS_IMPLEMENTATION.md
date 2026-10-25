# Chaos implementation contracts

## Source map

| Responsibility                                | Owner                                                                            |
|-----------------------------------------------|----------------------------------------------------------------------------------|
| Stable effect IDs, slugs, scope and selection | `include/chaos_effect.h`                                                         |
| Save encoding and runtime layouts             | `include/chaos_save.h`, `include/namespace_Memcard.h`, `include/chaos_runtime.h` |
| Eligibility and runtime                       | `include/chaos_gate.h`, `game/231/RB_Chaos.c`                                    |
| Effect implementations                        | `game/231/RB_ChaosEffects.c`                                                      |
| Creation flow and shared settings screen      | `game/230/MM_NewLoad.c`, `game/230/MM_ChaosConfig.c`                             |
| Pause entry and HUD                           | `game/MAIN/MainFreeze_Chaos.c`, `game/UI/UI_Chaos.c`                             |
| External filesystem interface                 | `platform/native_chaos_ipc.c`                                                    |

Player guide: [Chaos mode](features/CHAOS.md).
External protocol: [Chaos external file provider](features/CHAOS_EXTERNAL_PROVIDER.md).

Asset-free regression suites mirror these responsibilities:
`tests/test_chaos_runtime.c` covers scheduling, activation, triggers, IPC and HUD;
`tests/test_chaos_effects.c` covers gameplay modifiers and instant rewards.
Both use `tests/shared/chaos_fixture.h` for the shared world, real module include
chain and integration doubles. `tests/test_warpball.c` independently covers orb
launch and teardown without loading the Chaos runtime.

`RB_Chaos.c` owns activation, public gameplay hooks and per-driver dispatch,
sharing `Chaos_TargetsDriver` for recipient checks. `RB_ChaosEffects.c` contains
only private effect operations: callers check activation and applicability.
Effect operations retain their gameplay conditions (terrain, fuel, pickup
queues) but never query the Chaos runtime. The gameplay hooks preserve modifier
ordering and check eligibility before calling them; No Reserves is tested
through `Chaos_ApplyDriver`.

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

## Profile and menu integration

- Every New resets the live settings to Regular with all implemented effects
  selected. `chaos_mode` controls only the creation prompt; loaded files use
  `chaosSettings.armed`. Settings are immutable once the file is created.
- Creation retains choices when navigating Back, including across the character
  selection overlay. Confirm requires a selected effect; this does not reinterpret
  an existing armed, empty file as Regular.
- Armed Adventure files add **Chaos Rules** to hub/race/gem-cup pause menus.
  The shared setup screen is read-only, without Confirm. **Active Effects** lists
  enabled effects, not currently running activations, without ON/OFF controls.
- Custom menu strings live in `RECTMENU_Text.c`; `RECTMENU_GetString` handles
  retail LNG indices and custom IDs starting at `0x4000` for drawing and width.

## Runtime and checkpoints

`Chaos_BeginRace` runs after vehicle teleport in `MainGameStart_Initialize`.
`Chaos_Update` runs before gameplay threads, only after GO in eligible Adventure
races, with no active menu, debug menu or pending load. Pause freezes timers.
Race end, retry teardown and level-load stage 0 clear runtime before driver reuse;
settings and the dedicated `chaosRng` survive. `Chaos_Init(seed)` initializes the
generator once at process startup; the host supplies a seed from SDL's
automatically seeded RNG, before replay restoration;
Adventure saves do not restore it. Checkpoints restore its exact captured state.
Use that RNG for scheduling and
selection rather than retail RNG. Hard Crates' retail weapon interactions can
still consume retail RNG; do not conceal this by restoring it.

Timed activations are ordered, with at most four unique IDs. Reactivation removes the
old entry and appends a fresh one, resetting duration, phase, notice and one-off
behavior. Conflicting pairs replace each other: Inverted/Wobble Steering,
Low Gravity/Heavy Karts, Speed Capped/Super Speed, No Reserves/Too Boosted TO Fail.
Use This! removes No Items; Reserve Injection removes No Reserves before its grant.
Symmetric conflicts are listed once as pairs; the instant gifts' one-way
replacements remain explicit.
A fifth unrelated timed effect is
rejected; replacement remains possible at capacity. Expiry precedes scheduling,
so new entries receive their full duration.

Instant rewards execute once and use a separate four-entry announcement queue,
without entering `active` or consuming timed capacity. Repeats execute again and
refresh their notice; overflow drops the oldest notice. Zero duration is invalid
for both kinds. Selection draws directly from enabled effects with `chaosRng`.
Current scopes need no recipient-availability filter: the race ends when all
humans finish. Application still excludes finished drivers and respects scope.

Trigger behavior is described in the [player guide](features/CHAOS.md#triggers).
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
Published commands are enabled effects applicable to the current eligible race;
hubs publish an empty list. Requests are checked again at execution time.

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

`Chaos_GiveRandomItem` owns inventory replacement, uniform selection from an
explicit list of distinct Race2 items and mask choice. It passes the resolved
item ID to `VehPhysProc_AssignHeldItem`, extracted from ordinary roulette
finalization to share orb ownership, missile limits and stack quantities.
`Chaos_ClearItems` cancels roulette, cooldown and pending fire input after calling
`VehPhysProc_RemoveHeldItem`. That shared item operation handles removal and
missile/orb ownership, and is also used when ordinary item use completes.
The reusable operations live in `VehPhysProc_Items.c`, included by `VehPhysProc.c`.
Ordinary roulette selection and finalization remain intact.
Use This! is available in relic/crystal events too.
Relic HUDs display granted inventory alongside their time-crate feedback.
Direct mask/shield grants use normal activation without
consuming inventory; renewal reuses the existing mask model or shield layers.
The held item's mask choice is preserved.
The gift and ordinary roulette share `VehPhysProc_PlayItemAcquireSound`, using
the original `ITEMSET_ACQUIRED_*` sound IDs in `VehPhysGeneral.c`. Direct mask grants use
`VehPickupItem_MaskUseWeapon` with no inventory mask choice, allowing
normal renewal and configured mask selection. That power-up helper retains its
normal RNG; uniform inventory selection uses `chaosRng`.
Juiced Up tops up the ordinary pending Wumpa pickup queue;
HUD crediting animates the count and `RB_Player_ModifyWumpa` plays the full-count
chime. Existing pending fruit is counted toward the reward. Crystal challenges
credit Wumpas directly through that helper, keeping their crystal queue intact.

The warpball branch in `VehPickupItem_ShootNow` prepares its routing inline.
Race tracks use their checkpoint path;
crystal/battle arenas and levels without that path immediately start the normal
orb teardown after spawning, without seeking targets or running movement.
Teardown tolerates missing particles.

Reserve Injection brings reserves to at least 9,600 units (ten game seconds),
writing the reward directly, preserving larger balances and using `VehFire_Increment`
at third-drift-boost fire level if no boost remains. Reserve losses share
`VehFire_SetReserves`, whose single Chaos hook protects depletion, damage,
U-turn and rescue resets. `COLL_MOVED_ScrubImpact` skips clearing reserves and
`turbo_outsideTimer` for drivers protected by Too Boosted To Fail.
`VehFire_Increment` applies the original reserve grants directly, without
checking Chaos protection.

Sabotage (ID 22, `sabotage`) filters driver-local resolved brake input through
`Chaos_ApplyDriver` before braking can clear reserves or affect target speed.
It disables Square and the positive/reverse halves of both vertical sticks,
preserving forward throttle and shared gamepad input. Recipient/activation checks
remain in `RB_Chaos.c`; `RB_ChaosEffects.c` owns the input operation.

This Is Fire (ID 23, `this-is-fire`) upgrades existing turbo at activation and
sets new boost grants to `VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL`, using the existing super turbo pad constant.
`VehFire_SetLevel` shares normal fire speed/size assignment between activation
and `VehFire_Increment`; activation does not add reserves or count a boost.
`Chaos_FireLevelForDriver` filters new grants before their normal bookkeeping.
The effect replaces Defective Boosts symmetrically. Expiry does not restore a
previous fire tier; earned USF remains subject to normal boost-loss rules.

## HUD and audio

Each successful activation plays the preloaded double-gain transition swish and
starts a three-second announcement; timed effects then get 320 ms docking.
Instant notices expire without docking. Rejected requests
and redraws do neither. Race audio teardown releases level-local cached samples;
preloading avoids synchronous reads at activation.

The newest banner starts at the standard race-message baseline. Older banners
stack down in 1P and up in 2P, avoiding Final Lap. A separate right-hand table
shows at most four docked effects at 50% small-font scale, newest first. Docked
rows use UI OT slot 1 behind lap statistics; Final Lap is submitted after the HUD.
Pause freezes/hides announcements, menus hide them and all-human finish hides
them before results setup. Reset clears them.

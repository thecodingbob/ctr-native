# Chaos implementation contracts

## Source map

| Responsibility                                | Owner                                                                            |
|-----------------------------------------------|----------------------------------------------------------------------------------|
| Stable effect IDs, slugs, scope and selection | `include/chaos_effect.h`                                                         |
| Save encoding and runtime layouts             | `include/chaos_save.h`, `include/namespace_Memcard.h`, `include/chaos_runtime.h` |
| Eligibility and runtime                       | `include/chaos_gate.h`, `game/231/RB_Chaos.c`                                    |
| Effect implementations                        | `game/231/RB_ChaosEffects.c`                                                      |
| Shared driving-class constant loader          | `game/Vehicle/VehBirth_Consts.c` (included by `VehBirth.c`)                       |
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
queues) but never query active-effect eligibility. The gameplay hooks preserve modifier
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

Instant effects execute once and use a separate four-entry announcement queue,
without entering `active` or consuming timed capacity. Repeats execute again and
refresh their notice; overflow drops the oldest notice. Zero duration is invalid
for both kinds. Selection draws from enabled effects allowed in the current
event with `chaosRng`. Bots-only effects are excluded from relic, crystal,
time-trial and battle events; this rule also applies to publication and incoming
requests. There is no live recipient-availability filter. Application excludes
finished drivers and respects scope; bots-only scope selects slots above the
human count. Event filtering reuses `RACE_FIELD_NO_BOT_MODES` from race setup.

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

Effects target unfinished humans or bots according to their catalogue scope;
hubs are excluded. Apply local modifiers in
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
`Chaos_ApplyInputRules` before braking can clear reserves or affect target speed.
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

Hydraulics (ID 24, `hydraulics`) renews a normal hop request in
`Chaos_ApplyDriver` by filling an empty `jump_TenBuffer` for the current physics
step. It preserves genuine input buffers and never changes shared pad input or
simulates a held shoulder. Normal jump/friction checks own eligibility and force,
so there is no extra delay, airborne impulse or cooldown bypass. Requests also
apply while drifting; damage states and jump-lockout clouds do not receive them.
Hydraulics and No Hopping replace each other.

My Planet Needs Me (ID 25, `my-planet-needs-me`) adds one world-up impulse at
six times the effective normal jump force, including while airborne. It uses
the existing speed/vector converters and synchronizes the working velocity,
preserving horizontal momentum, inventory and reserves. It bypasses normal-hop
eligibility without changing cooldowns or queuing a repeated jump. Activation
removes No Hopping/Hydraulics, preserves gravity modifiers and uses only an instant
announcement. Focused tests capture the launch vector at the conversion boundary;
full trajectory validation is in game.

Who Bound This? (ID 26, `who-bound-this`) keeps independent per-human permutations
and analog edge history in `ChaosRuntime`. `Chaos_ApplyInputRules` remaps fresh
local pad copies before applying Inverted/Wobble Steering and Sabotage. Driving
physics, drift entry and drift updates all use the same logical controls;
shared pad/menu input remains unchanged. Analog throttle/reverse become mapped
button actions while the effect is active. No Hopping/Hydraulics stay in
`Chaos_ApplyDriver`, after normal jump-input resolution.

Try Something New (ID 27, `try-something-new`) is a one-off class change lasting
until race end. The runtime remembers the selected standard class so repeats
exclude the live class rather than always excluding the character baseline.
`VehBirth_SetConstsForClass` shares the ordinary metaphysics table loader and
configured multipliers; `VehBirth_SetConsts` still selects the character's class
for normal birth. Both live in `VehBirth_Consts.c`, allowing focused tests to use
the real loader without spawning vehicles. Changing class leaves characters,
shared metadata and current race state intact. Race teardown clears the selected
class bookkeeping; normal vehicle birth restores the character's class.
Class changes also stop the previous engine loop and start the selected class's
loop. `Chaos_EngineClassForDriver` keeps engine updates, audio initialization and
warp teardown on that same class, without overriding ghosts that share a player
ID. Reset restores the character's engine loop before clearing the override.

Small Kart Energy (ID 28, `small-kart-energy`) uses the same size pipeline as
Giant Karts. `Driver.kartScale` is a float multiplier (zero defaults to 1×),
resolved by `VehPhysForce_GetKartScale`; finished drivers use 1×. Visual scales,
tires and contact radii convert to integer dimensions through
`VehPhysForce_ScaleValue`. Squash compares the floating-point multipliers without
integer truncation and reuses normal damage protections. Small size is 0.5×;
forward speed targets and jump force are 0.7×. Size effects replace one another,
and expiry/reset restore normal size, including for finished humans.
Ground shadows scale their local footprint through the same helper, preserving
the normal height-based shrink/fade. Skid emission scales all four tire contact
positions, strip widths and lateral offsets before storing world-space vertices;
existing skid history keeps the size at which it was emitted. Both paths follow
`kartScale` directly, rather than checking individual Chaos effects.
Kart particles use `VehPhysForce_ScaleParticle` at emission, before adding world
positions. It scales spatial axes, growth and spatial oscillators together while
preserving angles, color and timing. Exhaust, burn smoke, terrain/wall/ground
sparks, mud and rescue/water particles share this path. `VehPhysForce_ScaleMatrix`
scales freshly rebuilt effect geometry for turbo flames, shields and masks;
attachment offsets are scaled separately from world positions. Water wake size,
revving animation and wall-crash scale resets also follow the kart multiplier.
TNT remains its original size: only its attachment offset follows the kart, so
it rests on the resized character's head without changing its animation/state.

Classic GTA (ID 29, `classic-gta`) overrides the final pose in `CAM_ThTick` after
normal camera processing, preserving the live mode and zoom state. Each human
viewport looks vertically down from `CHAOS_TOP_DOWN_HEIGHT` (2,560 world units), following kart yaw.
The driver's ground/contact block supplies visibility; only missing ground data
requires a driver-position camera probe. `Chaos_TopDownDriver` gates presentation
independently of simulation pause, so pause menus retain the camera/cutaway.
`Chaos_ApplyLevelVisibilityRules` runs after normal PVS processing and enables all
leaf/face mask bits for that viewport, leaving geometry selection to the overhead
frustum instead of the ground-level PVS. It invalidates the decoded-source caches
so expiry reloads retail masks even when the driver stays on the same block.
The four shared level BSP-list workers filter ceiling blocks before emission,
consuming visibility bits normally. Classification uses the high-detail triangle
topology and normal winding from collision, with 64-bit X/Z cross products.
Blocks with downward-facing triangles and no upward-facing triangles are omitted;
any road-facing triangle preserves the whole block. Vertical panels and the
driver's supporting/contact blocks remain. Triangular blocks use only their four
valid high-detail triangles. This avoids tile-boundary and driver-height toggles
and preserves unflagged ramps and mixed floor/ceiling geometry. Static level
models whose projected bounds cover the kart are culled before OT allocation if
they extend above the cutaway plane. Threaded gameplay objects are preserved.
These are per-viewport whole-object cutaways; they do not change collision data
or shared flags, and do not require checkpoint-owned camera state.

Arena Champions (ID 30, `arena-champions`) uses bots-only scope. Normal
rank/distance/leader-stat rubber-banding, boost, terrain and damage rules still
run. `Chaos_SpeedForDriver` reuses the same 2x operation as Super Speed for
the final navigation speed and horizontal velocity. AI dynamics stay in retail
units: doubling the output doubles actual acceleration too, keeps existing caps
and airborne square calculations valid, and avoids accumulating multipliers.
The 16-bit speed/reporting mirrors saturate rather than wrapping; navigation and
32-bit horizontal velocity keep the full speed. Vertical jump/gravity and impact
offsets retain normal behavior. There is no saved baseline or teardown mutation.
Event filtering is static and never inspects whether any bot is still racing.

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

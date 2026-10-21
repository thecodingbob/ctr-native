# Chaos state contracts

## Adventure saves

`MemcardData` is copied into the SLOTS payload with its native 32-bit,
little-endian layout. Its retail profile is still `0x1600` bytes and its
header continues to advertise that retail size. Each Chaos slot carries its own
format tag; the co-op extension keeps its existing layout:

| Payload offset | Contents |
| --- | --- |
| `0x1600` | Four `AdvMultiplayerSettings` entries, 4 bytes each |
| `0x1610` | Four `ChaosSettingsSaved` entries, 24 bytes each |
| `0x1670` | Remaining unused payload bytes |
| `0x167e` | Two-byte checksum |

Within each V1 Chaos entry:

| Offset | Contents |
| --- | --- |
| `0x00` | `u16 durationSeconds` |
| `0x02` | `u16 triggerIntervalSeconds` |
| `0x04` | `u8 trigger`, using the current `ChaosTrigger` values |
| `0x05` | `u8 armed` |
| `0x06` | 16-byte bitset, with capacity for 128 effects |
| `0x16` | `u16 format`, currently `0x0143` (`'C'`, version 1) |

Effect ID `n` maps to bit `n % 8` of byte `n / 8`, least-significant bit first.
IDs are explicit and stable: append new IDs, never renumber or reuse an existing
one. Designated catalogue entries bind names to those IDs independently of their
source order. Existing code presents the dense ID sequence in the menu.

The live `ChaosSettings` keeps `bool effects[CHAOS_EFFECT_COUNT]` and the derived
`enabledCount`. `chaos_save.h` translates between that growable runtime shape and
the fixed disk shape. Every encode starts with a zeroed entry and sets only
implemented, selected effect bits. Unassigned bits are zero, so later effects
start disabled in existing files. New files still select all implemented effects.
Unknown bits do not contribute to the decoded count and are cleared on rewrite.

`armed` distinguishes Regular from Chaos even when every effect is disabled.
The menu changes only the live settings during creation. Normal Adventure
save/load encodes/decodes those settings for the selected slot; clearing a slot
zeroes its settings. Loading never consults the creation-prompt option.
Files without Chaos settings have a zeroed extension and are Regular. Only the
V1 tag is decoded; missing or unsupported tags produce unarmed, empty live
settings. There is no migration from unreleased development formats.

The assertions in `namespace_Memcard.h` pin disk offsets, stride and capacity.
Adding effects up to ID 127 does not change the save layout or version. Growing
beyond that, changing the field encoding or changing trigger IDs requires a new
format and migration strategy. There are still 14 unused payload bytes before
the checksum. The retail profile and co-op extension remain in their original
locations.

## Creation and in-game inspection

Every New resets live settings to Regular with all catalogue effects selected.
`chaos_mode` gates only the Mode prompt; choosing Chaos sets `armed` and opens
setup after the optional player-count step. Confirm requires at least one
selected effect. With none, it remains visible in gray and activation plays
`MENU_SFX_LOCKED` without leaving setup. Re-enabling an effect enables Confirm.
This creation constraint does not reinterpret an armed file with an empty
selection as Regular when loading or inspecting it.

An armed Adventure file adds **Chaos Rules** after Options in the hub, race
and gem-cup pause menus. Regular files and other modes have no such action.
Visibility uses the loaded file's `armed` flag, independently of the
creation-prompt option and the number of selected effects.

The pause view reuses the creation screen's rows and rendering, with no Confirm
row and no value editing. **Active Effects** lists only effects enabled in the
file, without an ON/OFF column.
These are the file's enabled effects, not a list of currently running timers.
The creation screen retains its full editable effect catalogue.
Up/Down navigates; Cross/Circle opens Active Effects. Triangle returns from
Active Effects to Chaos Rules, then to the pause menu.
Start or Square returns directly to pause from either page. The pause menu keeps
its selection, and the game remains paused.

The prompt, pause action and profile marker share the custom string catalogue
in `game/RECTMENU_Text.c`. `RECTMENU_GetString` resolves retail LNG indices and
custom IDs starting at `0x4000`, stripping the locked-row flag for either kind.
Both menu drawing and width measurement use this resolver. Custom strings are
available before language loading, and their IDs can be assigned in static
menu initializers. The loader keeps the retail pointer table and string count;
it allocates no extension and needs no custom-string installation state.

## Whole-machine checkpoints

Adventure saves and checkpoints serve different lifetimes. Checkpoints also
restore an in-progress creation, including its menu navigation.

The existing SDATA checkpoint region captures:

- `chaosSettings` and the dedicated `chaosRng`;
- `chaosRuntime`: race initialization, ordered activation entries with independent
  effect/announcement timers and the timed-trigger countdown;
- `newLoad`: current chain step, setup handover flag, parent links and the
  character-select return context;
- `chaosMenu`: the standalone `RectMenu`, current page, scroll offsets, visible
  setting identities, row count, Effects action position and pause return menu.
  A non-NULL return menu also identifies the read-only view;
- `chaosPauseRows`: the extra pause action and navigation links, built from the
  unchanged retail rows each time an Adventure pause menu is requested.

D230 captures the ordinary chain menus, their selections and hierarchy links.
`NativeCheckpoint_RelocateSDataPointers` relocates the standalone menu's pointer
fields, its pause return pointer and the chain's parent/return pointers.
Active/desired menu pointers can therefore target the resident standalone menu
as well as overlay menus.
The callback address uses image relocation; resident and overlay addresses use
their recorded address ranges.

Titles, row specifications and the effect catalogue are immutable program data.
They need no mutable checkpoint copy. Menu values are read from live settings;
there are no value mirrors that can overwrite restored settings on the next frame.
Restoring must not call `MM_ChaosConfig_OnEnter`, which deliberately starts a new
visit on Duration and resets page/scroll state.

Checkpoint validation already requires matching region sizes and offsets.
Adding these resident fields changes SDATA's size, so checkpoints from the prior
layout are rejected rather than partially restored. Cross-build checkpoint
migration is not an Adventure-save migration.

## Race runtime

`RB_Chaos.c` initializes after `VehBirth_TeleportAll` in
`MainGameStart_Initialize`. Updates run before driver stages, only in eligible
Adventure gameplay after GO, with no active menu or pending load. Pause freezes
timers and cadence. `armed` gates execution independently of `chaos_mode`.

Activations are ordered entries, including duplicates. Expiry runs before
timed scheduling; new entries receive their full duration. Interval uses the
configured gap; Random draws each gap between 30% and 100% of that interval,
including the first one. Both retain frame overshoot. The queue holds four entries,
including repeated activations of the same effect;
overflow rejects the new request,
preserving existing entries. A rejected scheduled event consumes its gap but no
RNG choice. Race resets preserve settings and the dedicated RNG stream.

Race end, retry teardown and level-load stage 0 clear runtime state before driver
storage is reused. The runtime contains no pointers and needs no new relocator.
Future driver baselines, cooldowns, contact latches and takeover state must join
captured storage as effects are implemented.

`Chaos_ApplyDriver` runs in driving physics after retail hop-input resolution,
before jump and steering consumers. It applies active entries in order to the
driver and local physics values, checking catalogue scope and excluding finished
racers. There is no stored copy of per-driver input. Steering mirrors the baseline
axis around the controller center, so duplicates do not invert twice. No Hopping
clears the resolved hop latch and buffer; drift input and states remain retail.
Forced jumps remain available. Neither effect writes the shared gamepad or
replaces retail cloud effects. Normal input resumes on final expiry without a
snapshot.

Wobble Steering offsets the live axis with a two-second sine wave at 75% of the
controller range, clamping around the calibrated center (standard pad: 128 ± 128).
Per-activation elapsed game time owns its phase, freezes on pause and is captured
in resident state. Inverted Steering deactivates all Wobble activations and vice
versa, freeing their slots before the capacity check. Rejected invalid requests
leave effects untouched. Duplicates of the same effect retain independent timers
without compounding; the newest Wobble supplies the phase. No controller input
is stored or overwritten, and final expiry/reset returns to live input.

Low Gravity uses one quarter of the driver's gravity; Heavy Karts triples it.
`Chaos_GravityForDriver` derives the effective value from the current
`const_Gravity` when human physics reads it, before retail terrain multipliers.
The stored constant, jump impulse and terminal velocity are untouched, so there
is no baseline snapshot or restoration state. Heavy Karts permits manual hops;
the greater downward force shortens airtime. Both effects apply only to unfinished
humans in eligible races, never bots or hubs. Duplicate entries do not compound.
Activating either gravity effect removes all copies of the opposite effect;
the replaced effect cannot return when the newer one expires.

Speed Capped limits the resolved forward speed target to half the driver's live
`const_Speed_ClassStat`, including when reserves or a mask raise that target.
Already lower targets remain lower. Super Speed doubles the resolved forward
target. `Chaos_SpeedForDriver` runs after retail wumpa/boost/damage calculations
and before the existing `0x6400` hard limit; Super Speed therefore does not
guarantee twice the actual speed when that limit is reached. Throttle, terrain,
slope and collision processing remain retail; Speed Capped is a target ceiling,
not a clamp on the full velocity vector.
`Chaos_AccelerationForDriver` doubles positive forward acceleration for Super
Speed after retail selection (ordinary, reserve or turbo-pad), before frame-time
scaling. Reverse targets/acceleration are unchanged. Neither hook changes stored
constants or boost fuel; expiry/reset uses current retail values without a
baseline snapshot. Both effects apply to unfinished humans only, with independent
duplicate lifetimes that do not compound. Activating either removes all copies
of the opposite speed effect permanently.

No Reserves clears `reserves` and `turbo_outsideTimer` together through
`Chaos_ApplyReserveRules`, called only by the existing driver-input hook.
Retail boost grants remain unchanged: fuel granted after the hook can persist
until the next driver application. This applies to drift/hang-time, turbo pads,
turbo items and Super Engine. Retail boost presentation, input, mask behavior
and base speed are preserved. Only unfinished humans in eligible races are
affected. Duplicate lifetimes remain independent; expiry/reset allows new
reserves without restoring erased fuel. No driver snapshot is needed.

No Items empties held inventory, stack count, roulette/cooldown timers and pending
weapon requests through `Chaos_ApplyItemRules`. Its single call in
`VehPickupItem_ShootOnCirclePress` runs before pending launches and the later
driver roulette update, so even a completed roulette is discarded before award
selection. Crates keep their retail collision/reward path; new rewards are
discarded at the next item check. Already activated masks, shields, remote bombs,
invisibility, Super Engine and projectiles remain active. Wumpa, boost fuel and
rain-cloud ownership are untouched.
Each application clears `WARPBALL_HELD` directly. Roulette sound cleanup and
multiplayer item counters remain in their existing retail paths.
The effect applies only to unfinished humans in eligible races. Duplicate entries
retain independent lifetimes; final expiry/reset allows new inventory without
restoring discarded items. No inventory snapshot is stored.

Wumpa Drain removes one credited fruit per game second from unfinished humans,
stopping at zero. `Chaos_TickWumpaDrain` runs through the existing
`Chaos_ApplyDriver` hook after runtime timers and scheduling have updated.
It uses `ChaosActivation.elapsedMS`, also used by Wobble, and derives the previous
time as `max(0, elapsedMS - frameMS)`. A larger current whole-second count signals
a pulse; each activation removes at most one fruit per frame, even if a long
frame crosses several boundaries.
Each activation starts at zero and has independent pulse/expiry/notice timing;
overlapping copies can therefore drain additional fruit. Newly scheduled entries
do not pulse at zero. Expired entries have already been removed, so the expiry
frame applies no drain, including when it lands exactly on a second boundary.
The clocks are captured in SDATA and freeze with the runtime gates; no
effect-specific runtime timer is needed.
Drain changes only `numWumpas`, not pending pickup animations, collection statistics
or boost fuel. It is a direct modifier rather than damage; mask protection does
not block it. Already drained fruit is not restored on expiry/reset, and pulses
consume no RNG. Newly scheduled entries receive their full first second.

Anxious Mask requests retail rescue when an unfinished human
touches slowing terrain (`speedMultiplier < 0x100`). It reads the
physical contact quad's terrain rather than a cloud-overridden terrain value.
`Chaos_ApplyRescueRules`
runs in `COLL_FIXED_PlayerSearch` before checkpoint progress, preserving
`lastValid` when it adds `DRIVER_COLL_FLAG_MASK_GRAB_REQUEST`. The normal collision
consumer handles rescue eligibility and initialization later in the same callback,
including the existing rescue state, `lastValid`, suppression surfaces and the
retail debug disable flag. Chaos does not filter requests by kart state;
there is no additional cooldown or effect-specific runtime state. The retail
rescue owns destination, camera/input changes,
reserve loss and fruit penalties; Chaos adds no speed ceiling or damage hit.

Defective Boosts (stable ID 12, formerly Clear Boosts) inverts the live fire-speed
bonus through `Chaos_SpeedForDriver`. Retail has already added `fireSpeedCap`, so
the modifier subtracts twice that amount while reserves are nonzero, clamping
the resulting forward target to zero. Stored fire levels, reserve accumulation,
outside-pad bookkeeping and ordinary fuel consumption remain unchanged. This
covers drift, hang-time, starting boosts, normal/super pads, Turbo and Super
Engine. Existing reserves also become a penalty when the effect activates;
empty reserves remove it, and final effect expiry restores the current bonus.
Only unfinished humans in eligible races are affected. Duplicate activations
do not compound the inversion. No new timer, driver field or boost-event hook
is needed; reverse targets and retail acceleration remain unchanged.

Explosive Crates (stable ID 9, formerly Punishing Items) triggers on direct kart
contact with item, Wumpa and time crates. The three crate handlers call
`Chaos_ApplyCrateRules` only when a visible crate actually breaks and the hitter
is a driver. The helper creates the full bomb/missile burst (two crossed
`STATIC_WARPEDBURST` billboards and a shockwave) at the crate through
`RB_Burst_InitVisual`, plays its
positional explosion sound and applies retail blast damage with protection
bypass. Masks, shields and invincibility remain intact; existing damage-state
exclusions still apply. Only unfinished humans in eligible races trigger it.
Duplicate activations cause one explosion per contact. Normal crate rewards,
cooldowns and weapon-hit handling remain retail, including contacts made with
an already-full inventory. No new per-effect state is required.

Hard Crates (stable ID 14) keeps full-size item, Wumpa and time crates intact
when an unfinished human drives into them. Each crate handler returns
`COLL_HITBOX_FORCE_SOLID` before changing cooldown, scale or rewards on direct
kart contact. This overrides the crate's touch-only hitbox class in
`COLL_MOVED_PlayerSearch` and runs the retail solid scrub/impact response, so
the kart hits the crate rather than passing through, spinning or collecting it. Weapon hits, bots,
and crates still growing after an earlier break retain retail behavior. No new
per-effect state is required.

`Chaos_ApplyOneOff` runs once after a successful activation is registered.
Clock Curse uses it to apply the initial spin hit to unfinished humans. The retail
clock sound plays once in `Chaos_ApplyClockCurse`, which shares the retail damage
implementation but bypasses mask, shield and invincibility checks without
consuming those protections. Retail spin setup clears boost reserves and selects
the spin initializer; the mask-grab state exclusion remains in place. Rejected
activations cannot hit or play audio, and each successful duplicate gets its own
initial hit.

Clock Curse assigns `clockReceive` once from the activation's `remainingMS`.
The timer is signed 32-bit in the native tail of `Driver`; its original 16-bit
slot is reserved to preserve existing field offsets. Human and bot countdowns
and clock-waddle feedback retain the full value. Retail clock consumers provide
slowdown, blur and configured weapon-use restrictions. Each new activation
overwrites existing received-clock time, even when shorter; older activations
never resume it. Later retail clocks can overwrite it normally.
Resetting only Chaos runtime leaves the received clock to its ordinary countdown;
race initialization resets driver clocks through the existing vehicle path.
Only unfinished humans in eligible races are affected; no per-effect clock field
or baseline snapshot is needed.

Each activation has an independent three-second announcement, followed by 320 ms
of docking. Successful activations play the transition swish (`0x65`) once;
rejected requests and HUD redraws do not play it.
The swish is preloaded after race banks and music for Chaos files. OtherFX's shared
playback path resolves missing samples from their source banks on demand into a
level-local cache bank; ordinary bank teardown releases it. Positional native
reads preserve the CD cursor/callbacks and do not use MEMPACK. Unexpected cache
misses perform synchronous reads; preloading avoids that work at activation.
Chaos requests double gain (+6 dB), subject to the channel's normal saturation
and Effects volume slider. Menu and PERFECT swishes retain their normal gain.
Separate borderless tables keep docked labels from pushing banners
down the screen. Central banners reserve the viewport rows occupied by Final Lap;
docked rows stay fixed and do not displace central banners. The newest
banner starts at the same viewport quarter-height as solo Final Lap, or the shared multiplayer continue-prompt
baseline; concurrent banners stack downward in 1P and upward in 2P. Crowded layouts use compact rows.
Downscaled GT4 glyphs combine font scale and widescreen compression before
rounding their width, so small letters retain the selected display proportions.
Docked labels use 50% of the small font, six-pixel row spacing and a shared right edge. Both 1P and 2P
show at most the four most recently activated distinct docked effects, newest
first; the 2P list starts at Y=32, above the lap counter. Docked labels and docking
animations use UI OT slot 1, behind lap statistics in the normal HUD layer. This display
limit matches the runtime activation cap. Duplicates use the newest surviving
activation's announcement and remain eligible until final expiry. Drawing uses
the full-screen UI OT. Final Lap is submitted
after the HUD so its timer still reserves space on its last visible frame.
The overlay also disappears as soon as all humans finish, before results setup.
Pause freezes announcements and hides the overlay; menus hide it too. Rejected
activations leave announcements unchanged; reset clears them. Other effects and
the Pickup/External triggers are still pending.

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
including the first one. Both retain frame overshoot. The queue holds 128 entries;
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
limit does not remove running activations. Duplicates use the newest surviving
activation's announcement and remain eligible until final expiry. Drawing uses
the full-screen UI OT. Final Lap is submitted
after the HUD so its timer still reserves space on its last visible frame.
The overlay also disappears as soon as all humans finish, before results setup.
Pause freezes announcements and hides the overlay; menus hide it too. Rejected
activations leave announcements unchanged; reset clears them. Other effects and
the Pickup/External triggers are still pending.

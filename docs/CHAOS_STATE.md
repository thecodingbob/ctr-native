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

## Whole-machine checkpoints

Adventure saves and checkpoints serve different lifetimes. Checkpoints also
restore an in-progress creation, including its menu navigation.

The existing SDATA checkpoint region captures:

- `chaosSettings` and the dedicated `chaosRng`;
- `newLoad`: current chain step, setup handover flag, parent links and the
  character-select return context;
- `chaosMenu`: the standalone `RectMenu`, current page, scroll offsets, visible
  setting identities, row count and Effects action position.

D230 captures the ordinary chain menus, their selections and hierarchy links.
`NativeCheckpoint_RelocateSDataPointers` relocates the standalone menu's pointer
fields and the chain's parent/return pointers. Active/desired menu pointers can
therefore target the resident standalone menu as well as overlay menus.
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

Future active-effect timers, activation order, trigger cadence and takeover
state must join captured runtime storage (or gain an explicit checkpoint region)
when the engine is introduced. File-static mutable effect lists alone would not
meet this contract.

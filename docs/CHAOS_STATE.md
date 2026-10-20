# Chaos state contracts

## Adventure saves

`MemcardData` is copied into the SLOTS payload with its native 32-bit,
little-endian layout. Its retail profile is still `0x1600` bytes and its
header continues to advertise that retail size. The extension is unversioned:

| Payload offset | Contents |
| --- | --- |
| `0x1600` | Four `AdvMultiplayerSettings` entries, 4 bytes each |
| `0x1610` | Four `ChaosSettings` entries, 24 bytes each |
| `0x1670` | Remaining unused payload bytes |
| `0x167e` | Two-byte checksum |

Within each Chaos entry:

| Offset | Contents |
| --- | --- |
| `0x00` | `u16 durationSeconds` |
| `0x02` | `u16 triggerIntervalSeconds` |
| `0x04` | `u8 trigger`, using the current `ChaosTrigger` values |
| `0x05` | `u8 enabledCount`, a cache recomputed from the flags on load |
| `0x06` | 16 one-byte `bool` effect flags, in `ChaosEffectId` order |
| `0x16` | One-byte `bool armed` |
| `0x17` | Compiler tail padding, with no semantic meaning |

`armed` distinguishes Regular from Chaos even when every effect is disabled.
The menu changes only the live settings during creation. Normal Adventure
save/load copies those settings to/from the selected slot; clearing a slot
zeroes its settings. Loading never consults the creation-prompt option.
Legacy files with a zeroed extension are Regular.

The assertions in `namespace_Memcard.h` pin these offsets and the slot stride.
They are migration tripwires, not a limit on the effect catalogue. Adding or
reordering effects, changing trigger IDs, or changing these fields requires an
explicit encoding/version and migration strategy before accepting old files.
Recomputing `enabledCount` cannot repair a shifted `armed` field or slot stride.
Growing the native array alone is not supported. There are only 14 unused bytes
before the current checksum, so a future encoding must also review payload size.

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

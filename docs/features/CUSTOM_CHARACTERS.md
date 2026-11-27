# Custom characters: first playable prototype

This is a development prototype, not the full character importer. It started
with Arcade 1P, but selection, audio and model admission no longer enforce that
mode or player count. Assets are local user inputs and are not included in the
repository.

## Miku playtest

Run the normal `build/crash_team_racing_expanded.exe`, select regular Arcade,
one player and a single race. On character select, move to Papu and press Down
to reach the extra slot, physically placed below Papu Papu. The name comes from
`character.ini`; the slot has its own icon and uses the custom racing model as
its 3D preview. Press Up to return to Papu, or confirm and choose a track.

Navigation follows logical rows/columns, with no diagonal fallback or wrap.
With only one active custom below Papu, that cell can be unreachable when the
neighbouring bonus characters are locked.

## Matrix-based character selection

`D230.characterSelectGrids` holds the writable layout matrices directly. A cell
is `CHARACTER_GRID_UNUSABLE` (-2), `CHARACTER_GRID_EMPTY` (-1), or a roster entry
(retail IDs 0–15, followed by custom entries). Only EMPTY cells may receive a
custom entry. Locked retail characters retain their cells. Cursor state stores
row and column; navigation skips empty/unusable/locked/occupied cells along
the requested axis, stopping at the edge.

The old per-layout metadata copies, directional neighbours and diagonal fallback
tables are removed. A single derived draw-entry array supplies coordinates and
base IDs to the renderer. The existing transition arrays have extra capacity
directly, without copying them into a second set of tables.

Retail coordinates, spacing and preview-window sizes are retained. The eight
extra cells follow the user's screenshots:

- 1P/2P: one below Roo, one below Papu, three in a new outer column on each side;
- 3P: four above Joe/Penta/Fake Crash/Oxide and four below Tropy/Pinstripe/Roo/Papu;
- 4P: two to the left of Roo/Joe, two to the right of Papu/Pinstripe, two above
  Penta/Oxide and two below Tropy/Fake Crash (the lower central pair).

Axis coordinates are separate from the cell contents. This preserves every
retail position while placing the new 3P/4P edge rows at Y=0 and Y=183 to keep
their icon rectangles on screen. No retail row or preview window is moved.
The first custom stays below Papu in 1P/2P and 3P; in 4P it occupies the new cell
immediately to Papu's right. The menu exposes up to eight scanned characters
with recognized retail bases, preserving their separate names and identities.
The local layout fixture contains Miku plus seven numbered copies, under
`assets/characters/miku-2/` through `miku-8/`. These ignored assets are not shipped.
Custom icons still share the prototype's first icon texture (identical for these
copies); independent multiplayer identities remain unimplemented.

## Local fixture

The local fixture is `assets/characters/miku/`:

- `character.ini`: stable text identity, display name, author, retail base and
  file references;
- `models/hi.ctr`: standalone racing model with its internal name changed to
  `miku`, leaving the extracted source untouched;
- `models/med.ctr` and `models/low.ctr`: preserved imported LODs (not yet loaded);
- `icon/pixels.bin`: 44 by 26 indexed pixels, low nibble first;
- `icon/palette.bin`: 16 little-endian PS1 palette words;
- `audio/short-00.sfx` and `audio/short-01.sfx`: imported short voice effects, with their
  eight-byte HOWL OtherFX metadata followed by PS1 SPU ADPCM;
- `audio/line-00.xa` through `audio/line-18.xa`: imported voice clips, isolated
  by channel from the patched English GAME XA files using the XNF sector limit.

The prototype admits the first eight scanned characters with recognized retail bases.
`base` uses the retail model key: `crash`, `cortex`, `tiny`, `coco`, `ngin`,
`dingo`, `polar`, `pura`, `pinstripe`, `papu`, `roo`, `joe`, `ntropy`, `pen`,
`fake` or `oxide`. The key resolves through the existing character metadata,
not a separate Polar-specific mapping.
It keeps that base's retail ID for physics, audio and shared pack loading, and stores
the custom identity and model reference separately. Only player 1 receives the
custom model, so the retail base model is not globally replaced. The model is
allocated with the current level in MEMPACK. A failed load uses the retail base
and writes a warning to the log.

The racing model loads after the track assets, retaining the exact aligned
budget for pools, clipping and viewport visibility lists. If the model
would consume that reserve, the retail base is used instead. The preview loads
into the already initialized menu and needs no race reserve.

The preview reuses the racing model, not a separately animated menu model.
Model resolution is owned by `game/LOAD/LOAD_CharacterModels.c`, for both
resource APIs declared in `include/custom-characters/character_resources.h`:
`LOAD_GetDriverModel(driverID, baseID)` and
`LOAD_GetCharacterPreviewModel(driverID, baseID, identity)`. The resolver chooses
the custom model or the context's retail fallback: standalone/player-pack models
for drivers, LEV models (including the Oxide fallback) for menu previews.
Custom identity is not inferred from the retail model name, so another driver
using the same base retains the retail model.

The menu tracks current and desired preview roster entries separately from their
retail base IDs. The current entry and base change together at the slide midpoint,
before resolving the model and wheel size; even a custom and its own retail base
are distinct preview selections. These scalar entries live in `D230` and are
included in native state. Navigation coordinates remain in the matrices/cursors;
the derived draw metadata keeps only screen coordinates and retail metadata.

`CustomCharacter_Select` owns updating or clearing the selected character's
identity, retail base and model/audio paths. The menu chooses an entry rather
than editing the resource snapshot itself.

`LOAD_TenStages` only invokes `LOAD_FinalizeCharacterModels` after level assets
and before pools. That phase may allocate a custom driver model with the remaining
initialization budget reserved. Driver spawning only resolves prepared models;
preview requests may load into menu memory. The two contexts keep separate
identity-keyed caches, reset with the level and captured in native state.
`LOAD_ResetCharacterModelCache` owns resetting these caches; the load coordinator
calls it alongside the separate standalone-bot reset before old allocations are
released. Selection metadata is preserved across cache teardown.
`VehBirth` and character-select drawing no longer apply custom model overrides
or read custom model files themselves. Model detail remains HI in this prototype.

Imported voice resources take precedence over the global voice-language choice;
individual missing clips use the normal retail-language fallback. Short samples
share SPU upload, ownership transfer and cache completion with localized samples.
Both kinds
are allocated in the level's SPU cache, while XA streams keep their custom file
reference in audio snapshots. The `[audio]` section maps character-local keys
`short_00`–`short_01` and `line_00`–`line_18` to relative file paths. Filenames
are editable and no longer encode Polar's retail IDs. Semantic names can replace
the numbered filenames once the individual clips are identified; the keys retain
their local mapping. Missing entries use retail fallback. All asset references
stay relative to the character directory, without absolute paths or `..` components.
The importer must translate the source slot into these local indices. XA local
indices follow voice-set order, not subtraction from a retail track ID; this
also handles Ripper Roo's out-of-order IDs. Where retail voice sets share a
track, resolution uses its first matching local index. Short voices map the
selected base's two OtherFX IDs to `short_00` and `short_01`.

The game registers its character XA resolver with `NativeAudio` at startup.
Character selection and retail-to-local voice mapping stay in
`game/custom-characters/character_policy.h`; the backend does not read game state.
`CDSYS` requests ordinary track IDs for both playback and length. Both use the
same native source resolver, preferring custom clips before localized/retail
resources. Audio snapshot restore reopens the saved source rather than applying
the current selection's overrides. HOWL retains SPU caching, using the same
character policy module for short-voice indices.

Keep only runtime resources here, with all voices directly in `audio/`.
Extraction originals and verification images belong outside the runtime library.
The stable `id` in `character.ini`, not the folder name, identifies the character.

Race HUD names/icons and podium still use the retail base.
Custom identity/model assignment still follows player 1; removing the mode gate
does not implement independent multiplayer selections or custom AI. Adventure
hub and podium model loading, persistence, a full library manager and automatic
patch import remain incomplete. Do not use arbitrary untrusted models: current bounds
checks cover the outer model envelope and relocation table, not every nested
mesh or animation structure.

Build validation does not establish visual correctness. The user performs the
in-game playtest, including selection, racing and returning to the menu.

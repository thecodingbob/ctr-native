# Custom characters: developer reference

## Library and identity

`platform/custom-characters/` scans `assets/characters/`. Each character has a
`character.ini` with these sections:

- `[character]`: stable `id`, `name`, `short_name`, `author` and retail `base`.
- `[assets]`: `hi`, `med`, `low`, `icon_pixels` and `icon_clut` file references.
- `[audio]`: character-local `short_00`–`short_01` and `line_00`–`line_18` references.

Asset paths are relative to the character directory; absolute paths and `..`
components are rejected. Voices live directly in `audio/`. Racing models are
standalone CTR files. Icons contain 44×26 indexed pixels, low nibble first, and
16 little-endian PS1 palette words. Short voices contain eight bytes of HOWL
OtherFX metadata followed by SPU ADPCM; voice lines are channel-isolated XA clips.

`NativeCharacterLibrary_RefreshAssets` scans at startup and on entering character
selection, so random opponents do not require visiting selection first.

The stable identity is independent of the directory, roster position and retail
compatibility ID. `base` resolves through `GAME_CHARACTER_METADATA[].name_Debug`
and supplies physics and retail fallbacks. `CustomCharacter_Select` owns each
driver's resource snapshot, including identity, base and asset paths.
Different customs may share a base or coexist with that retail character.
Saved retail-only IDs must not be interpreted as current custom identities.

## Selection and resource loading

`CustomCharacter_PrepareAvailableEntries` supplies the same active roster (up to
eight valid scanned entries) to selection, menu icons and opponent preparation.
`D230.characterSelectGrids` contains roster references, EMPTY (-1) or
UNUSABLE (-2) cells. Only EMPTY cells accept custom entries. Navigation stays
on the requested axis without wrapping; cursors are restored by identity.
Preview transitions track roster entries separately from retail bases.
Retail icon geometry and preview sizes are preserved; with customs present,
3P/4P rows use Y=3,39,75,111,147,183.

`game/LOAD/custom-characters/` owns model and icon resolution through
`include/custom-characters/character_resources.h`:

- `LOAD_GetDriverModel` / `LOAD_GetCharacterPreviewModel` return custom models
  or the context's retail fallback.
- `LOAD_GetDriverIcon` / `LOAD_GetCharacterPreviewIcon` return ordinary `Icon`
  descriptors, falling back independently to the retail base icon.

Driver models are prepared after track assets and before pool initialization,
preserving the aligned memory reserve for pools, clipping and visibility lists.
Failed or over-budget loads use retail models. Previews reuse the racing model.
Separate identity-keyed race and preview caches share models for the level
lifetime; spawning only resolves prepared models. Cache reset precedes releasing
level allocations and preserves selection snapshots.

Icons are prepared on opening selection or from driver snapshots during level
loading. Drawing performs no file reads or uploads. Host texture bindings use
the generic `DecalGlobal` registry: providers own and unregister their bindings
before releasing textures, without affecting other providers. `DecalHUD` has no
character-loading policy. Checkpoint restoration repairs model pointers and
rebuilds icon textures instead of serializing OpenGL handles.

## Random opponents

`game/LOAD/LOAD_Roster.h` prepares eligible entries for both field-size prediction
and AI selection. Random All includes retail and active custom characters;
Random Unlocked filters retail unlocks but treats customs as always unlocked.
Human selections and retained cup opponents are excluded by identity, not base.
Vanilla uses retail candidates; boss races and the Purple Gem Cup retain their
managed rosters.

Assignment snapshots custom resources per driver, including expanded-grid AI
slots, using the same model, icon and voice resolvers as humans. Retail fallback
models are queued once per base. Later cup rounds retain eligible opponents.
`LOAD_RosterFinalize` publishes the admitted field and prunes unused snapshots
once, after driver requests are queued; managed rosters retain only human custom
resources. The existing menu backup/restore entry
points save and restore custom snapshots together with retail IDs, so race AI
assignments cannot become human selections. Backups contain resource paths, not
borrowed library pointers or host texture handles, and are captured in native state.

## Audio ownership

Explicitly speaker-owned requests resolve custom audio, then the selected voice
language, then retail audio. Unowned requests and retail speakers use normal
language/retail fallback, even when sharing a custom character's base.

`game/custom-characters/character_policy.h` maps the base's two OtherFX IDs to
local short-voice indices and XA voice-set order to local line indices. Shared
XA tracks use the first matching local index, not retail-ID subtraction.
Speaker context follows immediate and queued playback; short-voice caches are
per driver. Options voice previews use the followed driver and restore context.

`NativeAudio` uses the registered source resolver for playback and length,
without reading game state. Audio snapshots retain their source so restoration
does not apply a newly selected character's overrides. HOWL retains SPU cache
ownership and upload handling.

## Compatibility boundaries

Custom resources cover human selection, random AI, HI racing/preview models, voices and
icons in race standings, progress portraits and current-race/cup/versus/battle
results. Race HUD names, podium models and retail-only historical records retain
the retail base. MED/LOW loading, Adventure hub/podium integration,
persistent custom records, activation management and automatic character patch
import are outside the supported path.

Model admission checks the outer envelope and relocation table, not every nested
mesh or animation structure. It does not establish arbitrary model compatibility;
untrusted models are unsafe. Custom meshes must not inherit retail animations
without compatibility validation.

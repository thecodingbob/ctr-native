# Chaos external file provider

The game's External trigger accepts one effect request at a time by reading an
externally written file.

## Files

Paths are relative to the game's resolved asset base directory (the directory
containing `assets/` in the usual portable layout), not the shell working
directory. The game creates `chaos/` when publishing the catalogue.

| File                | Owner             | Contents                                      |
|---------------------|-------------------|-----------------------------------------------|
| `chaos/effects`     | Game              | One enabled, implemented effect slug per line |
| `chaos/command`     | External producer | One effect slug, e.g. `low-gravity`           |

The final files have **no extension**. Use ASCII or UTF-8 without a BOM.
Surrounding whitespace, including a final newline, is accepted in a command.
No verbs, durations or extra arguments are supported. Commands must be shorter
than 128 bytes; oversized input and embedded NUL bytes are consumed and rejected.

The catalogue reflects the active Chaos save's enabled selection for any trigger.
It is published at game initialization and again in `Chaos_BeginRace()`, never
from frame updates. It is empty when Chaos is unarmed. Publication failures are
logged; another attempt occurs at the next initialization, without frame retries.

## Producer sequence

1. Read `effects` and choose an available slug.
2. Wait until `command` is absent. Use only one producer for this slot.
3. Write the slug to `command.tmp`, close it, then atomically rename it to
   `command` in the same directory. Never overwrite a pending command.
4. Wait for `command` to disappear before publishing the next one.

For example, in PowerShell with the current directory set to the asset base:

```powershell
if (-not (Test-Path -LiteralPath 'chaos/command')) {
    [System.IO.File]::WriteAllText(
        (Join-Path (Get-Location) 'chaos/command.tmp'),
        'low-gravity',
        [System.Text.Encoding]::ASCII)
    Move-Item -LiteralPath 'chaos/command.tmp' -Destination 'chaos/command'
}
```

## Activation timing

- Load/create an armed Chaos Adventure save with **External** as its trigger.
- The first request waits one saved `interval` from GO. Subsequent requests wait
  one full interval from the previous successful external activation.
- Only eligible, active race time counts. Pauses, active menus, countdowns,
  loading, hubs and results do not consume a command or spend the interval.
- After the interval expires, the game checks the slot at most once per 100 ms
  on normal frames. An empty slot leaves it ready; there is no extra interval
  when a command later arrives.
- A pending command survives race changes. Each new eligible race waits a fresh
  interval from GO before consuming it.
- Effects use the save's duration. They may overlap if duration exceeds interval;
  reactivating an effect follows the ordinary unique-ID refresh/replacement rules.
- An interval of zero allows the next request on the next eligible poll.

The external voting timer is independent of gameplay time. A winner submitted
while paused or before the interval expires stays pending. There is no timing or
readiness channel to synchronize an overlay countdown with the game.

## Consumption and errors

The game reads, closes and deletes the command before applying it. If reading or
deletion fails, no effect runs and the file is retained for retry. Invalid,
unknown, disabled or otherwise rejected effects are consumed and their reason is
written to the normal application log (`[Chaos]` entries). Rejections do not
restart the interval.

**Deletion acknowledges consumption, not successful activation.** There is no
outbox or result file. Do not use deletion alone to acknowledge a paid request
in an external integration.

Replay playback neither consumes commands nor publishes catalogues. External
requests are not recorded in replays, so provider-driven recording verification
is not guaranteed. Pending files are outside quick states; loading a quick state
does not restore an already consumed command.

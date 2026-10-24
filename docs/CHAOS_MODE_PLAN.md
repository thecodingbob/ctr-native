# Chaos mode — remaining work

## Status

Implemented: per-Adventure-file settings and persistence, creation/pause menus,
all 16 catalogue effects, activation HUD/audio, and Interval, Random, Pickup and
External triggers.

Implementation contracts: [CHAOS_STATE.md](CHAOS_STATE.md).
External producer protocol: [CHAOS_EXTERNAL.md](features/CHAOS_EXTERNAL_PROVIDER.md).

## Remaining work

- Diagnose the rare GPU-link crash using `gpu-link-crash.log`; its cause is not
  established.
- Replace screenshot markers in [the player guide](features/CHAOS.md) with in-game captures.
- Build the external voting application described below.

Deferred effect ideas: Hyper Bots, Blind Kart, Tiny Karts, Item Rain. New effects
must append stable IDs/slugs; the current save bitset supports 128 effects.

## External voting application

The intended application reads the enabled catalogue, presents three randomly
selected effects, collects votes until its timer expires and submits the winner
through the single command slot. Voting, Twitch integration and stream overlay
belong to that application.

Decide before implementation:

- behavior with fewer than three effects, ties and no votes;
- whether candidates may repeat between rounds;
- one vote per viewer and whether votes can be changed;
- when the next round begins relative to command consumption.

The voting timer uses external time; the game interval uses active race time.
Exact synchronization through pauses/countdowns would require an additional
readiness/timing channel. The existing catalogue does not provide one.

## Validation entry points

Build: `cmake --build build -j`.
Run Chaos suites: `ctest --test-dir build --output-on-failure -R test_chaos`.

Suites cover catalogue/gates, new-file flow, setup, save encoding, runtime,
Pickup hooks and filesystem IPC. They do not replace visual menu/HUD checks,
full Adventure disk I/O or full checkpoint/replay validation in game.

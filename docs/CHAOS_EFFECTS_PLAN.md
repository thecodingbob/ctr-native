# New Chaos Effects — Implementation Plan

Source: [issue #90](https://github.com/thecodingbob/ctr-native/issues/90).
Scope: all 24 effects, implemented individually in A → B → C → D → E order.
Complete focused validation and user review before starting the next effect.
Add shared infrastructure only as required by the current effect.

Implemented behavior: [player guide](features/CHAOS.md).
Code ownership and integration contracts: [implementation guide](CHAOS_IMPLEMENTATION.md).
This plan tracks delivery, remaining specifications and outstanding validation.

## Delivery status

| Block | Effects, in implementation order | Status |
| --- | --- | --- |
| A. Items and reserves | Use This!, Juiced Up, It is I, Providence, Reserve Injection, Too Boosted TO Fail | Implemented |
| B. Driving, turbo and jumping | Sabotage, This Is Fire, Hydraulics, My Planet Needs Me, Who Bound This?, Try Something New | Implemented |
| C. Size, camera and bots | Small Kart Energy, Classic GTA, Arena Champions, You've Got Mail, Fresh Start, Senile Mask | Implemented |
| D. Characters and movement | Identity Crisis, Body Swap, Teleport, Defective Teleport | Implemented |
| E. Time manipulation | Za Warudo, King Crimson | Za Warudo implemented; King Crimson prototype implemented |

**Current effect: King Crimson.** Technical prototype implemented; in-game validation is pending.
The user's Defective Teleport smoke test passed.

Teleport's user smoke test passed; combined 2P/Body Swap/rescue checks remain pending.

Teleport regression checks include real retail progress projection on a coherent
synthetic circuit: destination stability, finish wrapping and selected branches.

## Remaining specifications

### Za Warudo

Humans keep driving while bots and the dynamic environment freeze, including
hazards, environmental animations, world particles and nonhuman weapons.
Human-owned weapons keep moving; their effects on frozen entities are retained
until time resumes. Crates and pickups remain collectible. Karts are mutually
pass-through. Nitro and potions also allow passage: contact records an impact
without blocking or damaging the human, then detonates when time resumes.
Other frozen hazards retain collision checks and can still hurt humans.
Triggered Nitro uses the ordinary explosion radius at resumption; red/green
potions recheck their normal contact radius and shatter even if nobody remains
nearby. Humans who have moved outside the relevant radius take no damage or red
cloud from the old contact. Mask hits on frozen karts are recorded
without blocking passage and apply once on resumption.
Human masks, boost flames and kart particles keep following/animating with their
kart; mask power-up duration remains suspended.

Duration is the smaller of the Chaos duration and 10 seconds. Race/special-event
clocks, power-up timers and other Chaos timers stop; only this effect's expiry
clock continues alongside human driving.

Use a blue/yellow-emphasized palette. Play the clock item sound followed by a
breaking time crate, then six normal-pitch time-crate ticks at equal intervals;
the rest of the freeze is silent apart from human sounds.
Suspend music and world/bot sounds; human sounds, menu navigation/confirmation
and dedicated effect sounds remain active, including pause entry and exit.
Options FX/voice/music slider previews also remain audible; the music preview
does not resume the race soundtrack.

Implemented with selective thread updates, a separate activation expiry, frozen
world/texture/particle clocks, selective audio suspension and a blue/yellow render
pass before the HUD, with vivid scene colors and unmodified HUD colors.
Human driving, cameras, masks, boost flames and kart particles continue;
item-effect durations stop.
Driving cooldowns and reserve consumption continue with human physics.
Direct human collections remain immediate. Human weapon impacts against frozen
karts, weapons and level objects resolve on resumption; pending participants stay
allocated until dispatch. Human pickup, launch and weapon sounds remain audible.
Activation retest must include recycled missile/orb/mine payloads and human weapon
particles: launch audio requires initialized ownership, and particle metadata
must not advance axes beyond the eleven-element array.

### King Crimson

Approximate 10 seconds of bot movement along the track path based on their speed,
without full simulation; advance the race timer by 10 seconds. Start with a
technical prototype. Bots may finish during the skip; update results and event
completion accordingly.

Only bot movement/progression and the race timer advance. Environment, weapons,
power-ups, other states and Chaos timers stay unchanged. Apply a blue/yellow-
emphasized palette for half a second and play the clock item sound followed by a
breaking time crate.

Investigate path-based advancement, lap/checkpoint consistency and destination
placement without moving humans. The two time effects replace each other on
activation.

Prototype: instant ID 39, `king-crimson`, restricted to events that support racing
bots. Reuses Teleport's checkpoint traversal/branch selection and interpolation,
then reconnects each moving unfinished bot to its existing AI path using the
nearest-navigation search shared with Body Swap's bot handover. Travel uses the
current AI speed, including Arena Champions' motion multiplier, for 10,000 ms;
there is no physics, collision or item simulation during the skipped interval.
Stationary bots remain stationary. The shared ordinary finish-line operation
counts every crossing chronologically, preserving finish order and skip-relative
finish times. Ordinary lap-statistics/results processing handles event completion.
The race clock advances even when a bot cannot be placed because its track/nav
data is unavailable; that bot stays put. Other Chaos, damage, reserve and item
timers do not consume skipped time. The instant notice supplies the 500-ms palette
window and delayed crate cue, sharing Za Warudo's grading and audio helpers without
ticks or a timed slot. Replacing Za Warudo releases audio and resolves its queued
impacts before relocating bots. Replacing King Crimson cancels its pending short
presentation.

Offline validation: application build and all 26 CTest checks pass. Chaos effects
suite: 8,094 assertions, including unchanged human/timer state, chronological bot
finishes, multi-lap skips with real checkpoint projection, mutually exclusive time
effects, the 500-ms palette window and shared cues without King Crimson ticks.

## Outstanding in-game validation

King Crimson: check 1P/2P, corners/ramps and stacked or alternate routes; bots must
continue forward smoothly after placement. Verify the ten-second clock jump,
multi-bot final-lap finish order and results, Body Swap ownership, Arena Champions,
rescues, stationary/damaged bots, repeated skips, pause/retry and quick states
during the half-second palette. Check both replacement orders with Za Warudo,
including pending deferred hits, and confirm unchanged human positions, HUD/menu
colors, weapons, power-ups and other effect durations at activation.

Asset-free suites cover scheduling, lifecycle/conflicts, driver scope, HUD and
gameplay operations. They do not replace the following visual, audio and physics
checks. Use `chaos_quick_race` for isolated effects and 2P Adventure for per-human
behavior.

| Effects | Checks |
| --- | --- |
| Item gifts | Renewed mask/shield visuals and 1P/2P announcements. |
| Sabotage | Square, Square + throttle, airborne braking, left-stick reverse and right-stick braking/reverse. |
| This Is Fire | Activation with/without reserves; drift/jump/pad grants; earned USF survives expiry until normal boost loss. |
| Hydraulics | Flat-ground hopping, landing/cooldown, drifting, slopes, jump-lockout clouds and gravity modifiers. |
| My Planet Needs Me | Ground/rising/falling launches, slopes, repeated activations and gravity/size modifiers; at high speeds confirm the bounded launch stays airborne without reversing. |
| Who Bound This? | Linear driving, drift entry and drift updates; analog mapping and held/tap/release behavior. |
| Try Something New | Repeated 1P/2P class changes, engine audio, drifting, speed/gravity modifiers and retry restoration. |
| Small Kart Energy / Giant Karts | Body/tires, collisions/squash/protection, shadows, skids, particles, flames, shields and masks at 0.5×/1×/2×; size changes while drifting and airborne. TNT retains its normal dimensions. |
| Classic GTA | Cortex Castle floor and Roo's Tubes ceiling classification; Sewer Speedway, ramps, stacked roads, jumps, rescue, size effects, 1P/2P, pause and expiry. Mixed floor/ceiling blocks intentionally remain visible. |
| Arena Champions | 1P/2P, bosses/cups, cornering/jumps, boost/damage, expiry and coexistence with human Super Speed. |
| Fresh Start | Suspension/mask visuals, neutral pose and camera smoothing during drift/jumps; rescue deferral, starting boost/over-rev, damage recovery, repeated activation, 1P/2P lights/audio and HUD spacing, pause/reset, unchanged bots/global timer. |
| Senile Mask | Flashing prompt in each correct-direction human viewport, including relic/crystal events; repeated Aku Aku/Uka Uka voice with half cooldown, no overlapping XA playback; silence/no prompt when actually wrong-way, normal warnings restored at expiry/reset. |
| Identity Crisis | All character models/textures, including bosses and Oxide; smoke coverage/timing at speed and at 0.5×/1×/2× size in both viewports; repeated 1P/2P transformations while driving, jumping, damaged or rescued; no duplicate live characters; class/engine ordering with Try Something New; retry/end-of-race and quick-state restoration, including during the smoke delay. |
| Body Swap | Human/human and human/bot input, camera/HUD and engine audio; damage/airborne takeover, bot navigation reattachment, active weapons/shields/masks, boss/cup behavior; modifiers follow players while class/Identity Crisis follow karts; immediate activation/expiry during rescue/start holds and pending transformations, reactivation, final-lap early return, retry/results and quick states. Relic/crystal/battle 2P pairs and solo events without targets. |
| Teleport | Combined 2P/Body Swap/rescue checks remain pending after the user smoke test. |
| Defective Teleport | Combined 2P/Body Swap/Fresh Start/rescue and retry/quick-state checks remain pending after the user smoke test. |
| Za Warudo | 1P/2P passage through karts and Nitro/potions; touched Nitro/potions detonate only on resumption. Other frozen mines, missiles and track hazards retain collisions; frozen world particles and live human masks/flames/smoke. Human projectiles and their effects continue, with external impacts deferred exactly once until resumption; test blasts against several targets, potion/door impacts and quick states with pending hits. Crates/pickups stay collectible, with audible Wumpa/launch sounds. Race/relic/crystal clocks, item-effect durations and other Chaos timers stop/resume normally. Blue/yellow grading excludes HUD; six normal-pitch ticks precede silence, while music/world/bot sounds pause and human sounds continue. Rescue, Body Swap, Fresh Start, pause, expiry and results/retry remain pending in-game checks. |

Record remaining failures and reproducible tracks/quick states here.

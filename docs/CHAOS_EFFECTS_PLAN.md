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
| D. Characters and movement | Identity Crisis, Body Swap, Teleport, Defective Teleport | Identity Crisis implemented; remaining effects not started |
| E. Time manipulation | Za Warudo, King Crimson | Not started |

**Current effect: Identity Crisis.** In-game validation is pending; Body Swap is next.

## Remaining specifications

### Body Swap

For the Chaos duration, possess another kart with its character, class, inventory,
reserves, damage and lap/checkpoint progression; the other controller takes the
previous kart. Form random disjoint pairs involving humans, with bot or human
targets. Return at expiry, or end the pair's swap before either kart finishes.
Human Chaos modifiers follow the player.

Investigate input/HUD ownership and interaction with character/class changes.

### Teleport / Defective Teleport

Move forward/backward 10% of a lap along the track path. Crossing the finish line
in either direction updates laps/checkpoints accordingly. Preserve speed,
reserves and inventory; align orientation/camera to the destination. Follow the
chosen branch, otherwise the main path.

Use a shared repositioning operation that updates terrain, checkpoints, rescue
state and camera together. Investigate path branches and lap validation.

### Za Warudo

Humans keep driving while bots and the dynamic environment freeze, including
hazards, environmental animations, world particles and all weapons. Newly fired
human weapons also freeze until expiry. Crates and pickups remain collectible;
frozen objects retain collisions.

Duration is the smaller of the Chaos duration and 10 seconds. Race/special-event
clocks, power-up timers and other Chaos timers stop; only this effect's expiry
clock continues alongside human driving.

Use a blue/yellow-emphasized palette. Play the clock item sound followed by a
breaking time crate. Suspend music and world/bot sounds; human sounds and any
dedicated effect sound remain active.

Investigate selective thread updates, an independent expiry clock and resumption
without accumulated catch-up time. Stretch goal: defer crate-breaking animations
and damage to opponents until the effect ends.

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

## Outstanding in-game validation

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

Record remaining failures and reproducible tracks/quick states here.

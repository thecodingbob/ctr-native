# New Chaos Effects — Implementation Plan

Source: [issue #90](https://github.com/thecodingbob/ctr-native/issues/90).
Status: block A implemented with focused test coverage. Later technical checks
remain marked with placeholders; in-game visual validation is pending.

Existing contracts: [implementation guide](CHAOS_IMPLEMENTATION.md).
Existing gameplay: [player guide](features/CHAOS.md).

Edit the agreed behavior directly during review. `[VERIFY: ...]` marks remaining
technical investigation; `[NOTES: ...]` is space for findings.

## Shared Work

- Support instant actions alongside timed effects. Separate their
  notifications from ongoing modifiers.
- Instant one-shot effects show only the activation notification and do not
  dock in the active-effects HUD, even when their result persists independently
  (for example, the orb launched by You've Got Mail).
- Add bots-only and individually selected targets; introduce effect-eligibility
  filtering when those targets or event-specific restrictions require it.
- Extend conflict rules for size, reserves, jumping, steering, and time effects.

Zero duration is invalid. Current effects draw directly from enabled settings;
recipient checks belong to application, and race end stops Chaos. At race start,
publish enabled commands only in eligible races; validate requests at execution
time. Add per-effect event/target filtering when such restrictions are introduced.

## A. Items and Reserves

| Effect              | Behavior                                                                                                                                                                                  |
|---------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Use This!           | Replace the held item and stop any roulette. Roll independently per human, uniformly across event-appropriate items, including orb/clock and multi-item packs as separate outcomes.       | 
| Juiced Up           | Fill Wumpas to 10 through normal gradual pickup crediting and the full-count chime; remains eligible when already full. |
| It is I             | Activate mask item directly without consuming the held item; use normal duration, Wumpa rules, and incompatibilities. Renew if already active.                                            |
| Providence          | Activate shield directly without consuming the held item; use normal color, duration, Wumpa rules, and incompatibilities. Renew if already active.                                        | 
| Reserve Injection   | Bring reserves to at least ten seconds (9,600 units), preserving larger balances. If not already boosting, activate third-drift-boost fire. Remains eligible when already full and boosting. |
| Too Boosted TO Fail | Reserves can only increase: boosts add reserves; consumption, damage, U-turns, and rescues cannot reduce them.                                                                            |

Conflicts use last-activation-wins: the previous incompatible effect disappears.
Too Boosted TO Fail and No Reserves replace each other. Use This! ends No Items;
Reserve Injection ends No Reserves before granting its reward.


## B. Driving, Turbo, and Jumping

| Effect             | Behavior                                                                                                                                                     |
|--------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Sabotage           | Disable the brake command entirely, including reverse and airborne braking.                                                                                  |
| This Is Fire       | Upgrade existing turbo and new turbo grants to USF without free reserves. Earned USF survives expiry according to normal game rules.                         |
| Hydraulics         | Automatically jump with normal force as soon as the game allows, with no additional delay; may interrupt drifting.                                           |
| My Planet Needs Me | Apply one immediate upward impulse at 8× normal jump force, including while airborne; active gravity modifiers still apply.                                  |
| Who Bound This?    | Independently permute throttle, brake, item, and both hop/drift actions per human for the Chaos duration. Keep steering, pause, and menu controls unchanged. |
| Try Something New  | Select a different standard driving class until race end, keeping the character.                                                                             |

No Hopping and both forced-jump effects replace each other on activation.
This Is Fire and Defective Boosts also replace each other.

Who Bound This? leaves steering unchanged and can coexist with steering effects.
For Try Something New and Identity Crisis, the most recent class change wins until race end.

[VERIFY: turbo grant paths in `game/Vehicle/VehFire.c`; jump press edges; analog
input; class restoration after character changes].

## C. Size, Camera, and Bots

| Effect            | Proposed behavior                                                                                                                                                                                               | Open decisions                                                    |
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------|
| Small Kart Energy | Dimensions/collisions at 0.5×; speed and jump at 0.7×. Larger karts squash it using existing damage duration, immunity, and mask/shield protections.                                                            | Decided.                                                          |
| Classic GTA       | Temporary vertical top-down camera per human viewport, oriented with the kart. Keep this view in tunnels and hide obstructing geometry.                                                                         | Height/transition to be tuned in game; verify occlusion handling. |
| Arena Champions   | Double bot speed and acceleration (2×); disable rubber-banding during the effect.                                                                                                                               | Decided.                                                          |
| You've Got Mail   | Launch a normal orb from the last-place bot at the highest-placed human, fixed at launch and independent of bot Wumpas, without consuming inventory. Must work even when the source bot is ahead of the target. | Decided.                                                          |
| Wait For It       | Immediately bring the kart to the ground, stop it, clear reserves, and replay the normal starting lights with starting boost available. Bots keep racing.                                                       | Decided.                                                          |
| Senile Mask       | For the Chaos duration, show the flashing wrong-way prompt while humans drive in the correct direction. Play the wrong-way voice twice as often and at double its normal volume.                                | Decided.                                                          |

Dependencies:

- Small Kart Energy requires fractional scaling; it replaces Giant Karts and vice versa.
- You've Got Mail needs explicit human targeting in `game/231/RB_Warpball.c`.
- Wait For It needs a local sequence independent of the global starting timer.
- Senile Mask needs a wrong-way presentation override without changing actual
  race progression, plus voice cadence and volume overrides.

[VERIFY: fractional scale consumers and collision behavior; camera occlusion;
bot cornering/jumps; existing orbs and finished targets; damage during restart;
normal wrong-way voice cadence and gain, and per-player warning behavior].

## D. Characters and Movement

| Effect             | Proposed behavior                                                                                                                                                                                                                                                                                                                                                         | Open decisions |
|--------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------|
| Identity Crisis    | Until race end, switch to a random playable character not already present in the race, regardless of unlocks, including that character's driving class.                                                                                                                                                                                                                   | Decided.       |
| Body Swap          | For the Chaos duration, possess another kart with its character, class, inventory, reserves, damage, and lap/checkpoint progression; the other controller takes the previous kart. Form random disjoint pairs involving humans, with bot or human targets. Return at expiry, or end the pair's swap before either kart finishes. Human Chaos modifiers follow the player. | Decided.       |
| Teleport           | Advance 10% of a lap along the track path; crossing the finish line updates laps/checkpoints. Preserve speed, reserves, and inventory; align orientation/camera to the destination. Follow the chosen branch, otherwise the main path.                                                                                                                                    | Decided.       |
| Defective Teleport | Move back 10% of a lap using the same path/state rules; crossing the starting line backwards updates laps/checkpoints accordingly.                                                                                                                                                                                                                                        | Decided.       |

Dependencies: mid-race character switching and a shared repositioning operation
that updates terrain, checkpoints, rescue state, and camera together.

[VERIFY: character resource availability and loading; player input/HUD ownership
after Body Swap; Try Something New interaction; path branches and lap validation].

## E. Time Manipulation

### Za Warudo

Humans keep driving while bots and the dynamic environment freeze, including
hazards, environmental animations, world particles, and all weapons. Newly fired
human weapons also freeze until expiry. Crates and pickups remain collectible.
Frozen objects retain collisions. The color palette changes to something highlighting
blue and yellow. The clock item sound effect plays, followed by the sound of breaking
a time crate.

Duration is the smaller of the Chaos duration and 10 seconds. Race/special-event
clocks, power-up timers, and other Chaos timers stop; only this effect's expiry
clock continues alongside human driving.

Stretch goal: defer crate-breaking animations and damage to opponents until the
effect ends.

Suspend music and world/bot sounds; human sounds remain active, along with any
dedicated Za Warudo sound effect.

[VERIFY: selective thread updates and an independent expiry clock; resumption
without accumulated catch-up time].

### King Crimson

Approximate 10 seconds of bot movement along the track path based on their speed,
without full simulation; advance the race timer by 10 seconds. Start with a
technical prototype.
The color palette changes for half a second to something highlighting
blue and yellow. The clock item sound effect plays, followed by the sound of breaking
a time crate.

Only bot movement/progression and the race timer advance. Environment, weapons,
power-ups, other states, and Chaos timers stay unchanged. Bots may finish the
race during the skip; update results and event completion accordingly.

[VERIFY: path-based advancement, lap/checkpoint consistency, and destination
placement without moving humans].

The two time effects use last-activation-wins: activating either removes the other
before applying its own behavior.

## Delivery Order

- [x] Shared instant lifecycle/eligibility + A: items and reserves.
- [ ] B: driving, turbo, and jumping.
- [ ] C: size, camera, and bots.
- [ ] D: characters and movement.
- [ ] E: time-manipulation prototypes and implementation.

Scope: all 24 effects, in the A → B → C → D → E order above.

Implement effects one at a time, following their listed order within each block.
Complete implementation, focused validation and user review of the current effect
before starting the next. Blocks group related effects; they are not batches to
implement together. Add shared infrastructure only as required by the current effect.

For each effect, verify its lifecycle/conflict rules and the relevant edge cases
listed above. Add reproducible tracks/quick states and results here:

Block A: focused runtime tests cover full timed capacity, instant notices without
docking, repeat rewards, notice overflow, pause/reset/resident restore, inventory
replacement, deployed/held orb ownership, event eligibility,
conflicts and real damage/boost paths. Visual checks of renewed mask/shield and
1P/2P announcements remain to be performed in game using `chaos_quick_race`.

Validation: the block-A game build and original 24 CTest suites passed.
After splitting the suites, `cmake --build build --target tests --parallel 4`
and `ctest --test-dir build --output-on-failure -R "^test_"` passed all 25
asset-free suites. The checks are now split between `test_chaos_runtime`,
`test_chaos_effects` and `test_warpball` (3,194 total), including
normal/juiced item sounds, gradual Wumpa reward crediting,
the ten-second reserve reward and subsequent earned reserves.
Review regressions also cover the explicit list of distinct retail race items,
item grants/publication in relic and crystal events. Wall-contact protection
now checks the recipient directly in `COLL_MOVED_ScrubImpact`, preserving both
reserves and `turbo_outsideTimer`; surface lookup uses the original shared metadata.
Reserve Injection writes its one-off reward directly. Ordinary reserve grants
retain their original behavior; only reserve losses use the Chaos freeze hook.
Orb regressions cover immediate teardown on solo/co-op crystal launches and
battle arenas, missing/allocated particle cleanup, ownership release and
ordinary checkpoint-based race launch.

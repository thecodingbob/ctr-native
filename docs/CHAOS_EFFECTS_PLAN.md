# New Chaos Effects — Implementation Plan

Source: [issue #90](https://github.com/thecodingbob/ctr-native/issues/90).
Status: block A implemented with focused test coverage. Later technical checks
remain marked with placeholders; in-game visual validation is pending.
Current effect: Classic GTA (block C), implemented for review; in-game validation pending.

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
| My Planet Needs Me | Apply one immediate upward impulse at 6× normal jump force, including while airborne; active gravity modifiers still apply.                                  |
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
- [x] B: driving, turbo, and jumping.
  - [x] Sabotage: implementation and focused tests; user review/in-game checks pending.
  - [x] This Is Fire: implementation and focused tests; user review/in-game checks pending.
  - [x] Hydraulics: implementation and focused tests; user review/in-game checks pending.
  - [x] My Planet Needs Me: implementation and focused tests; user review/in-game checks pending.
  - [x] Who Bound This?: implementation and focused tests; user approved proceeding.
  - [x] Try Something New: implementation and focused tests; user approved proceeding.
- [ ] C: size, camera, and bots.
  - [x] Small Kart Energy: implementation and focused tests; user approved proceeding.
  - [x] Classic GTA: implementation and focused tests; user review/in-game checks pending.
- [ ] D: characters and movement.
- [ ] E: time-manipulation prototypes and implementation.

Scope: all 24 effects, in the A → B → C → D → E order above.

Implement effects one at a time, following their listed order within each block.
Complete implementation, focused validation and user review of the current effect
before starting the next. Blocks group related effects; they are not batches to
implement together. Add shared infrastructure only as required by the current effect.

For each effect, verify its lifecycle/conflict rules and the relevant edge cases
listed above. Add reproducible tracks/quick states and results here:

Sabotage: appended as effect ID 22. The driver-local brake hook suppresses Square
and the reverse/braking halves of both vertical sticks before reserve loss and
gas/brake resolution. Forward input remains intact; the shared gamepad state is
unchanged. Focused checks cover grounded/airborne/reversing states, human/bot and
finished-driver scope, forward analog input and expiry. Validate in game with
`chaos_quick_race`: Square alone, Square + throttle, airborne braking, left-stick
reverse and right-stick braking/reverse. User approved proceeding to This Is Fire.
Validation: game build and the five focused catalogue, runtime, effects, save and
developer-setup suites passed.

This Is Fire: appended as timed effect ID 23 (`this-is-fire`). Existing turbo is
upgraded once at activation through the shared `VehFire_SetLevel` operation,
without reserve grants, new-boost flags or turbo-count changes. New grants pass
through `Chaos_FireLevelForDriver` in `VehFire_Increment`, using the same USF fire
level as super turbo pads. Defective Boosts and This Is Fire replace each other.
Expiry preserves earned USF; subsequent boosts and reserve losses follow normal
rules. Focused tests cover existing flame sizing, unboosted humans, bot scope,
all grant types, unchanged reserves, expiry and symmetric conflicts. All five
catalogue/runtime/effects/save/developer-setup suites passed. Game compilation
passed; linking was blocked by the running executable (`Permission denied`).
In-game checks: activate while boosting and without reserves, chain drift/jump/pad
boosts, then check that USF survives expiry until normal boost-loss rules apply.
User approved proceeding to Hydraulics.

Hydraulics: appended as timed effect ID 24 (`hydraulics`). `Chaos_ApplyDriver`
renews a one-physics-step normal hop request, including while drifting, without
altering the pad or synthesizing shoulder input. The normal jump/friction path
retains coyote-time, cooldown, existing forced jumps and normal jump force;
gravity modifiers still apply. No Hopping and Hydraulics replace each other.
Focused checks cover request renewal in driving/drifting/V-shift recovery,
unchanged physical timing/force, genuine jump input, cloud lockout, damage states,
human/bot/finished-driver scope, expiry and symmetric conflicts. All five focused
catalogue/runtime/effects/save/developer-setup suites passed. Game compilation
passed; linking was blocked by the running executable (`Permission denied`).
In-game checks: continuous hopping on flat ground without shoulder input,
landing/cooldown timing, hops while drifting, slopes, jump-lockout clouds and
Low Gravity/Heavy Karts. User approved proceeding to My Planet Needs Me.

My Planet Needs Me: appended as instant effect ID 25 (`my-planet-needs-me`).
Adds an upward impulse of six times the current normal jump force to existing
momentum, preserving horizontal velocity. Commits through the existing speed/vector
conversion and updates working velocity as well; no normal-hop cooldown or
coyote-time requirement. Removes No Hopping/Hydraulics before launching; gravity
effects remain active. Uses the instant announcement queue, without a timed slot.
Focused checks capture the vector passed to the speed conversion, covering ground
and airborne launches, horizontal momentum, one-shot/repeated activation, inventory
and reserves, full timed capacity, conflicts, gravity modifiers and driver scope.
The full speed/angle conversion and trajectory remain in-game validation.
All five focused catalogue/runtime/effects/save/developer-setup suites passed.
Game compilation passed; linking was blocked by the running executable
(`Permission denied`). In-game checks: ground launch, rising/falling activation,
slopes, repeated launches and Low Gravity/Heavy Karts. Review this effect before
starting Who Bound This?.
In-game review: the original 8× impulse exceeded signed 16-bit speed storage with
the retail jump force of 4,596 and reversed the kart. Reduced to 6× for now at
user request, preserving the existing physics layout. The launch-vector test now
uses that retail force; in-game retesting remains pending.

Who Bound This?: appended as timed effect ID 26 (`who-bound-this`). Each human
receives a stable nonidentity permutation of throttle, brake, item and both hop
buttons. Local pad copies cover linear driving and both drift-update paths;
analog throttle/reverse follow the permutation with coherent held/tap/release
edges. Shared input, steering, pause and menu controls remain unchanged.
`Chaos_ApplyInputRules` now also owns Inverted/Wobble Steering and Sabotage;
No Hopping/Hydraulics remain after normal jump-input resolution. The game build
and five focused Chaos suites passed. User approved proceeding to Try Something New.

Try Something New: appended as instant effect ID 27 (`try-something-new`), with
the resulting class lasting until race end. Picks independently from the other
three standard classes, excluding each driver's current class on every repeat.
`VehBirth_Consts.c` shares the existing table loader between ordinary vehicle
birth and class changes, including configured multipliers. Runtime stores the
selected class for subsequent picks and resident snapshots; driver constants
retain the actual class after the notice expires. Characters, shared metadata,
position, current momentum, inventory and reserves remain intact. Focused checks
use the real class loader for class selection, repeated changes, modifiers,
scope, persistence and next-race restoration. Game build and all six focused
Chaos/vehicle-spawn suites passed. In-game checks: repeat class changes in 1P/2P,
handling changes while drifting, concurrent speed/gravity modifiers and retry.
Review this effect before starting Small Kart Energy in block C.
Engine audio now follows the selected class immediately, including repeat
changes, ongoing pitch/volume updates and warp stop/restart. Reset restores the
character's engine audio. The game rebuild and six focused suites passed after
this correction; the effects suite performs 2,862 checks with zero failures.
In-game validation should also confirm the audible transition on activation.

Small Kart Energy: appended as timed effect ID 28 (`small-kart-energy`). Human
kart dimensions and contact radii use 0.5× scale; forward speed targets and jump
forces use 0.7×, following Giant Karts' unchanged-reverse convention. Giant Karts
and Small Kart Energy replace each other. `Driver.kartScale` is now a float
multiplier, supporting arbitrary fractions. `VehPhysForce_GetKartScale` shares
the zero/default and finished-driver rules between rendering/collision scaling
and size comparisons; integer dimensions round toward zero only at consumption.
Existing size-based squash handles larger bots/humans with the normal damage
duration, mask/shield protection and immunity. Focused checks cover fractional
dimensions, speed/jump, scope, protection, repeated contact, conflicts, pause,
expiry, finished drivers and reset. Validate in game in 1P/2P: body/tire size,
walls and kart contacts, bot squash, protected contact, jumps, Giant replacement
and retry. Review this effect before starting Classic GTA.
Validation: game build and all six focused Chaos/vehicle-spawn suites passed
after switching `kartScale` to float (1.0 retail, 0.5 small, 2.0 giant).
Size follow-up: ground-shadow footprint and newly emitted tire skid positions,
widths and lateral offsets now share `VehPhysForce_ScaleValue` for both size
effects. Stored world-space skid history is not rescaled. Game rebuild passed;
visually check shadows and front/rear skids at 0.5×/1×/2×, including changes of
size while drifting and airborne shadow shrinkage.
Visual-size audit: exhaust and damage smoke, turbo flames, terrain and impact
particles, water wake/splashes, mask orbit/beam, shield layers and body-scale
resets now use the shared kart multiplier. Particle color, rotation and timing
remain unchanged; emitted particles keep their emission scale. TNT keeps its
original size at user request, with only its head attachment offset adjusted.
Focused regression checks cover particle size/growth/motion, line endpoints,
spatial vs. color oscillators, effect matrices and finished-driver defaults.
All six focused suites passed. Compilation succeeded; executable linking was
blocked by the running game. Visual validation remains pending for these paths.

Classic GTA: appended as timed effect ID 29 (`classic-gta`). Each unfinished
human gets a vertical camera 2,560 world units above their own kart, with heading
following the kart and zero roll. The retail camera continues updating; the
final presentation override leaves mode, zoom preference and projection intact.
There is no pose snapshot to restore at expiry. Visibility uses the driver's
`underDriver`/contact block, with a driver-position probe only when neither is
available, so tunnels do not move the eye down. Overhead level geometry bypasses
ground-level leaf/face PVS masks and uses camera frustum culling instead, keeping
road beyond crests visible. Visibility source caches are invalidated so retail
PVS is decoded again at expiry even without changing ground block.
Shared level-render workers classify ceiling blocks by the winding of their
high-detail triangles, using the same topology as collision. Downward-facing
blocks are omitted throughout the viewport; blocks containing any upward-facing
triangle, vertical panels and supporting/contact blocks are retained. The
classification does not depend on moving X/Z bounds or a driver-height cutoff.
Static non-threaded models covering the kart are omitted
when their projected bounds extend above the cutaway plane. This is a whole
block/model cutaway, not polygon clipping; mixed road/ceiling blocks are retained
to preserve pavement. Collision data and global visibility flags are untouched.
The cutaway persists under pause, ends for finished humans and clears on
expiry/reset. Focused checks cover independent 2P poses/headings, ground-block
visibility selection, local roof/model cutaways, protected gameplay objects,
pause, expiry and reset. Tune height and cutaway in game on Roo's Tubes, Sewer
Speedway and Cortex Castle, in 1P/2P, including ramps, stacked roads, jumps,
damage/rescue, Giant/Small Karts, pause/resume and effect expiry. Review Classic
GTA before starting Arena Champions.
Validation: game build and all five focused Chaos suites passed. In-game camera
height, cutaway boundaries and transitions remain pending user review.
User-review follow-up: doubled camera height to widen the road view, removed
ground-level PVS restrictions for overhead geometry, and confined the cutaway to
blocks whose X/Z bounds actually cover the kart. Regression checks cover uphill
and downhill road, unflagged supporting ramps, upper roads directly overhead,
viewport-local visibility masks and cache invalidation. Visual recheck pending.
Follow-up validation: game build passed; Chaos runtime (526 checks) and effect
suite (2,948 checks) passed with zero failures.
Second visual-review follow-up: residual floor popping in Cortex Castle and
ceiling popping in Roo's Tubes exposed the moving bbox/height cutaway. Replaced
it with high-detail face-orientation classification, independent of kart position
and ground flags. Regression coverage includes crossing tile boundaries, steep
unflagged pavement, folded/mixed blocks, vertical panels and triangular blocks
with unused vertex slots. Runtime/effect tests pass; game source compilation
passes, but executable linking is blocked while the game is running. Visual
validation on the two reported sections remains pending.

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

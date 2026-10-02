---
name: game-domain-knowledge
description: Recall what Crash Team Racing actually is — its modes, adventure structure, characters, boost physics, items, tracks, and known retail quirks — before judging whether a change is faithful. Conceptual and player-facing; not a map of the code.
---

# Crash Team Racing domain knowledge

Use this skill when a task depends on **what the game is supposed to do** rather than
where the code lives: is a mechanic correct, is an unlock gate right, is a boost tier
real, does this mode have this feature at all. Code ownership lives in the other
skills; this one is the shared factual baseline.

Default to the **1999 PlayStation release (PS1 CTR)**. Where later games differ, the
differences are called out, because importing *Nitro-Fueled* or *Crash Nitro Kart*
rules into PS1 behaviour is the most common way to get "faithful" wrong.

## What the game is

*CTR: Crash Team Racing* (1999) is Naughty Dog's fourth Crash game and its last,
published by Sony Computer Entertainment for PlayStation. It is a kart racer in the
`Mario Kart` / `Diddy Kong Racing` lineage that critics generally placed above both on
controls, track design, and technical execution (Metacritic 88). It shipped to great
commercial success and is remembered mainly for two things: a deep, skill-expressive
**boost system**, and **Adventure**, a hub-and-boss campaign that reads like a
platformer's world map.

Premise: the alien **Nitros Oxide** intends to "speed up the whole world" (or turn
Earth into a parking lot) and must be beaten in a race.

Structural facts worth keeping in mind:

- **Four-player split-screen** with a multitap.
- The engine was built in parallel with *Crash Bandicoot: Warped* and shares its
  look. **PS1 memory limits shaped the design**: Oxide was never playable, Polar and
  Pura were originally one character and got split, and Komodo Moe was cut.
- To fit up to 64 kart tires in four-player split-screen, programmer Greg Omi
  rendered tires as **camera-facing 2D sprites** rather than geometry.

## Modes

Five modes in PS1 CTR. Everything else in this file hangs off these.

| Mode           | Shape                                                                                                                                       |
|----------------|---------------------------------------------------------------------------------------------------------------------------------------------|
| **Adventure**  | Single-player campaign. Hub worlds, warp pads, trophies, boss races, collectibles. The bulk of the game.                                    |
| **Time Trial** | Race the clock on one unlocked track, set a personal best, race a **ghost** of that best.                                                   |
| **Arcade**     | Single races or **cups** of four tracks, scored by finishing position, highest total wins. Also the unlock source for hidden Battle arenas. |
| **Versus**     | Multiplayer on a chosen track or cup.                                                                                                       |
| **Battle**     | 2–4 player arena combat with weapons. Customisable time limit, kill/score limit, weapon selection, and teams vs free-for-all.               |

Notes that trip people up:

- **Relic Race, CTR Challenge, and Crystal Challenge are not top-level modes.** They
  are Adventure event types unlocked per-track inside hubs.
- **Pick-ups are unavailable in Time Trial and Relic Races.** In Relic Races the
  normal `?` and Bounce crates are replaced by yellow **Time Crates**; Time Trial is
  simply item-free. Battle requires at least one weapon to be obtainable before a
  match can start.
- PS1 CTR has **no difficulty selector** for Adventure. That arrived with
  Nitro-Fueled.

### Arcade cups

| Cup         | Tracks                                                   |
|-------------|----------------------------------------------------------|
| **Wumpa**   | Crash Cove, Tiger Temple, Blizzard Bluff, Coco Park      |
| **Crystal** | Roo's Tubes, Dingo Canyon, Dragon Mines, Sewer Speedway  |
| **Nitro**   | Mystery Caves, Papu's Pyramid, Cortex Castle, Tiny Arena |
| **Crash**   | Polar Pass, N. Gin Labs, Hot Air Skyway, Slide Coliseum  |

Beating all four cups on every difficulty setting unlocks three hidden Battle arenas
(**Parking Lot**, **The North Bowl**, **Lab Basement**).

## Adventure structure

Four **hub worlds**. In each you drive around freely, and **Warp Pads** — marked by a
flashing dot on the minimap — launch a three-lap race against seven CPU rivals. A
first-place finish earns a **Trophy**. The number floating above a Warp Pad is how
many Trophies it needs to activate.

| Hub                  | Tracks                                                               | Boss              | Boss track     |
|----------------------|----------------------------------------------------------------------|-------------------|----------------|
| **N. Sanity Beach**  | Crash Cove, Roo's Tubes, Mystery Caves, Sewer Speedway, Skull Rock   | Ripper Roo        | Roo's Tubes    |
| **Gem Stone Valley** | Slide Coliseum, Turbo Track                                          | N.Oxide           | Oxide Station  |
| **The Lost Ruins**   | Coco Park, Tiger Temple, Papu's Pyramid, Dingo Canyon, Rampage Ruins | Papu Papu         | Papu's Pyramid |
| **Glacier Park**     | Polar Pass, Tiny Arena, Dragon Mines, Blizzard Bluff, Rocky Road     | Komodo Joe        | Dragon Mines   |
| **Citadel City**     | Cortex Castle, N. Gin Labs, Hot Air Skyway, Oxide Station            | Pinstripe Potoroo | Hot Air Skyway |

The two **Gem Stone Valley** tracks are bonuses unlocking as follows: **Slide Coliseum** 
(10 Relics) and **Turbo Track** (all five Gems). 
A boss is one-on-one and awards a **Boss Key**; four Trophies
in a hub opens its Boss Garage, and Keys open Area Doors to the next hub (some doors
want two Keys). Nitros Oxide's two Challenges and the final race live in Gem Stone
Valley.

**Collectible currencies and what opens them:**

| Currency      | How you get it                                                                                                           | What it opens                                                |
|---------------|--------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------|
| **Trophy**    | Win any track race in 1st                                                                                                | Warp Pads; four per hub opens the boss                       |
| **Boss Key**  | Beat a hub's boss                                                                                                        | The next Area Door                                           |
| **CTR Token** | CTR Challenge: collect the letters **C, T, R** on track *and* finish 1st. Five colours: Red, Blue, Green, Yellow, Purple | Purple is the exception — it is the Crystal Challenge reward |
| **Relic**     | Relic Race: solo three laps, beat the target time. Sapphire / Gold / Platinum tiers                                      | Collect enough Relics to enable Slide Coliseum               |
| **Crystal**   | Crystal Challenge bonus round: collect **20 crystals** before the timer expires                                          | Purple CTR Token                                             |
| **Gem**       | Gem Cup: four tokens of one colour unlock it; win the four-race tournament                                               | Turbo Track; all five unlock Turbo Track                     |

A **100% completion** requires all Trophies, Keys, Relics, Tokens, Gems, and then
beating Nitros Oxide. 

### Gem Cups

Held in the Gem Stone Valley hub. Four races, points by finishing position
(1st=9, 2nd=6, 3rd=3, 4th=1, 5th-8th=0), highest total wins.

| Cup        | Tracks                                                                                        | Unlocks character |
|------------|-----------------------------------------------------------------------------------------------|-------------------|
| **Red**    | Crash Cove, Mystery Caves, Blizzard Bluff, Papu's Pyramid                                     | Ripper Roo        |
| **Green**  | Roo's Tubes, Coco Park, Polar Pass, Cortex Castle                                             | Papu Papu         |
| **Blue**   | Tiger Temple, Sewer Speedway, Dragon Mines, N. Gin Labs                                       | Komodo Joe        |
| **Yellow** | Dingo Canyon, Tiny Arena, Hot Air Skyway, Oxide Station                                       | Pinstripe Potoroo |
| **Purple** | Roo's Tubes, Papu's Pyramid, Dragon Mines, Hot Air Skyway — each boss on their own home track | Fake Crash        |

### Relic Races

Relic tiers are Sapphire, Gold, Platinum. Time Crates freeze the clock for 1, 2, or 3
seconds, and breaking **all** of them gives a **ten-second** bonus. Platinum times are
the hard target.

## Characters and stats

Fifteen characters, **eight selectable from the start**.

**Start:** Crash Bandicoot, Coco Bandicoot, Tiny Tiger, Dr. Neo Cortex, N. Gin,
Dingodile, Polar, Pura.

**Unlockable:** Ripper Roo (Red Gem Cup), Papu Papu (Green Gem Cup), Komodo Joe (Blue
Gem Cup), Pinstripe Potoroo (Yellow Gem Cup), Fake Crash (Purple Gem Cup), Dr. N. Tropy
(beat all of his Time Trial times), Penta Penguin (Cheat code).

In PS1 CTR a character's **kart and engine are fixed to the character**; there is no
engine swapping. So character choice *is* class choice. Nitro-Fueled decouples them
(you pick engine separately and can drive any character in any class), which is why
modern advice like "characters don't matter, only the class does" does not describe the
PS1 game.

Three stat axes drive everything:

- **Speed** — top speed reachable on throttle alone.
- **Acceleration** — time to reach base speed from standing.
- **Turn** — how tightly the kart can turn.

## Kart physics and the boost system

This is the heart of the game.

**Controls.** Accelerate, brake/reverse, steer (digital d-pad or analog), fire the
held item, and hop. Both shoulder buttons are interchangeable for hop/power-slide, and
**pressing the *opposite* shoulder button fires the boost**. Analog throttle is
supported alongside digital buttons.

**Power slide (the core skill expression).**

1. Hold a shoulder button to hop, steer in the air, land sideways.
2. While sliding, the **Turbo Boost Meter** in the lower-right fills green to red and
   the exhaust smoke darkens.
3. Tap the *other* shoulder button while the meter is in the red to fire a mini-turbo.
4. Up to **three** boosts per drift; the third is stronger than the first two.

Failure states: pressing too late **back-fires** and kills the drift's remaining
boosts; sliding too long **spins out**. A full meter fires a stronger boost than a
partially filled one, so timing matters more than duration.

**Other boost sources.** Hang-time boosts from jumps scale with airtime, and are the
weakest per-hit. Turbo pads are fixed-strength. Revving the accelerator just before the
lights start gives a starting boost. Landing a jump into a power slide ("mid-air") is a
named advanced technique.

**Reserves and boost stacking.** Reserves are stored boost energy. A boost's duration
lasts as long as reserves remain, and reserves are banked by power-sliding. The bar
fills linearly in time but the reserve value gained from firing scales strongly
non-linearly with how full it is — so firing early is disproportionately wasteful.
Advanced play is *chaining* boosts to bank enough reserves for one long unbroken
turbo. U-turns spend all reserves.

**Fire tiers.** Community reverse-engineering (see TASVideos below) names the visual
boost states:

| Tier                           | Typical source                                                                                                                                                                      |
|--------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Green** (1 or 2 mini-turbos) | 1-2 chained mini-turbos, or a short jump                                                                                                                                            |
| **Yellow**                     | 3 chained mini-turbos, or a mid-length hang-time jump                                                                                                                               |
| **Sacred fire**                | A turbo pad, or a long hang-time jump                                                                                                                                               |
| **Ultimate sacred fire (USF)** | Only from Super Turbo Pads. Lasts while reserves hold, and only survives if you take no other fire type and drop no reserves. Super pads appear in the Gem Stone Valley hub tracks. |

These thresholds are approximate and region-sensitive; treat them as a mental model,
not a spec.

**Wumpa Fruit.** Each of the first nine fruit adds a small fixed amount to the stored
speed value — roughly +30 units on a 16-bit field, about +2.79% total at nine — and the
tenth converts your held item into its **juiced** form rather than adding speed. Some
speed is lost when you take a hit.

**Speed ghosts (SG).** A separate, temporary speed increase that is *not* scaled by
fire level, obtained from downhill momentum. Edge cases and half-pipes can produce
them; PAL's slightly stronger SG behaviour is tied to its jump-timing difference.

**Input quirk.** CTR samples input only on drawn frames, so rapid hop mashing is
frame-quantised ("froggy"). Landing with the meter at or below ~0.16 briefly locks out
another hop.

## Items and weapons

Collected by smashing `?` crates, which run a **slot-reel** animation; pressing fire
during the reel locks the result early.

**Weapons:** Tracking Missile, Bowling Bomb, Power Shield, Explosive TNT Crate,
N. Brio's Beaker, Aku Aku / Uka Uka Mask, Turbo, N. Tropy Clock, Warp Orb.
**Battle-only:** Super Engine, Invisibility.

The reel is position-gated: leading racers get defensive items (Beakers, TNT, Power
Shields), trailing racers get the strongest offensive ones (Warp Orbs, Masks, N. Tropy
Clocks). Being in first can override the box's normal result.

**Juiced forms** (ten Wumpa held): TNT Crate becomes an instant-detonate **Nitro**
Crate; Beaker turns red and additionally slows and rerolls the victim's held item;
Power Shield turns blue and persists until it absorbs a hit; Missile gains speed and
tracking accuracy; Warp Orb hits **every** racer ahead instead of just the leader;
Masks, N. Tropy Clock, Turbo, and Super Engine last longer.

Useful asymmetries that define good play: a Tracking Missile is survivable with a
TNT/Beaker/Bowling Bomb dropped behind you just before impact, or by cutting a tight
corner into a wall. Warp Orb is near-worthless in first and devastating in last.

**Wumpa crates** (Bounce / slatted crates) carry 3-7 fruit each. They are removed in
Relic Races along with `?` crates.

## Tracks

**N. Sanity Beach** — Crash Cove (Crash's home, gentle opening), Roo's Tubes (underwater
figure-eight tunnel), Mystery Caves (fiery cavern, flame pits), Sewer Speedway (rolling
barrels). **The Lost Ruins** — Coco Park (easy, flat, few jumps or pads), Tiger Temple
(flame-spitting statues), Papu's Pyramid (sharp turns, carnivorous plants), Dingo
Canyon (armadillos cut you off). **Glacier Park** — Polar Pass (icy patches),
Tiny Arena, Dragon Mines, Blizzard Bluff.
**Citadel City** — Cortex Castle, N. Gin Labs, Hot Air Skyway (balloon-lifted),
Oxide Station. **Bonus** — Slide Coliseum (a pure power-slide test, no turbo pads),
Turbo Track (pad-heavy, long straights).

Tracks are characterized by **shortcuts and hazards** as much as layout. Understanding 
shortcuts and turning technique is what Relic Races actually test.

## Known retail quirks and glitches

Treat these as load-bearing. Several are the reason certain code exists at all, and
"fixing" them is a behaviour change, not a bug fix.

- **Lap validation and the No Man Zone (NMZ).** A lap only counts if you pass three
  checkpoint regions in order: just after the start line, the main body, then just
  before the finish. Skipping straight to the finish does not count. The **NMZ** is a
  no-checkpoint area still technically on track; landing there skips progression
  without penalty. This is the basis of the start-line glitch.
- **Progression value overflow ("starline" glitch).** Driving out of bounds overflows
  an internal progression counter. Present on all tracks in NTSC-U; later versions
  patched it. Both PAL and NTSC-J carry it.
- **Speed ghost edge cases.** Road edges and some Sewer Speedway half-pipes yield SG
  boosts, cause not fully explained even by the original team.
- **U-turn.** Down + brake + steer is an extremely sharp turn that spends all
  reserves. Intended enough to be a named technique, but it dominates corners when
  combined with froggy hops.
- **PAL language-skip glitch.** Selecting the (unimplemented) Japanese voice option on
  PAL, where no matching `.XA` exists, corrupts the boss-head array during cutscenes
  and swaps speaking heads.
- **Memory card saves can corrupt**, particularly late-game with many Relics
  collected. Anecdotally reported on emulated memory cards; worth keeping in mind when
  a save path is being changed.

Also relevant when reading retail-era expectations: cheat/poke codes exist for
unlocking everything, infinite turbos, permanent Super Engine, super turbo pads, a
turbo counter, and unlocking Turbo Track and the bonus arenas.

## How to use this

- **Check the source tag before trusting a number.** Retail values come from the
  instruction booklet and encyclopaedic wikis. The fire-tier thresholds, Wumpa
  arithmetic, frame-lockout value, and checkpoint order come from community
  reverse-engineering and are approximate; cross-check against the actual game data
  before hard-coding anything from them.
- **Default to PS1 CTR.** Anything from Nitro-Fueled or Crash Nitro Kart is a
  deliberate divergence and needs saying out loud, not a silent import.
- **Separate "the game does this" from "this feels wrong".** A retail quirk you dislike
  is still behaviour to reproduce unless the user asked to change it.
- **A mechanic existing in the game does not mean it exists in every mode.** Check the
  mode before assuming items, tokens, cups, or collectibles are available.
- **HUD is part of the domain.** The Turbo Boost Meter, warp-pad Trophy counters,
  minimap dots, and gem/relic/token displays are canonical UI, not decoration.

## Sources

- [Wikipedia: Crash Team Racing](https://en.wikipedia.org/wiki/Crash_Team_Racing) — release
  facts, modes, development history, controls, technical achievements.
- [Bandipedia: CTR: Crash Team Racing](https://crashbandicoot.fandom.com/wiki/CTR:_Crash_Team_Racing)
  — characters, adventure track/requirement tables, cups, cheats, version differences.
- [TASVideos: Crash Team Racing game resources](https://tasvideos.org/GameResources/PSX/CrashTeamRacing)
  — the community reverse-engineering pass. Best public source for fire tiers, reserves,
  Wumpa arithmetic, lap-checkpoint validation and the NMZ, and regional jump timing.
  Partially incomplete; it flags its own open questions.
- [IGN: Relic Races Explained](https://www.ign.com/wikis/crash-team-racing-nitro-fueled/Relic_Races_Explained)
  and [Adventure Mode Explained](https://www.ign.com/wikis/crash-team-racing-nitro-fueled/Adventure_Mode_Explained)
  — Adventure event structure and relic/crate rules.
- [StrategyWiki: Crash Team Racing](https://strategywiki.org/wiki/Crash_Team_Racing) —
  per-track descriptions, shortcuts, and the weapons/powerups table.
- [Activision: Power-Up Planning](https://blog.activision.com/crash-bandicoot/2019-06/Crash-Team-Racing-Nitro-Fueled-Power-Up-Planning)
  and [Adventure Mode announcement](https://blog.activision.com/crash-bandicoot/2019-05/Announcement-Details-of-Crash-Team-Racing-Nitro-Fueled-Adventure-Mode-Revealed)
  — official item and progression behaviour.
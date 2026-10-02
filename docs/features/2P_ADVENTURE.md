# Multiplayer Adventure (2P co-op)

The two players are one competitor with two karts: no versus, no winner and no loser, 
and every round produces a single shared result.

## Starting a 2P file

Set `multiplayer_adventure = true` in `config.ini`, or flip it in the in-game Options
Config menu. It is tied to the extended character selector: turning one on turns the
other on, and turning `extended_character_select` off turns co-op back off.

Pick **New Adventure** as usual. The setup screen now has a player count row — **1
Player** for a normal solo Adventure, or **2 Players** for co-op, which only unlocks
when two controllers are joined. Both players then pick their own character.

The choice is stored on the save slot, so loading that file returns you to the same
player count. Saves created before this option existed are never rewritten; they load
as solo Adventure.

The hub stays single-player. Player 1 picks the next round as usual.

## Winning a round

**Trophy, CTR token and boss races.** Won only when both humans finish **first and
second, in either order**. Finishing 1st and 3rd loses the round for both of you. In a
boss race that means coming home ahead of the boss.

**CTR token races.** The letters are pooled — every C, T and R either of you picks up
counts toward the one set of three.

**Crystal challenge.** The crystals are pooled too, and either of you picking one up
moves the shared counter. Because two racers share one clock, the time limit is 60% of
what a solo attempt gets.

**Relic race.** To make one clock fair for two racers, the tier targets are 30% shorter 
and the pair is graded on its **slower** racer, so both of you have to be quick enough 
for the relic to be awarded. Time crates are pooled: one shared counter that moves when 
either of you picks one up, and collecting every crate still gives the 10-second bonus. 

**Cups.** Won when the two humans hold the **first and second aggregate places**, in
either order, at the end of the final round. 
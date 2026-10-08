# Controls

The **Controls** screen decides which device each of the four
players uses, and lets you rebind every button and stick direction on it.
Changes apply immediately and are remembered between sessions.

## Opening the screen

Options → **Config** → **Controls**. The title reads `CONTROLS P1` through
`CONTROLS P4`, so you always know whose controls you are looking at.

![The Controls screen for player 1: L1 and R1 hints flank the CONTROLS P1 title, a DEVICE row
shows the assigned controller, one row per button lists its current binding, and Restore Defaults
sits at the bottom](images/controls-menu.jpg)

## Choosing a device for each player

By default, every player is on **Automatic**: player 1 takes the first gamepad
plugged in, players 2 to 4 follow in order, and the keyboard is the last
fallback. You do not need to touch this for most setups — plug a controller in
and whoever was waiting for one gets it. Assignment is re-sorted whenever a
controller is plugged in or unplugged, including in the middle of a race.

To take control of the assignment yourself, open **DEVICE** with `Cross`. A **SELECT DEVICE** popup lists:

1. every device no other player is using
2. **Automatic** — resolve against whatever is free
3. **Disabled** — players 2 to 4 only

A controller already assigned to another player is not offered in your list.

Player 1 can never be disabled. If player 1's controller disappears mid-session,
the keyboard moves to player 1 and the player who had it drops back to
**Automatic**.

## Rebinding a control

Press `Cross` on any binding row to start listening. That row turns red and shows
`...`, and the control stops working while you decide.

1. Let go of everything on that device first. The game waits for it to go idle,
   so the press that opened the row is never taken as the new binding.
2. Press the key or button you want instead. The proposed binding appears in red.
3. Let go of it, then press `Cross` to confirm. The prompt shows the Cross icon
   for this step.

`Triangle` or `Start` gives up on it: the previous binding comes back and the screen
goes back to the section list. Doing nothing for a few seconds reverts the row as
well. While a row is listening, that device stops  driving the menu, so a press meant 
for one control cannot leak into another.

The screen only listens to the device you are editing: rebinding on the keyboard
ignores your gamepad, and the other way round. A confirmed change is saved
straight away.

Bindings belong to the device, not to the player: rebinding `L1` on a controller
changes it for whoever that controller is assigned to. **Restore Defaults** puts
every binding on that device back to the shipped layout — the keyboard's
`C`/`V`/`Z`/`X` face buttons, arrows, `Enter` and `Space`, or the standard gamepad
layout for a controller.

Two identical controllers share one layout, because the game identifies them the
same way. Restoring defaults on one of them restores both.

Nothing stops you putting the same key on two controls. If a press then does two
things at once, that is the cause — reassign one of them.

## Stick deadzone

An analog stick that rests off centre steers on its own, and
**DEADZONE** is the answer to that: `Left`/`Right` change it in steps of 5
percent, and each change is saved straight away.

The figure is how much of the stick's travel to ignore around centre, from 0 to 90. **0 is the console's own behaviour** and trims nothing, so leave it there
unless your stick actually drifts. 

Like the bindings, the figure belongs to the device rather than the player, so
it is shared by two identical controllers, and `Restore Defaults` puts it back to 0. The keyboard has no axes, so the row is not shown for it.

## Editing someone else's controls

`L1` and `R1` on player 1's device switch between players, and the cursor jumps
back to the top of the list.

Everything else on the screen answers to player 1's device *or* the selected
player's own, so the second player can rebind their own pad without player 1
handing it over. The player switch stays on player 1's shoulders on purpose.

A player set to **Disabled** has no device, so only player 1 can reach the screen
for them — which is enough to give them a device back.

## `controls.ini`

Your bindings and player assignment are kept in `controls.ini`, next to
`config.ini`, and rewritten after every change. Values are readable names rather
than numbers, so the file can be edited by hand:

```ini
[Assignment]
player1 = auto
player2 = auto

[device keyboard]
name = Keyboard
cross = C
l2 = Left Ctrl

[device 03000000c0400000e11000011010000]
name = Xbox 360 Controller
cross = A
l2 = Left Trigger
deadzone = 15
```

Keyboard values are key names such as `C`, `Space` or `Left Shift`. Controller
values are button and axis names as your driver reports them, and `none` clears a
control. A key or value the loader does not recognize is skipped, leaving that
one control at its default rather than breaking the rest of the file.

`auto` lets a player resolve itself, `disabled` takes a player out, and any other
value pins that player to the named device for as long as it stays plugged in.
Controllers are matched by their hardware ID, so a pad keeps its layout across
replugs and USB ports.

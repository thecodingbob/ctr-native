#include <common.h>

#include <platform/native_controls.h>

#include <stdio.h>
#include <string.h>

// Frame, row highlight and the option-row mechanics are shared with the section list.
// game/230.c includes MM_ConfigMenu.c first, so these definitions are already in the
// translation unit; they are redeclared here to make the dependency explicit.
static void Config_DrawFrame(uint32_t *ot);
static void Config_DrawRowHighlight(struct GameTracker *gGT, uint32_t *ot, int y);
static bool Config_OptionStepDue(int held, int direction);
static bool Config_OptionStep(int *value, int direction, int min, int max, int step);
static void Config_DrawPercent(int percent, int y, uint32_t *ot);

// Row geometry comes from MM_ConfigMenu.c, which every screen in this menu shares.

// Device picker popup. Width is measured from the longest entry rather than fixed,
// bounded so a long controller name cannot push the box off screen or off the frame.
// The frame itself spans 0x10 to 0x1F0, so this is the usable width minus a margin.
#define CONTROLS_POPUP_ROW_SPACING  0x0E
#define CONTROLS_POPUP_TITLE_H     0x0E
#define CONTROLS_POPUP_PAD_TOP     0x08
#define CONTROLS_POPUP_PAD_BOTTOM  0x08
#define CONTROLS_POPUP_PAD_X       0x0C
#define CONTROLS_POPUP_MAX_WIDTH   0x1A0
#define CONTROLS_POPUP_CENTER_Y     0x7E

enum ControlsRowKind
{
	CONTROLS_ROW_DEVICE = 0,
	CONTROLS_ROW_BINDING,
	CONTROLS_ROW_DEADZONE,
	CONTROLS_ROW_RESTORE,
};

struct ControlsRow
{
	int kind;
	int binding;
};

// One entry in the device picker: a device to pin, plus the two preference states
// that are not a device. Automatic is what leaves a player resolving itself, and
// Disabled is only offered from player 2 onwards.
struct ControlsChoice
{
	int device;
	int pref;
};

// Rebuilt when the picker opens, so hotplug cannot leave it offering a stale device.
static struct ControlsChoice s_choices[NATIVE_CONTROLS_MAX_DEVICES + 2];
static int s_choiceCount;
static int s_choiceSelected;
static bool s_pickerOpen;

// Which player the list belongs to. Persistent so the menu reopens where it was.
static int s_player;

// Row cursor is menu->rowSelected; the scroll origin is ours.
static int s_scroll;

// Rebuilt every frame from the selected player's device, because the device can
// change under the cursor at any time via hotplug or the row above.
static struct ControlsRow s_rows[NATIVE_CONTROLS_BIND_COUNT + 3];
static int s_rowCount;

static char s_valueBuffer[80];
static char s_titleBuffer[32];
// Filtering never grows a string, but a device name is driver-supplied and up to
// NATIVE_CONTROLS_NAME_LENGTH, so this is sized well past the longest label it carries.
static char s_safeBuffer[2 * NATIVE_CONTROLS_NAME_LENGTH];

// The keyboard has no analog axes, so those rows are hidden for it rather than
// shown as unbound. Listed explicitly instead of testing the enum range, so
// reordering the binding enum cannot silently change what is hidden.
static const int s_axisBindings[] = {
    NATIVE_CONTROLS_BIND_AXIS_LEFT_X,
    NATIVE_CONTROLS_BIND_AXIS_LEFT_Y,
    NATIVE_CONTROLS_BIND_AXIS_RIGHT_X,
    NATIVE_CONTROLS_BIND_AXIS_RIGHT_Y,
};

static bool Controls_BindingIsAxis(int binding)
{
	for (unsigned int i = 0; i < sizeof(s_axisBindings) / sizeof(s_axisBindings[0]); i++)
	{
		if (s_axisBindings[i] == binding)
		{
			return true;
		}
	}

	return false;
}

static bool Controls_DeviceHasAnalog(int device)
{
	return (device != NATIVE_CONTROLS_NO_DEVICE) && (NativeControls_GetDeviceKind(device) == NATIVE_CONTROLS_DEVICE_GAMEPAD);
}

static void Controls_BuildRows(void)
{
	const int device = NativeControls_GetPlayerDevice(s_player);
	const bool hasAnalog = Controls_DeviceHasAnalog(device);

	s_rowCount = 0;

	s_rows[s_rowCount].kind = CONTROLS_ROW_DEVICE;
	s_rows[s_rowCount].binding = -1;
	s_rowCount++;

	for (int binding = 0; binding < NATIVE_CONTROLS_BIND_COUNT; binding++)
	{
		if (!hasAnalog && Controls_BindingIsAxis(binding))
		{
			continue;
		}

		s_rows[s_rowCount].kind = CONTROLS_ROW_BINDING;
		s_rows[s_rowCount].binding = binding;
		s_rowCount++;
	}

	// Only for a device with axes to trim: the keyboard's stick rows are hidden above, so
	// a deadzone there would be a figure that changes nothing.
	if (hasAnalog)
	{
		s_rows[s_rowCount].kind = CONTROLS_ROW_DEADZONE;
		s_rows[s_rowCount].binding = -1;
		s_rowCount++;
	}

	s_rows[s_rowCount].kind = CONTROLS_ROW_RESTORE;
	s_rows[s_rowCount].binding = -1;
	s_rowCount++;
}

// Fills the picker with what the player could switch to: every device nobody else
// owns, then Automatic, then Disabled for players 2 onwards. Player 1 has no
// Disabled entry, matching NativeControls_SetPlayerPref refusing it.
//
// The cursor starts on whatever the player currently holds, so opening and closing
// without moving leaves things as they were.
static void Controls_BuildChoices(void)
{
	int available[NATIVE_CONTROLS_MAX_DEVICES];
	const int count = NativeControls_GetAvailableDevices(s_player, available, NATIVE_CONTROLS_MAX_DEVICES);
	const int currentDevice = NativeControls_GetPlayerDevice(s_player);
	const int currentPref = NativeControls_GetPlayerPref(s_player);

	s_choiceCount = 0;
	s_choiceSelected = -1;

	for (int i = 0; i < count; i++)
	{
		s_choices[s_choiceCount].device = available[i];
		s_choices[s_choiceCount].pref = NATIVE_CONTROLS_PREF_PINNED;

		if ((currentPref == NATIVE_CONTROLS_PREF_PINNED) && (available[i] == currentDevice))
		{
			s_choiceSelected = s_choiceCount;
		}

		s_choiceCount++;
	}

	// Automatic resolves the player against whatever is free, which is how a disabled
	// player gets a device back when every device is currently owned by someone else.
	s_choices[s_choiceCount].device = NATIVE_CONTROLS_NO_DEVICE;
	s_choices[s_choiceCount].pref = NATIVE_CONTROLS_PREF_AUTO;

	if (currentPref == NATIVE_CONTROLS_PREF_AUTO)
	{
		s_choiceSelected = s_choiceCount;
	}

	s_choiceCount++;

	if (s_player != 0)
	{
		s_choices[s_choiceCount].device = NATIVE_CONTROLS_NO_DEVICE;
		s_choices[s_choiceCount].pref = NATIVE_CONTROLS_PREF_DISABLED;

		if (currentPref == NATIVE_CONTROLS_PREF_DISABLED)
		{
			s_choiceSelected = s_choiceCount;
		}

		s_choiceCount++;
	}

	if (s_choiceSelected < 0)
	{
		s_choiceSelected = 0;
	}
}

static const char *Controls_ChoiceName(const struct ControlsChoice *choice)
{
	if (choice->pref == NATIVE_CONTROLS_PREF_AUTO)
	{
		return "Automatic";
	}

	if (choice->pref == NATIVE_CONTROLS_PREF_DISABLED)
	{
		return "Disabled";
	}

	return NativeControls_GetDeviceName(choice->device);
}

// The retail font has no glyph for many ASCII characters and repurposes four of them
// (@ [ ^ *) as button artwork, so free-form text reaching DecalFont with one of those
// either comes out blank or turns into a button nobody asked for. Device names come from
// the driver and can hold anything, so every string on this screen is filtered down to
// characters the font can actually show. The four button characters stay reachable
// through s_bindingGlyphs, which is the only path meant to draw them.
static bool Controls_IsSafeChar(char c)
{
	const u8 code = (u8)c;

	// Space, colon and period are handled outside the glyph table.
	if ((code == ' ') || (code == ':') || (code == '.'))
	{
		return true;
	}

	// Below 0x21 is the zero-advance accent run, and 0x21 upwards is the glyph table.
	if (code < 0x21)
	{
		return true;
	}

	switch (code)
	{
	case '@':
	case '[':
	case '^':
	case '*':
		return false;
	default:
		break;
	}

	return data.font_characterIconID[code - 0x21] != 0xFF;
}

// Copies text through the filter. Never grows the string, so dstSize only has to match
// the source.
static void Controls_FilterText(const char *text, char *dst, size_t dstSize)
{
	size_t out = 0;

	for (const char *c = text; (*c != '\0') && (out + 1 < dstSize); c++)
	{
		if (Controls_IsSafeChar(*c))
		{
			dst[out++] = *c;
		}
	}

	dst[out] = '\0';
}

static void Controls_OpenPicker(void)
{
	Controls_BuildChoices();
	s_pickerOpen = true;
}

static void Controls_ClosePicker(void)
{
	s_pickerOpen = false;
}

static void Controls_ApplyChoice(void)
{
	const struct ControlsChoice *choice = &s_choices[s_choiceSelected];

	NativeControls_SetPlayerPref(s_player, choice->pref, choice->device);
}

// One picker entry, filtered into the shared buffer. Measured after filtering, or the box
// would be sized to characters that never reach the screen. Neither caller draws through
// Controls_DrawSafeText, so the buffer can be shared.
static char *Controls_FilteredChoiceName(int index)
{
	Controls_FilterText(Controls_ChoiceName(&s_choices[index]), s_safeBuffer, sizeof(s_safeBuffer));
	return s_safeBuffer;
}

// Widest of the entry names, so the box hugs the list. Bounded because device names
// come from the host and "Xbox 360 Controller" is not the longest thing SDL can
// report.
static int Controls_PickerContentWidth(void)
{
	int widest = DecalFont_GetLineWidth((char *)"SELECT DEVICE", FONT_SMALL);

	for (int i = 0; i < s_choiceCount; i++)
	{
		const int width = DecalFont_GetLineWidth(Controls_FilteredChoiceName(i), FONT_SMALL);

		if (width > widest)
		{
			widest = width;
		}
	}

	return widest;
}

static void Controls_DrawPicker(struct GameTracker *gGT, uint32_t *ot)
{
	int contentWidth = Controls_PickerContentWidth();
	const int maxContent = CONTROLS_POPUP_MAX_WIDTH - (CONTROLS_POPUP_PAD_X * 2);

	if (contentWidth > maxContent)
	{
		// A host device name can be longer than the frame; the box stops growing and
		// the entry is left to clip rather than pushing the popup off screen.
		contentWidth = maxContent;
	}

	const int boxWidth = contentWidth + (CONTROLS_POPUP_PAD_X * 2);
	const int boxHeight = CONTROLS_POPUP_PAD_TOP + CONTROLS_POPUP_TITLE_H + (s_choiceCount * CONTROLS_POPUP_ROW_SPACING) +
	                      CONTROLS_POPUP_PAD_BOTTOM;
	const int boxX = (0x200 - boxWidth) / 2;
	const int boxY = CONTROLS_POPUP_CENTER_Y - (boxHeight / 2);

	const int titleY = boxY + CONTROLS_POPUP_PAD_TOP;
	const int firstRowY = titleY + CONTROLS_POPUP_TITLE_H;

	// Labels first, background last. Later draws sit behind earlier ones, so the
	// background lands behind its own contents while the whole popup still floats
	// above the screen, which submits it before this runs.
	DecalFont_DrawLineOT((char *)"SELECT DEVICE", boxX + (boxWidth / 2), titleY, FONT_SMALL,
	                     JUSTIFY_CENTER | ORANGE, ot);

	for (int i = 0; i < s_choiceCount; i++)
	{
		const int y = firstRowY + (i * CONTROLS_POPUP_ROW_SPACING);

		DecalFont_DrawLineOT(Controls_FilteredChoiceName(i), boxX + CONTROLS_POPUP_PAD_X, y, FONT_SMALL, ORANGE, ot);

		if (i == s_choiceSelected)
		{
			// Inset by less than the text padding so the bar frames the entry.
			RECT sel = {boxX + 4, y - 2, boxWidth - 8, 0x0C};
			CTR_Box_DrawClearBox(&sel, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot,
			                     &gGT->backBuffer->primMem);
		}
	}

	RECT popup = {boxX, boxY, boxWidth, boxHeight};
	RECTMENU_DrawInnerRect(&popup, 4, ot);
}

// Union of player 1's presses and the selected player's own, so the screen stays reachable
// when the selected player is disabled or has not touched their pad yet. A disabled player
// reads as no device and contributes nothing.
u32 MM_Controls_MenuTapped(void)
{
	u32 tapped = (u32)sdata->gGamepads->gamepad[0].buttonsTapped;

	if (s_player != 0)
	{
		tapped |= (u32)sdata->gGamepads->gamepad[s_player].buttonsTapped;
	}

	return tapped;
}

// The held counterpart of MM_Controls_MenuTapped, for the option rows: a numeric option
// reads a direction being held rather than the edge that opens a rebind.
u32 MM_Controls_MenuHeld(void)
{
	u32 held = (u32)sdata->gGamepads->gamepad[0].buttonsHeldCurrFrame;

	if (s_player != 0)
	{
		held |= (u32)sdata->gGamepads->gamepad[s_player].buttonsHeldCurrFrame;
	}

	return held;
}

// Handles input while the picker is up. Cross applies the highlighted choice,
// Circle and Triangle/Start back out without changing anything.
static void Controls_UpdatePicker(u32 tapped)
{
	if ((tapped & BTN_UP) != 0)
	{
		s_choiceSelected = (s_choiceSelected > 0) ? s_choiceSelected - 1 : s_choiceCount - 1;
		OtherFX_Play(0, 1);
	}
	else if ((tapped & BTN_DOWN) != 0)
	{
		s_choiceSelected = (s_choiceSelected < s_choiceCount - 1) ? s_choiceSelected + 1 : 0;
		OtherFX_Play(0, 1);
	}

	// Back closes the picker before it backs out of the section.
	if ((tapped & (BTN_TRIANGLE | BTN_START)) != 0)
	{
		Controls_ClosePicker();
		OtherFX_Play(2, 1);
		return;
	}

	if ((tapped & BTN_CIRCLE) != 0)
	{
		Controls_ClosePicker();
		OtherFX_Play(2, 1);
		return;
	}

	if ((tapped & BTN_CROSS) != 0)
	{
		Controls_ApplyChoice();
		Controls_ClosePicker();
		OtherFX_Play(1, 1);
	}
}

static void Controls_SelectPlayer(int player)
{
	if ((player < 0) || (player >= PLATFORM_INPUT_PAD_COUNT))
	{
		return;
	}

	s_player = player;
}

// Nudges the deadzone like a config slider: same cadence for a held direction, same
// clamping at the ends.
static void Controls_AdjustDeadzone(int device, int held)
{
	if (device == NATIVE_CONTROLS_NO_DEVICE)
	{
		OtherFX_Play(2, 1);
		return;
	}

	for (int direction = -1; direction <= 1; direction += 2)
	{
		if (!Config_OptionStepDue(held, direction))
		{
			continue;
		}

		int value = NativeControls_GetDeadzone(device);

		if (Config_OptionStep(&value, direction, NATIVE_CONTROLS_DEADZONE_MIN, NATIVE_CONTROLS_DEADZONE_MAX,
		                      NATIVE_CONTROLS_DEADZONE_STEP))
		{
			NativeControls_SetDeadzone(device, value);
			OtherFX_Play(1, 1);
		}
	}
}

static void Controls_HandleConfirm(int device, const struct ControlsRow *row)
{
	switch (row->kind)
	{
	case CONTROLS_ROW_DEVICE:
		Controls_OpenPicker();
		OtherFX_Play(0, 1);
		break;

	case CONTROLS_ROW_DEADZONE:
		// Left and Right are the editing gesture here; Cross has nothing to confirm.
		OtherFX_Play(2, 1);
		break;

	case CONTROLS_ROW_BINDING:
		if (device == NATIVE_CONTROLS_NO_DEVICE)
		{
			// Nothing to rebind without a device.
			OtherFX_Play(2, 1);
			break;
		}

		NativeControls_BeginCapture(device, row->binding);
		OtherFX_Play(1, 1);
		break;

	case CONTROLS_ROW_RESTORE:
		if (device == NATIVE_CONTROLS_NO_DEVICE)
		{
			OtherFX_Play(2, 1);
			break;
		}

		NativeControls_RestoreDeviceDefaults(device);
		OtherFX_Play(1, 1);
		break;

	default:
		break;
	}
}

// Draws a shoulder-button hint on the title row. rightJustify anchors the text's
// right edge to x, matching how the value column is laid out.
//
// Dropped by half the difference in glyph height so the smaller text sits centred
// against the big title rather than hanging off its top edge.
static void Controls_DrawHint(const char *text, int x, bool rightJustify, uint32_t *ot)
{
	const int textWidth = DecalFont_GetLineWidth((char *)text, FONT_SMALL);
	const int textX = rightJustify ? (x - textWidth) : x;
	const int textY = CONFIG_TITLE_Y + ((data.font_charPixHeight[FONT_BIG] - data.font_charPixHeight[FONT_SMALL]) / 2);

	DecalFont_DrawLineOT((char *)text, textX, textY, FONT_SMALL, ORANGE, ot);
}

// Button artwork for the four face buttons, indexed by binding. The retail font has no
// glyph slots for them: DecalFont recognises four ordinary ASCII characters and
// substitutes the button artwork, so these rows show the same icons the rest of the game
// uses instead of button names, and the glyph is the whole label. NULL for everything
// else, since the shoulders, d-pad, start/select and sticks have no glyph.
static const char *const s_bindingGlyphs[NATIVE_CONTROLS_BIND_COUNT] = {
    [NATIVE_CONTROLS_BIND_CROSS] = "*",
    [NATIVE_CONTROLS_BIND_CIRCLE] = "@",
    [NATIVE_CONTROLS_BIND_TRIANGLE] = "^",
    [NATIVE_CONTROLS_BIND_SQUARE] = "[",
};

static void Controls_DrawSafeText(const char *text, int x, int y, int flags, uint32_t *ot)
{
	Controls_FilterText(text, s_safeBuffer, sizeof(s_safeBuffer));
	DecalFont_DrawLineOT(s_safeBuffer, x, y, FONT_SMALL, flags, ot);
}

static void Controls_DrawValue(int device, const struct ControlsRow *row, int y, uint32_t *ot)
{
	if (row->kind == CONTROLS_ROW_RESTORE)
	{
		return;
	}

	if ((row->kind == CONTROLS_ROW_DEADZONE) && (device != NATIVE_CONTROLS_NO_DEVICE))
	{
		Config_DrawPercent(NativeControls_GetDeadzone(device), y, ot);
		return;
	}

	// Grey means there is nothing here to rebind, because the player has no device.
	const char *text;
	int colour;

	if (device == NATIVE_CONTROLS_NO_DEVICE)
	{
		text = (row->kind == CONTROLS_ROW_DEVICE) ? "Disabled" : "-";
		colour = GRAY;
	}
	else if (row->kind == CONTROLS_ROW_DEVICE)
	{
		text = NativeControls_GetDeviceName(device);
		colour = WHITE;
	}
	else
	{
		NativeControls_GetBindingLabel(device, row->binding, s_valueBuffer, sizeof(s_valueBuffer));
		text = (s_valueBuffer[0] != '\0') ? s_valueBuffer : "None";
		colour = WHITE;
	}

	Controls_DrawSafeText(text, CONFIG_VALUE_X, y, JUSTIFY_RIGHT | colour, ot);
}

static void Controls_DrawRows(struct RectMenu *menu, struct GameTracker *gGT, uint32_t *ot, int device)
{
	int visibleEnd = s_rowCount;
	if (visibleEnd > s_scroll + CONFIG_MAX_VISIBLE_ROWS)
	{
		visibleEnd = s_scroll + CONFIG_MAX_VISIBLE_ROWS;
	}

	// A rebind in progress is announced in the value column of its own row.
	const bool capturing = NativeControls_IsCapturing();

	for (int i = s_scroll; i < visibleEnd; i++)
	{
		const struct ControlsRow *row = &s_rows[i];
		const int y = CONFIG_ROW_START_Y + (i - s_scroll) * CONFIG_ROW_SPACING;

		const char *label = "";
		switch (row->kind)
		{
		case CONTROLS_ROW_DEVICE:
			label = "DEVICE";
			break;
		case CONTROLS_ROW_BINDING:
			label = s_bindingGlyphs[row->binding];

			if (label == NULL)
			{
				label = NativeControls_GetBindingName(row->binding);
			}

			break;
		case CONTROLS_ROW_DEADZONE:
			label = "Deadzone";
			break;
		case CONTROLS_ROW_RESTORE:
			label = "Restore Defaults";
			break;
		default:
			break;
		}

		// Deliberately not filtered: this column is the one place the four button
		// characters are meant to reach the font, and every other label here is a fixed
		// string that needs no cleaning.
		DecalFont_DrawLineOT((char *)label, CONFIG_LABEL_X, y, FONT_SMALL, ORANGE, ot);

		if (capturing && (row->kind == CONTROLS_ROW_BINDING) && (i == menu->rowSelected))
		{
			// Which half is pending: take a key, then confirm it with Cross.
			snprintf(s_valueBuffer, sizeof(s_valueBuffer), "%s",
			         NativeControls_IsConfirming() ? "* to confirm" : "...");
			DecalFont_DrawLineOT(s_valueBuffer, CONFIG_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | RED, ot);
		}
		else
		{
			Controls_DrawValue(device, row, y, ot);
		}

		if (i == menu->rowSelected)
		{
			Config_DrawRowHighlight(gGT, ot, y);
		}
	}
}

b32 MM_Controls_IsPickerOpen(void)
{
	return s_pickerOpen;
}

// Keeps the cursor and the scroll origin in step: a move resets the scroll so the cursor
// stays visible, and so does a player switch.
static void Controls_MoveSelection(struct RectMenu *menu, int delta)
{
	const int count = s_rowCount;

	if (delta < 0)
	{
		menu->rowSelected = (menu->rowSelected > 0) ? menu->rowSelected - 1 : count - 1;
	}
	else
	{
		menu->rowSelected = (menu->rowSelected < count - 1) ? menu->rowSelected + 1 : 0;
	}

	s_scroll = menu->rowSelected;
	OtherFX_Play(0, 1);
}

static void Controls_ClampSelection(struct RectMenu *menu)
{
	if ((menu->rowSelected < 0) || (menu->rowSelected >= s_rowCount))
	{
		menu->rowSelected = 0;
	}

	if (menu->rowSelected < s_scroll)
	{
		s_scroll = menu->rowSelected;
	}

	if (menu->rowSelected >= s_scroll + CONFIG_MAX_VISIBLE_ROWS)
	{
		s_scroll = menu->rowSelected - CONFIG_MAX_VISIBLE_ROWS + 1;
	}
}

// Player switching stays on player 1's shoulders only. Reading it from the selected player
// too would mean their own L1/R1 changed who is selected, so a second press moved them
// twice.
static void Controls_HandlePlayerSwitch(struct RectMenu *menu, u32 playerOneTapped)
{
	int delta = 0;

	if ((playerOneTapped & BTN_R1) != 0)
	{
		delta = 1;
	}
	else if ((playerOneTapped & BTN_L1) != 0)
	{
		delta = -1;
	}

	if (delta == 0)
	{
		return;
	}

	Controls_SelectPlayer(s_player + delta);
	menu->rowSelected = 0;
	s_scroll = 0;
	OtherFX_Play(0, 1);
}

// The picker owns input while it is up, and a rebind owns it while listening. Either one
// suppresses the screen underneath so their presses are unambiguous.
static void Controls_HandleInput(struct RectMenu *menu, u32 tapped, u32 held, int device)
{
	if (s_pickerOpen)
	{
		Controls_UpdatePicker(tapped);
		return;
	}

	if (NativeControls_IsCapturing())
	{
		return;
	}

	const u32 playerOneTapped = (u32)sdata->gGamepads->gamepad[0].buttonsTapped;
	Controls_HandlePlayerSwitch(menu, playerOneTapped);

	if ((tapped & BTN_UP) != 0)
	{
		Controls_MoveSelection(menu, -1);
	}
	else if ((tapped & BTN_DOWN) != 0)
	{
		Controls_MoveSelection(menu, 1);
	}

	// Left/Right and Cross all open the picker on the device row, so the choices
	// are visible before anything changes.
	if ((s_rows[menu->rowSelected].kind == CONTROLS_ROW_DEVICE) && ((tapped & (BTN_LEFT | BTN_RIGHT)) != 0))
	{
		Controls_OpenPicker();
		OtherFX_Play(0, 1);
	}
	else if (s_rows[menu->rowSelected].kind == CONTROLS_ROW_DEADZONE)
	{
		Controls_AdjustDeadzone(device, held);
	}

	if ((tapped & BTN_CROSS) != 0)
	{
		Controls_HandleConfirm(device, &s_rows[menu->rowSelected]);
	}
}

// Distinct sound from a commit, so an abandoned rebind is not mistaken for a confirmed one.
static void Controls_PollCapture(void)
{
	const int result = NativeControls_PollCapture();

	if (result == 1)
	{
		OtherFX_Play(1, 1);
	}
	else if (result == 2)
	{
		OtherFX_Play(2, 1);
	}
}

static void Controls_DrawTitle(uint32_t *ot)
{
	// Uppercase here, unlike the section name used in the options list: the retail
	// titles this shares a font with are all caps.
	snprintf(s_titleBuffer, sizeof(s_titleBuffer), "CONTROLS P%d", s_player + 1);
	DecalFont_DrawLineOT(s_titleBuffer, 0x100, CONFIG_TITLE_Y, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);
}

void MM_MenuProc_Controls(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;

	Controls_BuildRows();
	Controls_ClampSelection(menu);

	// Read before the input handling, which can move the selection to another player.
	const int device = NativeControls_GetPlayerDevice(s_player);
	Controls_HandleInput(menu, MM_Controls_MenuTapped(), MM_Controls_MenuHeld(), device);
	Controls_PollCapture();

	// This OT draws back to front: a primitive added later sits behind the ones already in
	// the list, which is why Config_DrawFrame can draw the opaque menu background after
	// every row and still leave the text visible. The picker therefore has to be submitted
	// before the screen it covers, while inside Controls_DrawPicker the order is the mirror
	// image and the popup's own background goes last.
	if (s_pickerOpen)
	{
		Controls_DrawPicker(gGT, ot);
	}

	Controls_DrawTitle(ot);
	Controls_DrawHint("L1", CONFIG_LABEL_X, false, ot);
	Controls_DrawHint("R1", CONFIG_VALUE_X, true, ot);
	Controls_DrawRows(menu, gGT, ot, NativeControls_GetPlayerDevice(s_player));
	Config_DrawFrame(ot);
}

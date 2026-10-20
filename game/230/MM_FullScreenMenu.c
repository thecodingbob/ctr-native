#include <common.h>

#ifndef CTR_NATIVE_MM_FULLSCREEN_MENU_C
#define CTR_NATIVE_MM_FULLSCREEN_MENU_C

#include <stdio.h>

// ================= shared full-screen menu layout and navigation =================
//
// Any menu that is a full-screen box: an opaque frame, a title, and a scrolling list of
// label and value pairs.

// Row geometry.
#define FULLSCREEN_MENU_TITLE_X     0x100
#define FULLSCREEN_MENU_TITLE_Y     0x18
#define FULLSCREEN_MENU_ROW_START_Y 0x3C
#define FULLSCREEN_MENU_ROW_SPACING 0x0E
#define FULLSCREEN_MENU_LABEL_X     0x38
#define FULLSCREEN_MENU_VALUE_X     0x1DC

// Vertically centres FONT_SMALL hints alongside a FONT_BIG title.
#define FULLSCREEN_MENU_TITLE_BASELINE_NUDGE_Y \
	((data.font_charPixHeight[FONT_BIG] - data.font_charPixHeight[FONT_SMALL]) / 2)

// Rows visible at once; the rest are reached by scrolling.
#define FULLSCREEN_MENU_MAX_VISIBLE_ROWS 10

// Background, separator and the highlighted row. Shared so that every screen of this kind
// sits in the same box, whichever menu it belongs to.
//
// Note the OT draws back to front: a primitive added later sits behind the ones already
// in the list, which is why the frame can be drawn after every row and still leave the
// text visible.
static void FullScreenMenu_DrawFrame(uint32_t *ot)
{
	{
		RECT sep = {0x20, 0x2C, 0x1C0, 2};
		Color sepColor;
		ColorCode_SetPacked(&sepColor, sdata->battleSetup_Color_UI_1);
		RECTMENU_DrawOuterRect_Edge(&sep, &sepColor, 0x20, ot);
	}

	RECT bg = {0x10, 4, 0x1E0, 0xCE};
	RECTMENU_DrawInnerRect(&bg, 4, ot);
}

static void FullScreenMenu_DrawRowHighlight(struct GameTracker *gGT, uint32_t *ot, int y)
{
	RECT sel = {0x30, y - 2, 0x1B0, 0x0C};
	CTR_Box_DrawClearBox(&sel, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot, &gGT->backBuffer->primMem);
}

// The screen's title, centred above the rows. Every screen draws one, so the font, the
// colour and the column are written once here rather than per screen.
static void FullScreenMenu_DrawTitle(const char *title, int nudgeY, uint32_t *ot)
{
	DecalFont_DrawLineOT((char *)title, FULLSCREEN_MENU_TITLE_X, FULLSCREEN_MENU_TITLE_Y + nudgeY, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);
}

// A row's label, in the left column. Shared so a row looks the same whichever screen it is
// on; the value beside it is left to the caller, because what a value looks like is what
// separates one kind of row from another.
static void FullScreenMenu_DrawRowLabel(const char *label, int y, uint32_t *ot)
{
	DecalFont_DrawLineOT((char *)label, FULLSCREEN_MENU_LABEL_X, y, FONT_SMALL, ORANGE, ot);
}

// ================= option rows: for menus that edit settings =================

// ================= value column =================
//
// Everything that can sit in the value column goes through here, so the column, the font
// and the colour are written once. What a value looks like is the only thing that differs
// between rows, and that is what these four are for.

// A piece of text: an option's name, or what a row that has no number of its own shows.
static void FullScreenMenu_DrawValueText(const char *text, int y, uint32_t *ot)
{
	DecalFont_DrawLineOT((char *)text, FULLSCREEN_MENU_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
}

static void FullScreenMenu_DrawOnOff(bool on, int y, uint32_t *ot)
{
	FullScreenMenu_DrawValueText(on ? "ON" : "OFF", y, ot);
}

// A number with a unit after it, for a setting that is not a percentage: seconds, frames,
// megabytes. The unit is what tells two settings apart when both are plain numbers.
static void FullScreenMenu_DrawValueWithSuffix(int value, const char *suffix, int y, uint32_t *ot)
{
	char text[16];

	snprintf(text, sizeof(text), "%d%s", value, suffix);
	FullScreenMenu_DrawValueText(text, y, ot);
}

static inline void FullScreenMenu_DrawPercent(int percent, int y, uint32_t *ot)
{
	FullScreenMenu_DrawValueWithSuffix(percent, "%", y, ot);
}

// One name out of a list, drawn where its value goes. A value outside the list falls back
// to the first name, so a setting restored from a file cannot leave its row blank.
static const char *FullScreenMenu_ChoiceName(const char *const *choices, int count, int value)
{
	if ((choices == NULL) || (count <= 0))
	{
		return "";
	}

	if ((value < 0) || (value >= count))
	{
		return choices[0];
	}

	return choices[value];
}

// Frames a held Left/Right waits before it steps again, so one press moves a single step
// and holding runs the value through its range at a pace that can be seen.
#define FULLSCREEN_MENU_STEP_REPEAT_FRAMES 3

// True when a held direction should step this frame. Takes the button mask rather than a
// pad, because a screen may answer for more than one player and each has its own mask.
static bool FullScreenMenu_OptionStepDue(int held, int direction)
{
	const int button = (direction < 0) ? BTN_LEFT : BTN_RIGHT;

	if ((held & button) == 0)
	{
		return false;
	}

	return (sdata->frameCounter % FULLSCREEN_MENU_STEP_REPEAT_FRAMES) == 0;
}

// Applies one clamped step. Reports whether the value moved, so a caller can commit only
// on a real change instead of once per frame while a direction is held.
static bool FullScreenMenu_OptionStep(int *value, int direction, int min, int max, int step)
{
	const int before = *value;
	int wanted = before + (direction * step);

	if (wanted < min)
	{
		wanted = min;
	}
	else if (wanted > max)
	{
		wanted = max;
	}

	*value = wanted;

	return *value != before;
}

// ================= shared list navigation =================

struct FullScreenMenu_Scroll
{
	int offset; // first visible row
};

// First visible row, given the current selection and how many rows there are.
static int FullScreenMenu_ScrollOffsetFor(const struct FullScreenMenu_Scroll *scroll, int selected, int numRows)
{
	int offset = scroll->offset;

	if (selected < offset)
	{
		offset = selected;
	}
	else if (selected >= offset + FULLSCREEN_MENU_MAX_VISIBLE_ROWS)
	{
		offset = selected - FULLSCREEN_MENU_MAX_VISIBLE_ROWS + 1;
	}

	if (offset > numRows - FULLSCREEN_MENU_MAX_VISIBLE_ROWS)
	{
		offset = numRows - FULLSCREEN_MENU_MAX_VISIBLE_ROWS;
	}

	if (offset < 0)
	{
		offset = 0;
	}

	return offset;
}

// Row a screen should stop on, for a list of this many rows. Keeps the selection inside
// the list even when a row it was pointing at has just gone away.
static int FullScreenMenu_ClampRow(int selected, int numRows)
{
	if (numRows <= 0)
	{
		return 0;
	}

	if (selected < 0)
	{
		return 0;
	}

	if (selected > numRows - 1)
	{
		return numRows - 1;
	}

	return selected;
}

// Where the list ends on screen: the last visible row, clamped to the end of the list so
// a short list does not report rows that are not there.
static int FullScreenMenu_VisibleEnd(const struct FullScreenMenu_Scroll *scroll, int numRows)
{
	int end = scroll->offset + FULLSCREEN_MENU_MAX_VISIBLE_ROWS;

	if (end > numRows)
	{
		end = numRows;
	}

	return end;
}

// Y of the row at visible index `i`, counted from the top of the list.
static int FullScreenMenu_RowY(int i)
{
	return FULLSCREEN_MENU_ROW_START_Y + i * FULLSCREEN_MENU_ROW_SPACING;
}

// Moves the selection by one row, wrapping at either end, and follows it with the scroll
// so the selected row is always on screen. One direction per press, so holding does not
// run: lists step per tap, unlike a value's Left/Right, which runs on a timer.
static int FullScreenMenu_MoveRow(struct FullScreenMenu_Scroll *scroll, int selected, int direction, int numRows)
{
	if (numRows <= 0)
	{
		return 0;
	}

	if (direction < 0)
	{
		selected = (selected > 0) ? selected - 1 : numRows - 1;
	}
	else
	{
		selected = (selected < numRows - 1) ? selected + 1 : 0;
	}

	scroll->offset = FullScreenMenu_ScrollOffsetFor(scroll, selected, numRows);

	return selected;
}

// ================= settings list =================
//
// Row metadata and value operations, independent of how a screen stores its settings.

enum FullScreenMenu_RowKind
{
	FULLSCREEN_ROW_TOGGLE, // ON or OFF
	FULLSCREEN_ROW_CHOICE, // one name out of a list
	FULLSCREEN_ROW_STEP,   // a number in a range, drawn with a unit
	FULLSCREEN_ROW_ACTION, // no value of its own; the screen answers it
};

struct FullScreenMenu_Row
{
	const char *label;
	enum FullScreenMenu_RowKind kind;
	const char *const *choices; // FULLSCREEN_ROW_CHOICE names
	int numChoices;             // FULLSCREEN_ROW_CHOICE
	int min;                    // FULLSCREEN_ROW_STEP
	int max;                    // FULLSCREEN_ROW_STEP
	int step;                   // FULLSCREEN_ROW_STEP
	const char *suffix;         // FULLSCREEN_ROW_STEP unit
};

static void FullScreenMenu_DrawRowValue(const struct FullScreenMenu_Row *row, int value, int y, uint32_t *ot)
{
	switch (row->kind)
	{
	case FULLSCREEN_ROW_TOGGLE:
		FullScreenMenu_DrawOnOff(value != 0, y, ot);
		break;

	case FULLSCREEN_ROW_CHOICE:
		FullScreenMenu_DrawValueText(FullScreenMenu_ChoiceName(row->choices, row->numChoices, value), y, ot);
		break;

	case FULLSCREEN_ROW_STEP:
		FullScreenMenu_DrawValueWithSuffix(value, row->suffix, y, ot);
		break;

	case FULLSCREEN_ROW_ACTION:
		// An action row has no value to put in the column.
		break;
	}
}

// What a direction does to a row. Only a row with a range to move within answers one, which
// is what the options menu does with its sliders: a choice and a toggle are changed by the
// confirm button and not by the directions, so a held direction cannot run them. Reports
// whether the value moved.
static bool FullScreenMenu_RowStep(const struct FullScreenMenu_Row *row, int *value, int direction)
{
	if (row->kind != FULLSCREEN_ROW_STEP)
	{
		return false;
	}

	return FullScreenMenu_OptionStep(value, direction, row->min, row->max, row->step);
}

// What the confirm button does to a row that holds a value: a toggle flips and a choice
// moves to the next name, round the end of the list and back to the first. A row with a
// range has no single next value, so the button leaves it to the directions, and neither
// touches a row the screen answers itself. Reports whether the value moved.
static bool FullScreenMenu_RowActivate(const struct FullScreenMenu_Row *row, int *value)
{
	switch (row->kind)
	{
	case FULLSCREEN_ROW_TOGGLE:
		*value = (*value != 0) ? 0 : 1;
		return true;

	case FULLSCREEN_ROW_CHOICE:
	{
		if (row->numChoices <= 0)
		{
			return false;
		}

		*value = (*value + 1) % row->numChoices;
		return true;
	}

	case FULLSCREEN_ROW_STEP:
	case FULLSCREEN_ROW_ACTION:
		return false;
	}

	return false;
}

// Keeps the selection inside the list and the list scrolled to it, for when rows have been
// added or taken away since the last frame.
static void FullScreenMenu_FollowScroll(struct FullScreenMenu_Scroll *scroll, s16 *selected, int numRows)
{
	*selected = (s16)FullScreenMenu_ClampRow(*selected, numRows);
	scroll->offset = FullScreenMenu_ScrollOffsetFor(scroll, *selected, numRows);
}

// ================= driving a full-screen list =================

static void FullScreenMenu_NavScroll(struct GamepadBuffer *pad, struct FullScreenMenu_Scroll *scroll, s16 *selected, int numRows)
{
	FullScreenMenu_FollowScroll(scroll, selected, numRows);

	if ((pad->buttonsTapped & BTN_UP) != 0)
	{
		*selected = (s16)FullScreenMenu_MoveRow(scroll, *selected, -1, numRows);
		OtherFX_Play(0, 1);
	}

	if ((pad->buttonsTapped & BTN_DOWN) != 0)
	{
		*selected = (s16)FullScreenMenu_MoveRow(scroll, *selected, 1, numRows);
		OtherFX_Play(0, 1);
	}
}

// Menus supply row content; clipping, placement and highlighting stay shared.
typedef void (*FullScreenMenu_DrawRow)(const void *context, int row, int y, bool selected, uint32_t *ot);

static void FullScreenMenu_DrawRows(const struct FullScreenMenu_Scroll *scroll, int numRows, int selected,
                                   FullScreenMenu_DrawRow drawRow, const void *context,
                                   struct GameTracker *gGT, uint32_t *ot)
{
	const int visibleEnd = FullScreenMenu_VisibleEnd(scroll, numRows);

	for (int i = scroll->offset; i < visibleEnd; i++)
	{
		const int y = FullScreenMenu_RowY(i - scroll->offset);

		drawRow(context, i, y, i == selected, ot);

		if (i == selected)
		{
			FullScreenMenu_DrawRowHighlight(gGT, ot, y);
		}
	}
}

static void FullScreenMenu_DrawScreen(const char *title, int nudgeY, const struct FullScreenMenu_Scroll *scroll,
                                     int numRows, int selected, FullScreenMenu_DrawRow drawRow, const void *context,
                                     struct GameTracker *gGT, uint32_t *ot)
{
	FullScreenMenu_DrawTitle(title, nudgeY, ot);
	FullScreenMenu_DrawRows(scroll, numRows, selected, drawRow, context, gGT, ot);
	FullScreenMenu_DrawFrame(ot);
}

#endif // CTR_NATIVE_MM_FULLSCREEN_MENU_C

#include <common.h>

// ================= Chaos setup screens =================
//
// Two full-screen boxes asking for the settings of a file being created with Chaos,
// shown after the racer-count screen and before the character select. The frame, the row
// geometry, the list navigation and the option-row mechanics come from MM_FullScreenMenu.c,
// which the options menu and Controls use as well.
#define CHAOS_MENU_STATE (EXECUTE_FUNCPTR | DISABLE_INPUT_ALLOW_FUNCPTRS)

#include "MM_FullScreenMenu.c"

// Defined below, but named here because the menu structs initialise their funcPtr.
static void MM_MenuProc_ChaosConfig(struct RectMenu *menu);
static void MM_MenuProc_ChaosEffects(struct RectMenu *menu);

// How long one activation lasts, and how often a timed trigger fires. Both in seconds, in
// steps the player can hit with a held direction without passing through values that are
// not worth having.
enum
{
	CHAOS_CONFIG_DURATION_MIN_SECONDS = 5,
	CHAOS_CONFIG_DURATION_MAX_SECONDS = 60,
	CHAOS_CONFIG_DURATION_STEP_SECONDS = 5,

	CHAOS_CONFIG_INTERVAL_MIN_SECONDS = 5,
	CHAOS_CONFIG_INTERVAL_MAX_SECONDS = 120,
	CHAOS_CONFIG_INTERVAL_STEP_SECONDS = 5,
};

// Shown for the two triggers that are timed, so the resolution row can be read as the
// reason the interval row is there at all.
static bool Chaos_TriggerIsTimed(int trigger)
{
	return (trigger == CHAOS_TRIGGER_RANDOM) || (trigger == CHAOS_TRIGGER_INTERVAL);
}

static const char *Chaos_TriggerName(int trigger)
{
	switch (trigger)
	{
	case CHAOS_TRIGGER_PICKUP:
		return "Pickup";
	case CHAOS_TRIGGER_INTERVAL:
		return "Interval";
	case CHAOS_TRIGGER_EXTERNAL:
		return "External";
	default:
		return "Random";
	}
}

// Seconds as a plain number, for the two rows that are not percentages. The shared helper
// in MM_FullScreenMenu.c always appends a percent sign, which is right for a video setting
// and wrong for "how many seconds an effect lasts".
static void Chaos_DrawSeconds(int seconds, int y, uint32_t *ot)
{
	char text[8];

	snprintf(text, sizeof(text), "%ds", seconds);
	DecalFont_DrawLineOT(text, FULLSCREEN_MENU_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
}

// ---------------------------------------------------------------- settings screen

// Duration, Trigger, and the interval when the trigger is timed. The interval row is the
// only one that comes and goes, so the row count follows the trigger rather than being fixed.
enum ChaosConfigRow
{
	CHAOS_CONFIG_ROW_DURATION = 0,
	CHAOS_CONFIG_ROW_TRIGGER,
	CHAOS_CONFIG_ROW_INTERVAL, // only while the trigger is timed
	CHAOS_CONFIG_ROW_MAX
};

static int Chaos_ConfigRowCount(void)
{
	return Chaos_TriggerIsTimed(sdata->chaosSettings.trigger) ? (CHAOS_CONFIG_ROW_INTERVAL + 1) : (CHAOS_CONFIG_ROW_TRIGGER + 1);
}

// Terminator only: these screens never let the generic row handling run, so the table is
// not navigation, it is what stops RECTMENU walking off the end of nothing.
static struct MenuRow s_chaosConfigRows[1];

static struct FullScreenMenu_Scroll s_chaosConfigScroll = {0};

struct RectMenu MM_menuChaosConfig = {
	.rows = s_chaosConfigRows,
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.state = CHAOS_MENU_STATE,
	.funcPtr = MM_MenuProc_ChaosConfig,
};

// Applies one step of the selected row. The trigger cycles rather than climbing a range,
// so a clamped arithmetic step would be wrong for it and it wraps by hand.
//
// The two seconds fields are u16 while the shared step helper works on int, so each is
// widened into a local. Casting the stored address instead would let the helper write four
// bytes over the field beside it.
static void Chaos_ConfigEditRow(int row, int direction)
{
	struct ChaosSettings *settings = &sdata->chaosSettings;

	if (row == CHAOS_CONFIG_ROW_DURATION)
	{
		int seconds = settings->durationSeconds;

		FullScreenMenu_OptionStep(&seconds, direction, CHAOS_CONFIG_DURATION_MIN_SECONDS, CHAOS_CONFIG_DURATION_MAX_SECONDS,
		                          CHAOS_CONFIG_DURATION_STEP_SECONDS);
		settings->durationSeconds = (u16)seconds;
	}
	else if (row == CHAOS_CONFIG_ROW_TRIGGER)
	{
		int trigger = settings->trigger + direction;

		if (trigger < 0)
		{
			trigger = CHAOS_TRIGGER_COUNT - 1;
		}
		else if (trigger >= CHAOS_TRIGGER_COUNT)
		{
			trigger = 0;
		}

		settings->trigger = (u8)trigger;
	}
	else if (row == CHAOS_CONFIG_ROW_INTERVAL)
	{
		int seconds = settings->triggerIntervalSeconds;

		FullScreenMenu_OptionStep(&seconds, direction, CHAOS_CONFIG_INTERVAL_MIN_SECONDS, CHAOS_CONFIG_INTERVAL_MAX_SECONDS,
		                          CHAOS_CONFIG_INTERVAL_STEP_SECONDS);
		settings->triggerIntervalSeconds = (u16)seconds;
	}
}

static void Chaos_DrawConfigRows(struct RectMenu *menu, struct GameTracker *gGT, uint32_t *ot)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	const int numRows = Chaos_ConfigRowCount();
	const int visibleEnd = FullScreenMenu_VisibleEnd(&s_chaosConfigScroll, numRows);

	for (int i = s_chaosConfigScroll.offset; i < visibleEnd; i++)
	{
		const int y = FullScreenMenu_RowY(i - s_chaosConfigScroll.offset);

		switch (i)
		{
		case CHAOS_CONFIG_ROW_DURATION:
			FullScreenMenu_DrawRowLabel("Duration", y, ot);
			Chaos_DrawSeconds(settings->durationSeconds, y, ot);
			break;

		case CHAOS_CONFIG_ROW_TRIGGER:
			FullScreenMenu_DrawRowLabel("Trigger", y, ot);
			DecalFont_DrawLineOT((char *)Chaos_TriggerName(settings->trigger), FULLSCREEN_MENU_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
			break;

		default:
			FullScreenMenu_DrawRowLabel("Interval", y, ot);
			Chaos_DrawSeconds(settings->triggerIntervalSeconds, y, ot);
			break;
		}

		if (i == menu->rowSelected)
		{
			FullScreenMenu_DrawRowHighlight(gGT, ot, y);
		}
	}
}

void MM_MenuProc_ChaosConfig(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];

	const int numRows = Chaos_ConfigRowCount();

	menu->rowSelected = FullScreenMenu_ClampRow(menu->rowSelected, numRows);
	s_chaosConfigScroll.offset = FullScreenMenu_ScrollOffsetFor(&s_chaosConfigScroll, menu->rowSelected, numRows);

	if ((pad->buttonsTapped & BTN_UP) != 0)
	{
		menu->rowSelected = FullScreenMenu_MoveRow(&s_chaosConfigScroll, menu->rowSelected, -1, numRows);
		OtherFX_Play(0, 1);
	}

	if ((pad->buttonsTapped & BTN_DOWN) != 0)
	{
		menu->rowSelected = FullScreenMenu_MoveRow(&s_chaosConfigScroll, menu->rowSelected, 1, numRows);
		OtherFX_Play(0, 1);
	}

	for (int direction = -1; direction <= 1; direction += 2)
	{
		if (FullScreenMenu_OptionStepDue(pad->buttonsHeldCurrFrame, direction))
		{
			Chaos_ConfigEditRow(menu->rowSelected, direction);
		}
	}

	FullScreenMenu_DrawTitle("CHAOS SETUP", 0, ot);
	Chaos_DrawConfigRows(menu, gGT, ot);
	FullScreenMenu_DrawFrame(ot);

	// Triangle goes back to the last choice made in the chain, so the player can still
	// change it. Cross moves on to the effects.
	if ((pad->buttonsTapped & BTN_TRIANGLE) != 0)
	{
		OtherFX_Play(2, 1);
		MM_NewLoad_ReturnFromChaosSetup();
		return;
	}

	if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
	{
		OtherFX_Play(1, 1);
		sdata->ptrDesiredMenu = &MM_menuChaosEffects;
	}
}

// ---------------------------------------------------------------- effects screen

static struct MenuRow s_chaosEffectRows[1];

static struct FullScreenMenu_Scroll s_chaosEffectScroll = {0};

struct RectMenu MM_menuChaosEffects = {
	.rows = s_chaosEffectRows,
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.state = CHAOS_MENU_STATE,
	.funcPtr = MM_MenuProc_ChaosEffects,
};

static void Chaos_DrawEffectRows(struct RectMenu *menu, struct GameTracker *gGT, uint32_t *ot)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	const int visibleEnd = FullScreenMenu_VisibleEnd(&s_chaosEffectScroll, CHAOS_EFFECT_COUNT);

	for (int i = s_chaosEffectScroll.offset; i < visibleEnd; i++)
	{
		const ChaosEffectId id = ChaosEffect_FromIndex(i);
		const ChaosEffectDef *def = ChaosEffect_Def(id);
		const int y = FullScreenMenu_RowY(i - s_chaosEffectScroll.offset);

		FullScreenMenu_DrawRowLabel(def->label, y, ot);
		DecalFont_DrawLineOT(ChaosEffect_IsEnabled(settings->effects, id) ? "ON" : "OFF", FULLSCREEN_MENU_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);

		if (i == menu->rowSelected)
		{
			FullScreenMenu_DrawRowHighlight(gGT, ot, y);
		}
	}
}

void MM_MenuProc_ChaosEffects(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];

	menu->rowSelected = FullScreenMenu_ClampRow(menu->rowSelected, CHAOS_EFFECT_COUNT);
	s_chaosEffectScroll.offset = FullScreenMenu_ScrollOffsetFor(&s_chaosEffectScroll, menu->rowSelected, CHAOS_EFFECT_COUNT);

	if ((pad->buttonsTapped & BTN_UP) != 0)
	{
		menu->rowSelected = FullScreenMenu_MoveRow(&s_chaosEffectScroll, menu->rowSelected, -1, CHAOS_EFFECT_COUNT);
		OtherFX_Play(0, 1);
	}

	if ((pad->buttonsTapped & BTN_DOWN) != 0)
	{
		menu->rowSelected = FullScreenMenu_MoveRow(&s_chaosEffectScroll, menu->rowSelected, 1, CHAOS_EFFECT_COUNT);
		OtherFX_Play(0, 1);
	}

	for (int direction = -1; direction <= 1; direction += 2)
	{
		if (FullScreenMenu_OptionStepDue(pad->buttonsHeldCurrFrame, direction))
		{
			const ChaosEffectId id = ChaosEffect_FromIndex(menu->rowSelected);

			ChaosEffect_Toggle(sdata->chaosSettings.effects, &sdata->chaosSettings.enabledCount, id);
		}
	}

	FullScreenMenu_DrawTitle("CHAOS EFFECTS", 0, ot);
	Chaos_DrawEffectRows(menu, gGT, ot);
	FullScreenMenu_DrawFrame(ot);

	if ((pad->buttonsTapped & BTN_TRIANGLE) != 0)
	{
		OtherFX_Play(2, 1);
		sdata->ptrDesiredMenu = &MM_menuChaosConfig;
		return;
	}

	if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
	{
		OtherFX_Play(1, 1);
		MM_NewLoad_FinishChaosSetup();
	}
}
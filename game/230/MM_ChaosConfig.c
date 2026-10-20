#include <common.h>
#include "MM_FullScreenMenu.c"

// Standalone creation screen, entered after Players and before character selection.
// Page changes draw immediately; leaving the screen happens after drawing.
static void MM_MenuProc_ChaosConfig(struct RectMenu *menu);

struct RectMenu MM_menuChaosConfig = {
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.state = EXECUTE_FUNCPTR | DISABLE_INPUT_ALLOW_FUNCPTRS,
	.funcPtr = MM_MenuProc_ChaosConfig,
};

enum
{
	CHAOS_DURATION_MIN_SECONDS = 5,
	CHAOS_DURATION_MAX_SECONDS = 60,
	CHAOS_DURATION_STEP_SECONDS = 5,
	CHAOS_INTERVAL_MIN_SECONDS = 5,
	CHAOS_INTERVAL_MAX_SECONDS = 120,
	CHAOS_INTERVAL_STEP_SECONDS = 5,
};

static const char *const s_chaosTriggerNames[CHAOS_TRIGGER_COUNT] = {
	[CHAOS_TRIGGER_RANDOM] = "Random",
	[CHAOS_TRIGGER_PICKUP] = "Pickup",
	[CHAOS_TRIGGER_INTERVAL] = "Interval",
	[CHAOS_TRIGGER_EXTERNAL] = "External",
};

static bool Chaos_TriggerIsTimed(int trigger)
{
	return trigger == CHAOS_TRIGGER_RANDOM || trigger == CHAOS_TRIGGER_INTERVAL;
}

// Setting identities are independent of their visible positions.
enum ChaosSetting
{
	CHAOS_SETTING_DURATION,
	CHAOS_SETTING_TRIGGER,
	CHAOS_SETTING_INTERVAL,
	CHAOS_SETTING_EFFECTS,
	CHAOS_SETTING_CONFIRM,
	CHAOS_SETTING_COUNT,
};

static const struct FullScreenMenu_Row s_chaosSettings[CHAOS_SETTING_COUNT] = {
	[CHAOS_SETTING_DURATION] = {
		.label = "Duration", .kind = FULLSCREEN_ROW_STEP,
		.min = CHAOS_DURATION_MIN_SECONDS, .max = CHAOS_DURATION_MAX_SECONDS,
		.step = CHAOS_DURATION_STEP_SECONDS, .suffix = "s"},
	[CHAOS_SETTING_TRIGGER] = {
		.label = "Trigger", .kind = FULLSCREEN_ROW_CHOICE,
		.choices = s_chaosTriggerNames, .numChoices = CHAOS_TRIGGER_COUNT},
	[CHAOS_SETTING_INTERVAL] = {
		.label = "Interval", .kind = FULLSCREEN_ROW_STEP,
		.min = CHAOS_INTERVAL_MIN_SECONDS, .max = CHAOS_INTERVAL_MAX_SECONDS,
		.step = CHAOS_INTERVAL_STEP_SECONDS, .suffix = "s"},
	[CHAOS_SETTING_EFFECTS] = {.label = "Effects", .kind = FULLSCREEN_ROW_ACTION},
	[CHAOS_SETTING_CONFIRM] = {.label = "Confirm", .kind = FULLSCREEN_ROW_ACTION},
};

enum ChaosPage
{
	CHAOS_PAGE_SETTINGS,
	CHAOS_PAGE_EFFECTS,
};

struct ChaosMenuPage
{
	const char *title;
	struct FullScreenMenu_Scroll scroll;
	int numRows;
};

static struct ChaosMenuPage s_chaosPages[] = {
	[CHAOS_PAGE_SETTINGS] = {.title = "CHAOS SETUP"},
	[CHAOS_PAGE_EFFECTS] = {.title = "CHAOS EFFECTS", .numRows = CHAOS_EFFECT_COUNT},
};
static enum ChaosPage s_page;
static enum ChaosSetting s_visibleSettings[CHAOS_SETTING_COUNT];
static int s_effectsRow;

static void Chaos_BuildSettingRows(void)
{
	struct ChaosMenuPage *page = &s_chaosPages[CHAOS_PAGE_SETTINGS];
	page->numRows = 0;

	for (int setting = 0; setting < CHAOS_SETTING_COUNT; setting++)
	{
		if (setting == CHAOS_SETTING_INTERVAL && !Chaos_TriggerIsTimed(sdata->chaosSettings.trigger))
		{
			continue;
		}
		if (setting == CHAOS_SETTING_EFFECTS)
		{
			s_effectsRow = page->numRows;
		}
		s_visibleSettings[page->numRows++] = (enum ChaosSetting)setting;
	}
}

static int Chaos_SettingValue(enum ChaosSetting setting)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	switch (setting)
	{
	case CHAOS_SETTING_DURATION:
		return settings->durationSeconds;
	case CHAOS_SETTING_TRIGGER:
		return settings->trigger;
	case CHAOS_SETTING_INTERVAL:
		return settings->triggerIntervalSeconds;
	case CHAOS_SETTING_EFFECTS:
	case CHAOS_SETTING_CONFIRM:
	case CHAOS_SETTING_COUNT:
	default:
		return 0;
	}
}

// Shared row operations work on ints; commit only the changed field with its stored type.
static void Chaos_EditSetting(enum ChaosSetting setting, int direction)
{
	const struct FullScreenMenu_Row *row = &s_chaosSettings[setting];
	int value = Chaos_SettingValue(setting);
	const bool changed = direction == 0
	                         ? FullScreenMenu_RowActivate(row, &value)
	                         : FullScreenMenu_RowStep(row, &value, direction);
	if (!changed)
	{
		return;
	}

	struct ChaosSettings *settings = &sdata->chaosSettings;
	switch (setting)
	{
	case CHAOS_SETTING_DURATION:
		settings->durationSeconds = (u16)value;
		break;
	case CHAOS_SETTING_TRIGGER:
	{
		const bool wasTimed = Chaos_TriggerIsTimed(settings->trigger);
		settings->trigger = (u8)value;
		if (wasTimed != Chaos_TriggerIsTimed(settings->trigger))
		{
			Chaos_BuildSettingRows();
		}
		break;
	}
	case CHAOS_SETTING_INTERVAL:
		settings->triggerIntervalSeconds = (u16)value;
		break;
	case CHAOS_SETTING_EFFECTS:
	case CHAOS_SETTING_CONFIRM:
	case CHAOS_SETTING_COUNT:
	default:
		break;
	}
}

void MM_ChaosConfig_OnEnter(void)
{
	s_page = CHAOS_PAGE_SETTINGS;
	s_chaosPages[CHAOS_PAGE_SETTINGS].scroll.offset = 0;
	s_chaosPages[CHAOS_PAGE_EFFECTS].scroll.offset = 0;
	MM_menuChaosConfig.rowSelected = 0;
	Chaos_BuildSettingRows();
}

static void Chaos_DrawRow(const void *context, int row, int y, bool selected, uint32_t *ot)
{
	(void)context;
	(void)selected;
	if (s_page == CHAOS_PAGE_EFFECTS)
	{
		const ChaosEffectId effect = ChaosEffect_FromIndex(row);
		FullScreenMenu_DrawRowLabel(ChaosEffect_Def(effect)->label, y, ot);
		FullScreenMenu_DrawOnOff(ChaosEffect_IsEnabled(sdata->chaosSettings.effects, effect), y, ot);
	}
	else
	{
		const enum ChaosSetting setting = s_visibleSettings[row];
		const struct FullScreenMenu_Row *spec = &s_chaosSettings[setting];
		FullScreenMenu_DrawRowLabel(spec->label, y, ot);
		FullScreenMenu_DrawRowValue(spec, Chaos_SettingValue(setting), y, ot);
	}
}

static void MM_MenuProc_ChaosConfig(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];
	struct ChaosMenuPage *page = &s_chaosPages[s_page];
	s16 selected = menu->rowSelected;
	bool leave = false;
	bool finish = false;

	FullScreenMenu_NavScroll(pad, &page->scroll, &selected, page->numRows);
	if (s_page == CHAOS_PAGE_SETTINGS)
	{
		for (int direction = -1; direction <= 1; direction += 2)
		{
			if (FullScreenMenu_OptionStepDue(pad->buttonsHeldCurrFrame, direction))
			{
				Chaos_EditSetting(s_visibleSettings[selected], direction);
			}
		}
	}

	if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
	{
		OtherFX_Play(1, 1);
		if (s_page == CHAOS_PAGE_EFFECTS)
		{
			ChaosEffect_Toggle(sdata->chaosSettings.effects, &sdata->chaosSettings.enabledCount,
			                   ChaosEffect_FromIndex(selected));
		}
		else
		{
			switch (s_visibleSettings[selected])
			{
			case CHAOS_SETTING_EFFECTS:
				s_page = CHAOS_PAGE_EFFECTS;
				selected = 0;
				break;
			case CHAOS_SETTING_CONFIRM:
				finish = true;
				break;
			case CHAOS_SETTING_DURATION:
			case CHAOS_SETTING_TRIGGER:
			case CHAOS_SETTING_INTERVAL:
				Chaos_EditSetting(s_visibleSettings[selected], 0);
				break;
			case CHAOS_SETTING_COUNT:
				break;
			}
		}
	}

	if ((pad->buttonsTapped & BTN_TRIANGLE) != 0)
	{
		OtherFX_Play(2, 1);
		if (s_page == CHAOS_PAGE_EFFECTS)
		{
			s_page = CHAOS_PAGE_SETTINGS;
			selected = s_effectsRow;
		}
		else
		{
			leave = true;
		}
	}

	page = &s_chaosPages[s_page];
	FullScreenMenu_FollowScroll(&page->scroll, &selected, page->numRows);
	menu->rowSelected = selected;
	FullScreenMenu_DrawScreen(page->title, 0, &page->scroll, page->numRows, selected,
	                          Chaos_DrawRow, NULL, gGT, ot);

	if (finish)
	{
		MM_NewLoad_FinishChaosSetup();
	}
	else if (leave)
	{
		MM_NewLoad_ReturnFromChaosSetup();
	}
}

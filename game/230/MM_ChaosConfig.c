#include <common.h>

// ================= Chaos setup screen =================
//
// One full-screen box asking for the settings of a file being created with Chaos, shown
// after the racer-count screen and before the character select. The frame, the row
// geometry, the value column and the settings list come from MM_FullScreenMenu.c, which the
// options menu and Controls use as well, so this screen is described by a row table and
// holds no drawing code of its own.
//
// It is a menu in its own right, the way the options menu is, rather than a step of the
// new-file chain in MM_MenuFlow.c. MM_NewLoad_Exit promotes it with ptrDesiredMenu, which
// makes it the active menu, and that is the point: a step of the chain is a box hanging off
// the one before it, so the chain underneath keeps drawing its selected row through whatever
// is on top of it. Being the active menu means nothing else draws.
//
// That shape brings the rest of the machinery with it. RECTMENU_ProcessState calls funcPtr
// on the active menu every frame (`:923`) and skips RECTMENU_ProcessInput for it (`:934`),
// so the screen owns its input, its cursor and its submenus.
#include "MM_FullScreenMenu.c"

static void MM_MenuProc_ChaosConfig(struct RectMenu *menu);

// Only here because the generic code reads menu->rows without a guard, and it has to hold a
// real terminator: RECTMENU_STRING_NONE is -1 and a zeroed entry is 0, which reads as a live
// row. Nothing ever walks it, since the screen skips RECTMENU_ProcessInput entirely.
static struct MenuRow s_chaosRows[1] = {{RECTMENU_STRING_NONE, 0, 0, 0, 0}};

struct RectMenu MM_menuChaosConfig = {
	.rows = s_chaosRows,
	.stringIndexTitle = RECTMENU_STRING_NONE,
	.state = EXECUTE_FUNCPTR | DISABLE_INPUT_ALLOW_FUNCPTRS,
	.funcPtr = MM_MenuProc_ChaosConfig,
};

// How long one activation lasts, and how often a timed trigger fires. Both in seconds.
enum
{
	CHAOS_DURATION_MIN_SECONDS = 5,
	CHAOS_DURATION_MAX_SECONDS = 60,
	CHAOS_DURATION_STEP_SECONDS = 5,

	CHAOS_INTERVAL_MIN_SECONDS = 5,
	CHAOS_INTERVAL_MAX_SECONDS = 120,
	CHAOS_INTERVAL_STEP_SECONDS = 5,
};

// The triggers, as a choice row: the screen names them here rather than in a switch, so the
// row that shows them has nothing to know about them.
static const char *const s_chaosTriggerNames[] = {
	"Random",
	"Pickup",
	"Interval",
	"External",
};

enum
{
	CHAOS_TRIGGER_NAME_COUNT = sizeof(s_chaosTriggerNames) / sizeof(s_chaosTriggerNames[0]),
};

// Shown for the two triggers that are timed, so the interval row can be read as the reason
// that row is there at all.
static bool Chaos_TriggerIsTimed(int trigger)
{
	return (trigger == CHAOS_TRIGGER_RANDOM) || (trigger == CHAOS_TRIGGER_INTERVAL);
}

// Which of the two screens of this box is up. The effects are a submenu of the settings
// rather than a screen of their own, the same way a section of the options menu is a
// submenu of the options menu, so the two share a title bar and a back button.
enum ChaosPage
{
	CHAOS_PAGE_SETTINGS = 0,
	CHAOS_PAGE_EFFECTS,
};

static enum ChaosPage s_page = CHAOS_PAGE_SETTINGS;

// ---------------------------------------------------------------- settings

// The interval row is only there while the trigger is timed, so it is the one row that
// comes and goes and every row after it shifts with it.
enum ChaosSettingRow
{
	CHAOS_SETTING_DURATION = 0,
	CHAOS_SETTING_TRIGGER,
	CHAOS_SETTING_INTERVAL, // only while the trigger is timed
	CHAOS_SETTING_EFFECTS,
	CHAOS_SETTING_CONFIRM,
	CHAOS_SETTING_ROWS_MAX,
};

// Where the interval row is, or -1 when the trigger leaves it no meaning.
static int Chaos_SettingIntervalRow(void)
{
	return Chaos_TriggerIsTimed(sdata->chaosSettings.trigger) ? (int)CHAOS_SETTING_INTERVAL : -1;
}

// The rows from here on are always there, and the indices above already account for the
// interval row being present, so each of them moves one row back only when it is not. Not
// the other way round: adding for its presence as well puts the last row past the array.
static int Chaos_SettingEffectsRow(void)
{
	return (int)CHAOS_SETTING_EFFECTS - (Chaos_SettingIntervalRow() < 0 ? 1 : 0);
}

static int Chaos_SettingConfirmRow(void)
{
	return Chaos_SettingEffectsRow() + 1;
}

static int Chaos_SettingNumRows(void)
{
	return Chaos_SettingConfirmRow() + 1;
}

// The interval row is the only one that comes and goes, so the table is rebuilt whenever
// the trigger changes and the row count follows it. Sized from the enum, so a row added
// there cannot leave the arrays short.
static struct FullScreenMenu_Row s_settingRows[CHAOS_SETTING_ROWS_MAX];
static int s_settingValues[CHAOS_SETTING_ROWS_MAX];

// Written at the computed positions rather than at the ones the names above carry: the
// interval row shifts every row after it, and a row written at its own name's index would
// be drawn at the wrong place or not at all.
static void Chaos_BuildSettingRows(void)
{
	s_settingRows[CHAOS_SETTING_DURATION] = (struct FullScreenMenu_Row){
		"Duration", FULLSCREEN_ROW_STEP, NULL, CHAOS_DURATION_MAX_SECONDS, CHAOS_DURATION_MIN_SECONDS,
		CHAOS_DURATION_STEP_SECONDS, "s"};

	s_settingRows[CHAOS_SETTING_TRIGGER] = (struct FullScreenMenu_Row){
		"Trigger", FULLSCREEN_ROW_CHOICE, s_chaosTriggerNames, CHAOS_TRIGGER_NAME_COUNT, 0, 0, NULL};

	const int intervalRow = Chaos_SettingIntervalRow();

	if (intervalRow >= 0)
	{
		s_settingRows[intervalRow] = (struct FullScreenMenu_Row){
			"Interval", FULLSCREEN_ROW_STEP, NULL, CHAOS_INTERVAL_MAX_SECONDS, CHAOS_INTERVAL_MIN_SECONDS,
			CHAOS_INTERVAL_STEP_SECONDS, "s"};
	}

	s_settingRows[Chaos_SettingEffectsRow()] = (struct FullScreenMenu_Row){
		"Effects", FULLSCREEN_ROW_ACTION, NULL, 0, 0, 0, NULL};

	s_settingRows[Chaos_SettingConfirmRow()] = (struct FullScreenMenu_Row){
		"Confirm", FULLSCREEN_ROW_ACTION, NULL, 0, 0, 0, NULL};
}

static struct FullScreenMenu_List s_settingList;

// The list works on its own ints and starts out as zeroes, so it has to be filled from the
// stored settings before the first frame writes them back. Done once per visit rather than on
// a value, because a duration of zero is a legal thing to store.
static bool s_settingsLoaded;

// The settings are the shape the file stores, which is u8 and u16. Reading and writing
// through the list rather than casting a stored address is what keeps a step from writing
// four bytes over the field beside it.
//
// The interval is read and written only while its row is there. A trigger that is not timed
// hides the row, and hiding it must not forget the setting: the row reads the stored value
// again when the trigger goes back to one of the two timed ones.
static void Chaos_SettingsToValues(void)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	const int intervalRow = Chaos_SettingIntervalRow();

	s_settingValues[CHAOS_SETTING_DURATION] = settings->durationSeconds;
	s_settingValues[CHAOS_SETTING_TRIGGER] = settings->trigger;

	if (intervalRow >= 0)
	{
		s_settingValues[intervalRow] = settings->triggerIntervalSeconds;
	}
}

static void Chaos_ValuesToSettings(void)
{
	struct ChaosSettings *settings = &sdata->chaosSettings;
	const int intervalRow = Chaos_SettingIntervalRow();

	settings->durationSeconds = (u16)s_settingValues[CHAOS_SETTING_DURATION];
	settings->trigger = (u8)s_settingValues[CHAOS_SETTING_TRIGGER];

	if (intervalRow >= 0)
	{
		settings->triggerIntervalSeconds = (u16)s_settingValues[intervalRow];
	}
}

// Refresh the list from the current new-file settings whenever the chain opens this screen.
void MM_ChaosConfig_OnEnter(void)
{
	s_settingsLoaded = false;
	s_page = CHAOS_PAGE_SETTINGS;
}

static void Chaos_EnsureSettingsLoaded(void)
{
	if (!s_settingsLoaded)
	{
		s_settingsLoaded = true;
		Chaos_SettingsToValues();
	}
}

// ---------------------------------------------------------------- effects

// One toggle per effect, in catalogue order, so the order on screen is the order the effects
// are declared in chaos_effect.h. The counts are toggles rather than a list of names
// because the catalogue already holds the label each one draws.
static struct FullScreenMenu_Row s_effectRows[CHAOS_EFFECT_COUNT];
static int s_effectValues[CHAOS_EFFECT_COUNT];

static void Chaos_BuildEffectRows(void)
{
	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		s_effectRows[i] = (struct FullScreenMenu_Row){
			ChaosEffect_Def(ChaosEffect_FromIndex(i))->label, FULLSCREEN_ROW_TOGGLE, NULL, 0, 0, 0, NULL};
	}
}

static struct FullScreenMenu_List s_effectList;

// The effects array is the catalogue's own order and so is this one, which lets a toggle
// here be the same toggle the engine uses rather than a second way of writing the array.
static void Chaos_EffectsToValues(void)
{
	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		s_effectValues[i] = ChaosEffect_IsEnabled(sdata->chaosSettings.effects, ChaosEffect_FromIndex(i)) ? 1 : 0;
	}
}

static void Chaos_ValuesToEffects(void)
{
	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		const ChaosEffectId id = ChaosEffect_FromIndex(i);

		if (ChaosEffect_IsEnabled(sdata->chaosSettings.effects, id) != (s_effectValues[i] != 0))
		{
			ChaosEffect_Toggle(sdata->chaosSettings.effects, &sdata->chaosSettings.enabledCount, id);
		}
	}
}

// ---------------------------------------------------------------- screen

// Return to the settings row that opened the effects page.
static void Chaos_LeaveEffectsPage(s16 *selected)
{
	// Written before the page changes, because the per-frame sync below picks the page it
	// syncs from.
	Chaos_ValuesToEffects();

	*selected = Chaos_SettingEffectsRow();
	s_page = CHAOS_PAGE_SETTINGS;
}

// The list for the page that is up, built fresh because a page can change while the input
// is being read and the frame still has to be drawn from whichever page is up at the end.
static struct FullScreenMenu_List *Chaos_PageList(const char **title)
{
	if (s_page == CHAOS_PAGE_EFFECTS)
	{
		Chaos_BuildEffectRows();

		s_effectList.rows = s_effectRows;
		s_effectList.numRows = CHAOS_EFFECT_COUNT;
		s_effectList.values = s_effectValues;

		*title = "CHAOS EFFECTS";
		return &s_effectList;
	}

	Chaos_BuildSettingRows();

	s_settingList.rows = s_settingRows;
	s_settingList.numRows = Chaos_SettingNumRows();
	s_settingList.values = s_settingValues;

	*title = "CHAOS SETUP";
	return &s_settingList;
}

void MM_MenuProc_ChaosConfig(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];

	// What this frame decided, carried out once the frame has been drawn. A screen that
	// leaves before drawing leaves a blank frame behind it, and on that blank frame
	// whatever is behind the box is what the player sees.
	bool leave = false;
	bool finish = false;

	Chaos_EnsureSettingsLoaded();

	const char *title;
	struct FullScreenMenu_List *list = Chaos_PageList(&title);

	s16 selected = menu->rowSelected;

	FullScreenMenu_NavList(pad, list, &selected);

	for (int direction = -1; direction <= 1; direction += 2)
	{
		if (FullScreenMenu_OptionStepDue(pad->buttonsHeldCurrFrame, direction))
		{
			FullScreenMenu_ListStep(list, selected, direction);
		}
	}

	// Cross answers the selected row, and every kind of row has an answer: a value row
	// cycles, the same way the options menu cycles a setting, and a row that is only an
	// action opens or confirms. The directions above step a value on their own.
	if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
	{
		OtherFX_Play(1, 1);

		if (!FullScreenMenu_RowIsAction(&list->rows[selected]))
		{
			FullScreenMenu_ListActivate(list, selected);
		}
		else if (selected == Chaos_SettingEffectsRow())
		{
			Chaos_SettingsToValues();
			Chaos_EffectsToValues();
			selected = 0;
			s_page = CHAOS_PAGE_EFFECTS;
		}
		else
		{
			finish = true;
		}
	}

	if ((pad->buttonsTapped & BTN_TRIANGLE) != 0)
	{
		OtherFX_Play(2, 1);

		if (s_page == CHAOS_PAGE_EFFECTS)
		{
			Chaos_LeaveEffectsPage(&selected);
		}
		else
		{
			leave = true;
		}
	}

	if (s_page == CHAOS_PAGE_EFFECTS)
	{
		// A toggle has to reach the array through ChaosEffect_Toggle, which is what keeps
		// the cached count in step with it.
		Chaos_ValuesToEffects();
	}
	else
	{
		Chaos_ValuesToSettings();
	}

	// Drawn from whichever page is up now, so the frame that changes page shows the page
	// being changed to rather than nothing at all.
	list = Chaos_PageList(&title);
	FullScreenMenu_ListFollow(list, &selected);

	menu->rowSelected = selected;

	FullScreenMenu_DrawScreen(title, 0, &list->scroll, list->numRows, selected,
	                          FullScreenMenu_DrawSettingRow, list, gGT, ot);

	if (finish)
	{
		MM_NewLoad_FinishChaosSetup();
	}
	else if (leave)
	{
		MM_NewLoad_ReturnFromChaosSetup();
	}
}

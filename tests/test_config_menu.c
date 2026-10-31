#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
#include <platform/native_config.c>

static struct GameTracker s_tracker;
static struct DB s_buffer;
static int s_configSaves;
static int s_controlsSaves;
static bool s_capturing;
static bool s_pickerOpen;
static NativeConfig s_initialConfig;
static struct RectMenu s_caller;
static const char *s_observedLabel;
static bool s_observingRow;
static bool s_valueDrawn;
static int s_valueFlags;
static char s_valueText[64];

// Count menu persistence calls without touching any real configuration file.
static void ConfigMenuTest_Save(void);
#define NativeConfig_Save ConfigMenuTest_Save
#include <game/230/MM_ConfigMenu.c>
#undef NativeConfig_Save

// The menu has no public page/scroll accessor; inspect those internals only to
// distinguish staying at the section list from accidentally reopening a page.

static void ConfigMenuTest_Save(void) { s_configSaves++; }
const char *NativeAssets_GetBaseDir(void)
{
	CtrTest_Bail("menu tests must not access configuration files");
	return NULL;
}
void NativeControls_Save(void) { s_controlsSaves++; }
void NativeControls_CancelCapture(void) { s_capturing = false; }
void MM_MenuProc_Controls(struct RectMenu *menu) { (void)menu; }
b32 MM_Controls_IsPickerOpen(void) { return s_pickerOpen; }
u32 MM_Controls_MenuTapped(void) { return (u32)sdata->gGamepads->gamepad[0].buttonsTapped; }
void MM_Controls_ResetNavigation(void)
{
	s_pickerOpen = false;
	NativeControls_CancelCapture();
}

void RECTMENU_ClearInput(void)
{
	sdata->AnyPlayerTap = 0;
	sdata->AnyPlayerHold = 0;
	memset(sdata->buttonTapPerPlayer, 0, sizeof(sdata->buttonTapPerPlayer));
	memset(sdata->buttonHeldPerPlayer, 0, sizeof(sdata->buttonHeldPerPlayer));
}

int OtherFX_Play(u32 id, int flags) { (void)id; (void)flags; return 0; }
void DecalFont_DrawLineOT(char *text, s16 x, s16 y, s16 font, s16 flags, u32 *ot)
{
	(void)y; (void)font; (void)ot;
	if (x == FULLSCREEN_MENU_LABEL_X)
	{
		s_observingRow = s_observedLabel != NULL && strcmp(text, s_observedLabel) == 0;
	}
	else if (x == FULLSCREEN_MENU_VALUE_X && s_observingRow)
	{
		s_valueDrawn = true;
		s_valueFlags = flags;
		snprintf(s_valueText, sizeof(s_valueText), "%s", text);
	}
}
void RECTMENU_DrawInnerRect(RECT *rect, int inset, u32 *ot)
{
	(void)rect; (void)inset; (void)ot;
}
void RECTMENU_DrawOuterRect_Edge(RECT *rect, const Color *color, u32 flags, u32 *ot)
{
	(void)rect; (void)color; (void)flags; (void)ot;
}
void CTR_Box_DrawClearBox(const RECT *rect, const Color *color, s32 transparency, u32 *ot, struct PrimMem *prims)
{
	(void)rect; (void)color; (void)transparency; (void)ot; (void)prims;
}

static void SetInput(int tapped, int held)
{
	sdata->gGamepads->gamepad[0].buttonsTapped = tapped;
	sdata->gGamepads->gamepad[0].buttonsHeldCurrFrame = held;
}

static void UpdateMenu(void)
{
	struct RectMenu *menu = sdata->ptrActiveMenu;
	menu->funcPtr(menu);
}

static void TestSession(enum ConfigMenuContext context, int mode)
{
	struct RectMenu caller = {.rowSelected = 5};
	s_tracker.gameMode1 = mode;
	s_capturing = true;
	s_pickerOpen = true;
	SetInput(BTN_CROSS, BTN_RIGHT);
	MM_ConfigMenu_Open(&caller, context);
	sdata->ptrActiveMenu = sdata->ptrDesiredMenu;
	sdata->ptrDesiredMenu = NULL;
	CTR_TEST_CHECK(!s_capturing && !s_pickerOpen, "opening clears stale capture and picker state");
	CTR_TEST_CHECK(sdata->ptrActiveMenu->rowSelected == 0, "opening selects the first section");
	UpdateMenu();
	CTR_TEST_CHECK(s_currentSection == -1, "opening confirm does not also enter a section");
	CTR_TEST_CHECK(s_tracker.gameMode1 == mode, "opening preserves the caller's game and pause flags");

	SetInput(BTN_CROSS, 0);
	sdata->ptrActiveMenu->rowSelected = 1; // Controls precedes General.
	UpdateMenu();
	CTR_TEST_CHECK(s_currentSection == 1, "confirm enters the selected section");
	// Back and confirm together must not reopen the section or edit an option.
	SetInput(BTN_TRIANGLE | BTN_CROSS, BTN_RIGHT);
	UpdateMenu();
	CTR_TEST_CHECK(s_currentSection == -1, "back consumes simultaneous confirm when leaving a section");
	CTR_TEST_CHECK(!g_config.skipIntro, "leaving a section does not edit its option");
	CTR_TEST_CHECK(s_configSaves == 0 && s_controlsSaves == 0, "leaving a section does not close the session");

	s_capturing = true;
	SetInput(BTN_START | BTN_CROSS, BTN_DOWN);
	UpdateMenu();
	CTR_TEST_CHECK(sdata->ptrDesiredMenu == &caller, "closing returns to the actual caller");
	CTR_TEST_CHECK(caller.rowSelected == 5, "closing preserves the caller's selected row");
	CTR_TEST_CHECK(s_tracker.gameMode1 == mode, "closing preserves pause until the caller resumes");
	CTR_TEST_CHECK(s_configSaves == 1 && s_controlsSaves == 1, "closing saves config and controls once");
	CTR_TEST_CHECK(!s_capturing, "closing cancels capture");
	CTR_TEST_CHECK(sdata->gGamepads->gamepad[0].buttonsTapped == 0 &&
	               sdata->AnyPlayerTap == 0 && sdata->AnyPlayerHold == 0,
	               "closing consumes menu input before returning");
	CTR_TEST_CHECK(sdata->gGamepads->gamepad[0].buttonsHeldCurrFrame == BTN_DOWN,
	               "closing preserves physical held state to avoid a synthetic tap next frame");
	s_configSaves = 0;
	s_controlsSaves = 0;
}

static int EntryValue(const ConfigEntry *entry)
{
	return entry->type == CFG_BOOL ? *(bool *)entry->valuePtr : *(int *)entry->valuePtr;
}

static void OpenEntry(int index, enum ConfigMenuContext context)
{
	MM_ConfigMenu_Open(&s_caller, context);
	sdata->ptrActiveMenu = sdata->ptrDesiredMenu;
	sdata->ptrDesiredMenu = NULL;
	for (int section = 0; section < s_numSections; section++)
	{
		const int first = s_sectionToEntry[section];
		if (index >= first && index < first + s_sectionCount[section])
		{
			sdata->ptrActiveMenu->rowSelected = (s16)section;
			SetInput(BTN_CROSS, 0);
			UpdateMenu();
			sdata->ptrActiveMenu->rowSelected = (s16)(index - first);
			return;
		}
	}
	CtrTest_Bail("every configured option must remain reachable");
}

static void TestAvailability(void)
{
	const char *const allowedKeys[] = {
		"controls", "skip_intro", "skip_podium_ceremonies", "show_reserves_meter",
		"skip_hints", "boss_cutscenes", "increase_draw_distance", "dithering",
		"fullscreen", "vsync", "aspect_ratio", "render_scale", "smooth_scaling",
		"fxaa", "texture_filtering", "pgxp",
	};
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		const ConfigEntry *entry = &g_configEntries[i];
		bool allowed = strcmp(entry->section, CONFIG_SECTION_DEVELOPER_HACKS) == 0;
		for (int k = 0; k < (int)len(allowedKeys); k++)
		{
			allowed |= strcmp(entry->key, allowedKeys[k]) == 0;
		}
		CTR_TEST_CHECK(entry->availableDuringPause == allowed, entry->key);
	}
}

static void TestEditing(enum ConfigMenuContext context)
{
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		const ConfigEntry *entry = &g_configEntries[i];
		if (entry->type == CFG_ACTION || entry->valuePtr == &g_config.developerHacksEnabled)
		{
			continue;
		}
		g_config = s_initialConfig;
		g_config.developerHacksEnabled = true;
		// Non-default values prove disabled rows display the current value, not a placeholder.
		char expectedText[64];
		if (entry->type == CFG_BOOL)
		{
			*(bool *)entry->valuePtr = true;
			snprintf(expectedText, sizeof(expectedText), "ON");
		}
		else if (entry->type == CFG_ENUM)
		{
			*(int *)entry->valuePtr = entry->enumValues[1].value;
			snprintf(expectedText, sizeof(expectedText), "%s", entry->enumValues[1].name);
		}
		else
		{
			*(int *)entry->valuePtr = entry->min + entry->step;
			snprintf(expectedText, sizeof(expectedText), "%d%%", EntryValue(entry));
		}
		const int before = EntryValue(entry);
		const bool editable = context == CONFIG_MENU_MAIN || entry->availableDuringPause;
		OpenEntry(i, context);
		s_observedLabel = entry->label;
		s_valueDrawn = false;
		SetInput(0, 0);
		UpdateMenu();
		CTR_TEST_CHECK(s_valueDrawn && strcmp(s_valueText, expectedText) == 0, entry->key);
		CTR_TEST_CHECK(s_valueFlags == (JUSTIFY_RIGHT | (editable ? WHITE : GRAY)),
		               "the value's color matches its editability");
		const int buttons[] = {BTN_CROSS, BTN_CIRCLE};
		for (int b = 0; b < (int)len(buttons); b++)
		{
			const int previous = EntryValue(entry);
			SetInput(buttons[b], 0);
			UpdateMenu();
			const bool shouldChange = editable && entry->type != CFG_INT;
			CTR_TEST_CHECK((EntryValue(entry) != previous) == shouldChange,
			               "confirm edits only enabled bool and enum values");
		}
		// Frame zero makes held-arrow repetition due for every numeric row.
		sdata->frameCounter = 0;
		SetInput(0, BTN_RIGHT);
		UpdateMenu();
		if (entry->type == CFG_INT)
		{
			CTR_TEST_CHECK(EntryValue(entry) == before + (editable ? entry->step : 0),
			               "right edits only enabled sliders");
			SetInput(0, BTN_LEFT);
			UpdateMenu();
			CTR_TEST_CHECK(EntryValue(entry) == before, "left edits only enabled sliders");
		}
		else if (!editable)
		{
			CTR_TEST_CHECK(EntryValue(entry) == before, "disabled values ignore all editing input");
		}
	}
	s_observedLabel = NULL;
}

static void TestDeveloperGate(void)
{
	g_config = s_initialConfig;
	g_config.developerHacksEnabled = true;
	g_config.oneLapRaces = true;
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		if (g_configEntries[i].valuePtr == &g_config.developerHacksEnabled)
		{
			OpenEntry(i, CONFIG_MENU_PAUSE);
			SetInput(BTN_CROSS, 0);
			UpdateMenu();
			CTR_TEST_CHECK(!g_config.developerHacksEnabled && !g_config.oneLapRaces,
			               "the developer gate can reset its options while paused");
			CTR_TEST_CHECK(s_currentSection == -1 && g_configMenu.rowSelected < s_numSections,
			               "hiding Developer Hacks returns to a valid section selection");
			SetInput(0, 0);
			UpdateMenu();
			for (int section = 0; section < s_numSections; section++)
			{
				CTR_TEST_CHECK(strcmp(g_configEntries[s_sectionToEntry[section]].section,
				                      CONFIG_SECTION_DEVELOPER_HACKS) != 0,
				               "the disabled developer section is hidden");
			}
			return;
		}
	}
	CtrTest_Bail("the developer gate must exist");
}

int main(void)
{
	// Other unity-build menus use these shared helpers; this suite tests Config only.
	(void)FullScreenMenu_RowActivate;
	(void)FullScreenMenu_RowStep;
	(void)FullScreenMenu_DrawRowValue;
	sdata->gGT = &s_tracker;
	s_initialConfig = g_config;
	sdata->gGamepads = &sdata->gamepadSystem;
	s_tracker.backBuffer = &s_buffer;
	CTR_TEST_SECTION("Main-menu session");
	TestSession(CONFIG_MENU_MAIN, MAIN_MENU);
	CTR_TEST_SECTION("Paused race session");
	TestSession(CONFIG_MENU_PAUSE, ARCADE_MODE | PAUSE_1);
	CTR_TEST_SECTION("Reopening resets section navigation");
	struct RectMenu caller = {0};
	MM_ConfigMenu_Open(&caller, CONFIG_MENU_MAIN);
	sdata->ptrActiveMenu = sdata->ptrDesiredMenu;
	SetInput(BTN_CROSS, 0);
	UpdateMenu();
	MM_ConfigMenu_Open(&caller, CONFIG_MENU_PAUSE);
	CTR_TEST_CHECK(s_currentSection == -1 && s_configScroll.offset == 0,
	               "a new session starts at the section list with reset scrolling");
	CTR_TEST_SECTION("Pause availability policy");
	TestAvailability();
	CTR_TEST_SECTION("Main-menu editing and values");
	TestEditing(CONFIG_MENU_MAIN);
	CTR_TEST_SECTION("Paused editing, disabled values and colors");
	TestEditing(CONFIG_MENU_PAUSE);
	CTR_TEST_SECTION("Disabling Developer Hacks while paused");
	TestDeveloperGate();
	return CtrTest_Summary();
}

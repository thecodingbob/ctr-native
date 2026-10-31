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
static bool s_sliderDrawn;
static RECT s_sliderMarker;
static s16 s_sliderTriangle[6];
static int s_sliderBoxCount;
static int s_volumes[GAME_OPTIONS_VOLUME_COUNT];
static int s_stereo;
static int s_previewRow;
static bool s_previewPlaying;
static char *s_strings[LNG_STEREO + 1];

// Count menu persistence calls without touching any real configuration file.
static void ConfigMenuTest_Save(void);
#define NativeConfig_Save ConfigMenuTest_Save
#include <game/230/MM_ConfigMenu.c>
#undef NativeConfig_Save
#include <game/230/MM_AudioConfig.c>
#include <game/UI/UI_VolumeSlider.c>
#include <game/RaceConfig.c>

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
s32 howl_VolumeGet(s32 type) { return s_volumes[type]; }
void howl_VolumeSet(s32 type, u8 volume) { s_volumes[type] = volume; }
s32 howl_ModeGet(void) { return s_stereo; }
void howl_ModeSet(s32 mode) { s_stereo = mode; }
void OptionsMenu_TestSound(s32 row, b32 playing)
{
	s_previewRow = row;
	s_previewPlaying = playing;
}
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
		s_sliderBoxCount = 0;
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

void CTR_Box_DrawSolidBox(RECT *rect, const Color *color, u32 *ot, struct PrimMem *prims)
{
	(void)color; (void)ot; (void)prims;
	if (s_observingRow && s_sliderBoxCount++ == 0)
	{
		s_sliderMarker = *rect;
	}
}

void RECTMENU_DrawRwdTriangle(s16 *position, char *color, u32 *ot, struct PrimMem *prims)
{
	(void)color; (void)ot; (void)prims;
	if (s_observingRow)
	{
		s_sliderDrawn = true;
		memcpy(s_sliderTriangle, position, sizeof(s_sliderTriangle));
	}
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
	int generalSection = -1;
	for (int section = 0; section < s_numSections; section++)
	{
		if (strcmp(g_configEntries[s_sectionToEntry[section]].section, "General") == 0)
		{
			generalSection = section;
		}
	}
	CTR_TEST_CHECK(generalSection >= 0, "the General section is present");
	sdata->ptrActiveMenu->rowSelected = (s16)generalSection;
	UpdateMenu();
	CTR_TEST_CHECK(s_currentSection == generalSection, "confirm enters the selected section");
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
		"audio", "controls", "skip_intro", "skip_podium_ceremonies", "show_reserves_meter",
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

static void TestAudio(enum ConfigMenuContext context, int gameMode)
{
	s_tracker.gameMode1 = gameMode;
	s_volumes[HOWL_VOLUME_TYPE_FX] = 128;
	s_volumes[HOWL_VOLUME_TYPE_MUSIC] = 64;
	s_volumes[HOWL_VOLUME_TYPE_VOICE] = 192;
	s_stereo = true;
	int audioEntry = -1;
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		if (strcmp(g_configEntries[i].section, CONFIG_SECTION_AUDIO) == 0)
		{
			audioEntry = i;
		}
	}
	CTR_TEST_CHECK(audioEntry >= 0, "Audio is available in Options");
	OpenEntry(audioEntry, context);
	const char *const labels[] = {"FX", "MUSIC", "VOICE"};
	for (int row = 0; row < GAME_OPTIONS_VOLUME_COUNT; row++)
	{
		s_observedLabel = labels[row];
		s_sliderDrawn = false;
		SetInput(row == 0 ? 0 : BTN_DOWN, 0);
		UpdateMenu();
		CTR_TEST_CHECK(s_sliderDrawn, "each colon-free audio label is paired with an original-style slider");
		CTR_TEST_CHECK(s_sliderTriangle[0] == AUDIO_CONFIG_SLIDER_X &&
		               s_sliderTriangle[2] == FULLSCREEN_MENU_VALUE_X,
		               "the original slider shape fits inside Audio's value column");
		const int markerTravel = FULLSCREEN_MENU_VALUE_X - AUDIO_CONFIG_SLIDER_X - 5;
		CTR_TEST_CHECK(s_sliderMarker.x == AUDIO_CONFIG_SLIDER_X + s_volumes[row] * markerTravel / 256 + 1,
		               "the original slider marker displays the live channel volume");
		if (row == HOWL_VOLUME_TYPE_MUSIC && context == CONFIG_MENU_MAIN)
		{
			CTR_TEST_CHECK(!s_previewPlaying, "the main menu's own music supplies feedback without an extra preview");
		}
		else
		{
			CTR_TEST_CHECK(s_previewPlaying && s_previewRow == row, "other volume rows select their original preview");
		}
		int before[GAME_OPTIONS_VOLUME_COUNT];
		memcpy(before, s_volumes, sizeof(before));
		SetInput(0, BTN_RIGHT);
		UpdateMenu();
		for (int channel = 0; channel < GAME_OPTIONS_VOLUME_COUNT; channel++)
		{
			CTR_TEST_CHECK(s_volumes[channel] == before[channel] + (channel == row ? 4 : 0),
			               "a held direction edits only the selected audio channel");
		}
		SetInput(BTN_CROSS | BTN_CIRCLE, 0);
		UpdateMenu();
		CTR_TEST_CHECK(s_volumes[row] == before[row] + 4, "confirm does not edit volume sliders");
	}
	s_volumes[HOWL_VOLUME_TYPE_VOICE] = 254;
	SetInput(0, BTN_RIGHT);
	UpdateMenu();
	CTR_TEST_CHECK(s_volumes[HOWL_VOLUME_TYPE_VOICE] == 255, "volume clamps at full scale");
	s_volumes[HOWL_VOLUME_TYPE_VOICE] = 1;
	SetInput(0, BTN_LEFT | BTN_RIGHT);
	UpdateMenu();
	CTR_TEST_CHECK(s_volumes[HOWL_VOLUME_TYPE_VOICE] == 0, "Left takes priority and volume clamps at silence");

	s_observedLabel = "MODE";
	s_valueDrawn = false;
	SetInput(BTN_DOWN, 0);
	UpdateMenu();
	CTR_TEST_CHECK(!s_previewPlaying, "the mode row stops volume preview");
	CTR_TEST_CHECK(s_valueDrawn && strcmp(s_valueText, s_strings[LNG_STEREO]) == 0,
	               "the colon-free mode row displays the live stereo mode");
	SetInput(BTN_CROSS, 0);
	UpdateMenu();
	CTR_TEST_CHECK(!s_stereo && strcmp(s_valueText, s_strings[LNG_MONO]) == 0, "Cross selects and displays mono");
	SetInput(BTN_CIRCLE, 0);
	UpdateMenu();
	CTR_TEST_CHECK(s_stereo, "Circle selects stereo again");

	SetInput(BTN_UP, 0);
	UpdateMenu();
	CTR_TEST_CHECK(s_previewPlaying, "returning to a volume row restarts its preview");
	SetInput(BTN_TRIANGLE | BTN_CROSS, BTN_RIGHT);
	UpdateMenu();
	CTR_TEST_CHECK(!s_previewPlaying && s_currentSection == -1, "leaving Audio stops preview and returns to sections");
	CTR_TEST_CHECK(s_tracker.gameMode1 == gameMode, "Audio preserves the session's game and pause flags");
	for (int channel = 0; channel < GAME_OPTIONS_VOLUME_COUNT; channel++)
	{
		CTR_TEST_CHECK(GAME_SAVE.options.volumes[channel] == s_volumes[channel],
		               "leaving Audio captures volume in the existing game options");
	}
	CTR_TEST_CHECK(GAME_SAVE.options.audioMode == s_stereo, "leaving Audio captures stereo in the existing game options");
	memset(s_volumes, 0, sizeof(s_volumes));
	s_stereo = false;
	sdata->boolHasLoadedOptions = false;
	RaceConfig_LoadGameOptions();
	CTR_TEST_CHECK(s_volumes[HOWL_VOLUME_TYPE_FX] == 132 && s_volumes[HOWL_VOLUME_TYPE_MUSIC] == 68 &&
	               s_volumes[HOWL_VOLUME_TYPE_VOICE] == 0 && s_stereo,
	               "the existing game-options loader restores Audio's saved values");

	OpenEntry(audioEntry, context);
	SetInput(0, 0);
	UpdateMenu();
	MM_ConfigMenu_Open(&s_caller, context);
	CTR_TEST_CHECK(!s_previewPlaying, "reopening Options from Audio also stops its preview");
	CTR_TEST_CHECK(strcmp(s_strings[LNG_FX], "FX:") == 0 && strcmp(s_strings[LNG_MODE], "MODE:") == 0,
	               "Audio leaves the shared localized strings intact");
	s_observedLabel = NULL;
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
	sdata->lngStrings = s_strings;
	s_strings[LNG_OPTIONS] = "OPTIONS";
	s_strings[LNG_FX] = "FX:";
	s_strings[LNG_MUSIC] = "MUSIC:";
	s_strings[LNG_VOICE] = "VOICE:";
	s_strings[LNG_MODE] = "MODE:";
	s_strings[LNG_MONO] = "MONO";
	s_strings[LNG_STEREO] = "STEREO";
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
	CTR_TEST_SECTION("Audio from the main menu");
	TestAudio(CONFIG_MENU_MAIN, MAIN_MENU);
	CTR_TEST_SECTION("Audio while paused");
	TestAudio(CONFIG_MENU_PAUSE, ARCADE_MODE | PAUSE_1);
	return CtrTest_Summary();
}

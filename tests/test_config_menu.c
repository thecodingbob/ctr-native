#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
NativeConfig g_config;
const ConfigEntry g_configEntries[] = {
	{.section = "General", .label = "Skip Intros", .type = CFG_BOOL, .valuePtr = &g_config.skipIntro},
};
const int g_numConfigEntries = len(g_configEntries);

static struct GameTracker s_tracker;
static struct DB s_buffer;
static int s_configSaves;
static int s_controlsSaves;
static bool s_capturing;
static bool s_pickerOpen;

#include <game/230/MM_ConfigMenu.c>

// The menu has no public page/scroll accessor; inspect those internals only to
// distinguish staying at the section list from accidentally reopening a page.

void NativeConfig_Save(void) { s_configSaves++; }
void NativeControls_Save(void) { s_controlsSaves++; }
void NativeControls_CancelCapture(void) { s_capturing = false; }
void NativeConfig_ApplyDependencies(const ConfigEntry *entry) { (void)entry; }
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
	(void)text; (void)x; (void)y; (void)font; (void)flags; (void)ot;
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
	UpdateMenu();
	CTR_TEST_CHECK(s_currentSection == 0, "confirm enters the selected section");
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

int main(void)
{
	// Other unity-build menus use these shared helpers; this suite tests Config only.
	(void)FullScreenMenu_RowActivate;
	(void)FullScreenMenu_RowStep;
	(void)FullScreenMenu_DrawRowValue;
	sdata->gGT = &s_tracker;
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
	return CtrTest_Summary();
}

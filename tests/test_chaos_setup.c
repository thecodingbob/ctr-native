#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
static struct GameTracker s_tracker;
static struct DB s_buffer;
static int s_drawnRows;
static int s_drawnFrames;
static const char *s_drawnTitle;
static bool s_drewInterval;
static bool s_drewEffects;
static bool s_drewConfirm;
static int s_finishes;
static int s_returns;
static bool s_drewBeforeExit;

void MM_NewLoad_FinishChaosSetup(void)
{
	s_finishes++;
	s_drewBeforeExit = s_drawnFrames != 0 && s_drawnRows != 0;
}

void MM_NewLoad_ReturnFromChaosSetup(void)
{
	s_returns++;
	s_drewBeforeExit = s_drawnFrames != 0 && s_drawnRows != 0;
}

// Exercise the real input callback; drawing doubles record submissions, not pixels.
#include <game/230/MM_ChaosConfig.c>

int OtherFX_Play(u32 soundID, int flags)
{
	(void)soundID;
	(void)flags;
	return 0;
}

void DecalFont_DrawLineOT(char *text, s16 x, s16 y, s16 font, s16 flags, u32 *ot)
{
	(void)y;
	(void)font;
	(void)flags;
	(void)ot;
	if (x == FULLSCREEN_MENU_TITLE_X)
	{
		s_drawnTitle = text;
	}
	else if (x == FULLSCREEN_MENU_LABEL_X)
	{
		s_drawnRows++;
		s_drewInterval |= strcmp(text, "Interval") == 0;
		s_drewEffects |= strcmp(text, "Effects") == 0;
		s_drewConfirm |= strcmp(text, "Confirm") == 0;
	}
}

void RECTMENU_DrawInnerRect(RECT *rect, int inset, u32 *ot)
{
	(void)rect;
	(void)inset;
	(void)ot;
	s_drawnFrames++;
}

void RECTMENU_DrawOuterRect_Edge(RECT *rect, const Color *color, u32 flags, u32 *ot)
{
	(void)rect;
	(void)color;
	(void)flags;
	(void)ot;
}

void CTR_Box_DrawClearBox(const RECT *rect, const Color *color, s32 transparency, u32 *ot, struct PrimMem *prims)
{
	(void)rect;
	(void)color;
	(void)transparency;
	(void)ot;
	(void)prims;
}

static void EnterSetup(void)
{
	memset(&sdata_static, 0, sizeof(sdata_static));
	memset(&s_tracker, 0, sizeof(s_tracker));
	memset(&s_buffer, 0, sizeof(s_buffer));
	sdata->gGT = &s_tracker;
	s_tracker.backBuffer = &s_buffer;
	sdata->gGamepads = &sdata->gamepadSystem;
	sdata->chaosSettings.durationSeconds = CHAOS_DURATION_DEFAULT_SECONDS;
	sdata->chaosSettings.triggerIntervalSeconds = CHAOS_TRIGGER_INTERVAL_DEFAULT_SECONDS;
	sdata->chaosSettings.armed = true;
	sdata->chaosSettings.enabledCount = CHAOS_EFFECT_COUNT;
	for (int effect = 0; effect < CHAOS_EFFECT_COUNT; effect++)
	{
		sdata->chaosSettings.effects[effect] = true;
	}
	s_finishes = 0;
	s_returns = 0;
	s_drewBeforeExit = false;
	MM_ChaosConfig_OnEnter();
}

static void Frame(int tapped, int held)
{
	s_drawnRows = 0;
	s_drawnFrames = 0;
	s_drawnTitle = NULL;
	s_drewInterval = false;
	s_drewEffects = false;
	s_drewConfirm = false;
	sdata->gGamepads->gamepad[0].buttonsTapped = tapped;
	sdata->gGamepads->gamepad[0].buttonsHeldCurrFrame = held;
	MM_MenuProc_ChaosConfig(&MM_menuChaosConfig);
}

static void TestTriggerLayoutAndIntervalRetention(void)
{
	CTR_TEST_SECTION("trigger changes preserve the hidden interval and keep both action rows");
	EnterSetup();
	MM_menuChaosConfig.rowSelected = 2;
	Frame(0, BTN_RIGHT);
	CTR_TEST_CHECK_EQ(sdata->chaosSettings.triggerIntervalSeconds, 25, "the numeric interval commits immediately");
	MM_menuChaosConfig.rowSelected = 1;
	for (int trigger = CHAOS_TRIGGER_PICKUP; trigger <= CHAOS_TRIGGER_COUNT; trigger++)
	{
		Frame(BTN_CROSS, 0);
		const int expectedTrigger = trigger % CHAOS_TRIGGER_COUNT;
		const bool timed = expectedTrigger == CHAOS_TRIGGER_RANDOM || expectedTrigger == CHAOS_TRIGGER_INTERVAL;
		CTR_TEST_CHECK_EQ(sdata->chaosSettings.trigger, expectedTrigger, "Cross cycles the stored trigger");
		CTR_TEST_CHECK_EQ(s_drawnRows, timed ? 5 : 4, "the destination layout is drawn in the same frame");
		CTR_TEST_CHECK(s_drewInterval == timed, "Interval is present only for timed triggers");
		CTR_TEST_CHECK(s_drewEffects && s_drewConfirm, "Effects and Confirm remain visible after every trigger change");
		CTR_TEST_CHECK_EQ(sdata->chaosSettings.triggerIntervalSeconds, 25, "hiding Interval never loses its value");
	}
}

static void TestEffectsCommitAndPageTransitions(void)
{
	CTR_TEST_SECTION("effect toggles commit directly and Back draws the destination page");
	EnterSetup();
	MM_menuChaosConfig.rowSelected = 3;
	Frame(BTN_CROSS, 0);
	CTR_TEST_CHECK(s_drawnTitle != NULL && strcmp(s_drawnTitle, "CHAOS EFFECTS") == 0, "opening Effects draws its title immediately");
	CTR_TEST_CHECK_EQ(s_drawnRows, FULLSCREEN_MENU_MAX_VISIBLE_ROWS, "Effects uses the shared visible-row limit");
	MM_menuChaosConfig.rowSelected = CHAOS_EFFECT_LOW_GRAVITY;
	Frame(BTN_CROSS, 0);
	CTR_TEST_CHECK(!sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY], "Cross updates the stored effect without leaving its page");
	CTR_TEST_CHECK_EQ(sdata->chaosSettings.enabledCount, CHAOS_EFFECT_COUNT - 1, "the cached count changes with the toggle");
	Frame(BTN_TRIANGLE, 0);
	CTR_TEST_CHECK(s_drawnTitle != NULL && strcmp(s_drawnTitle, "CHAOS SETUP") == 0, "Back draws Settings immediately");
	CTR_TEST_CHECK_EQ(MM_menuChaosConfig.rowSelected, 3, "Back selects the Effects action");
	CTR_TEST_CHECK_EQ(s_drawnRows, 5, "the return frame contains all settings rows");
	CTR_TEST_CHECK_EQ(s_returns, 0, "Effects Back stays inside the setup screen");
	MM_menuChaosConfig.rowSelected = 4;
	Frame(BTN_CROSS, 0);
	CTR_TEST_CHECK_EQ(s_finishes, 1, "Confirm proceeds once");
	CTR_TEST_CHECK(s_drewBeforeExit, "Confirm draws before handing control back to the chain");
}

static void TestLiveValuesAndInputKinds(void)
{
	CTR_TEST_SECTION("rendering reads live values and input changes only the intended field");
	EnterSetup();
	sdata->chaosSettings.durationSeconds = 35;
	sdata->chaosSettings.triggerIntervalSeconds = 65;
	unsigned char before[sizeof(struct ChaosSettings)];
	memcpy(before, &sdata->chaosSettings, sizeof(before));
	Frame(0, 0);
	CTR_TEST_CHECK(memcmp(before, &sdata->chaosSettings, sizeof(before)) == 0, "an idle frame cannot overwrite live settings with cached values");
	Frame(BTN_CROSS, 0);
	CTR_TEST_CHECK_EQ(sdata->chaosSettings.durationSeconds, 35, "Cross does not step numeric values");
	Frame(0, BTN_RIGHT);
	CTR_TEST_CHECK_EQ(sdata->chaosSettings.durationSeconds, 40, "a slider steps from the live value rather than a mirror");
	MM_menuChaosConfig.rowSelected = 1;
	Frame(0, BTN_RIGHT);
	CTR_TEST_CHECK_EQ(sdata->chaosSettings.trigger, CHAOS_TRIGGER_RANDOM, "directions do not cycle enum choices");
	Frame(BTN_TRIANGLE, 0);
	CTR_TEST_CHECK_EQ(s_returns, 1, "Settings Back returns to the chain once");
	CTR_TEST_CHECK(s_drewBeforeExit, "Settings Back draws before handing control back to the chain");
	MM_ChaosConfig_OnEnter();
	CTR_TEST_CHECK_EQ(MM_menuChaosConfig.rowSelected, 0, "a new visit starts on Duration");
	CTR_TEST_CHECK_EQ(sdata->chaosSettings.durationSeconds, 40, "re-entering setup preserves the current creation's values");
}

int main(void)
{
	TestTriggerLayoutAndIntervalRetention();
	TestEffectsCommitAndPageTransitions();
	TestLiveValuesAndInputKinds();
	return CtrTest_Summary();
}

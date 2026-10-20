#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
struct rData rdata;
struct OverlayDATA_230 D230;
NativeConfig g_config;
struct RectMenu MM_menuChaosConfig;

// Match the overlay's aliases without including its renderer and asset loaders.
#define MM_MENU_MAIN (D230.menuMainMenu)
#define MM_TITLE_MENU_STATE (D230.titleMenuState)
#define MM_MENU_PLAYERS_1P2P (D230.menuPlayers1P2P)
#define MM_MENU_ADVENTURE (D230.menuAdventure)
#define MM_MENU_CHAOS_PROMPT (D230.menuChaosPrompt)
#define MM_MENU_CHAOS_CONFIG (MM_menuChaosConfig)
#define MM_DESIRED_MENU_INDEX (D230.desiredMenuIndex)

static int s_setupEntries;

void MM_ChaosConfig_OnEnter(void)
{
	s_setupEntries++;
}

void MM_JumpTo_Title_Returning(void)
{
}

b32 MainGameEnd_IsCoopRace(struct GameTracker *gGT)
{
	(void)gGT;
	return false;
}

#include <game/GAMEPROG.c>
// Begin is file-static: include its actual module to catch a missing reset at the caller.
#include <game/230/MM_NewLoad.c>
#include <game/230/MM_Chaos.c>

static void ResetFixture(void)
{
	memset(&sdata_static, 0, sizeof(sdata_static));
	memset(&D230, 0, sizeof(D230));
	memset(&g_config, 0, sizeof(g_config));
	sdata->ptrActiveMenu = &MM_MENU_MAIN;
	s_setupEntries = 0;
}

static void CheckFreshSettings(void)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	CTR_TEST_CHECK(!settings->armed, "a new file starts Regular");
	CTR_TEST_CHECK_EQ(settings->durationSeconds, CHAOS_DURATION_DEFAULT_SECONDS, "duration starts at its default");
	CTR_TEST_CHECK_EQ(settings->triggerIntervalSeconds, CHAOS_TRIGGER_INTERVAL_DEFAULT_SECONDS, "interval starts at its default");
	CTR_TEST_CHECK_EQ(settings->trigger, CHAOS_TRIGGER_RANDOM, "trigger starts at its default");
	CTR_TEST_CHECK_EQ(settings->enabledCount, CHAOS_EFFECT_COUNT, "all default effects are counted");
	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		CTR_TEST_CHECK(settings->effects[i], "every effect starts selected");
	}
	CTR_TEST_CHECK_EQ(settings->enabledCount, ChaosEffect_CountEnabled(settings->effects), "the cached count matches the effect array");
	CTR_TEST_CHECK_EQ(s_setupEntries, 0, "New does not inherit a previous file's Chaos setup handover");
}

static void TestNewAfterChaosCreation(void)
{
	CTR_TEST_SECTION("New resets a previous Chaos creation with either optional step enabled or disabled");
	for (int prompt = 0; prompt <= 1; prompt++)
	{
		for (int players = 0; players <= 1; players++)
		{
			ResetFixture();
			g_config.chaosMode = true;
			MM_NewLoad_Begin(&MM_MENU_ADVENTURE);
			MM_MENU_CHAOS_PROMPT.funcState = RECTMENU_FUNC_STATE_INPUT;
			MM_MENU_CHAOS_PROMPT.rowSelected = 0;
			MM_MenuProc_Chaos(&MM_MENU_CHAOS_PROMPT);
			CTR_TEST_CHECK(sdata->chaosSettings.armed, "the preceding creation really selected Chaos");
			CTR_TEST_CHECK_EQ(s_setupEntries, 1, "the preceding creation reached Chaos setup");

			sdata->chaosSettings.durationSeconds = 45;
			sdata->chaosSettings.triggerIntervalSeconds = 90;
			sdata->chaosSettings.trigger = CHAOS_TRIGGER_PICKUP;
			ChaosEffect_Toggle(sdata->chaosSettings.effects, &sdata->chaosSettings.enabledCount, CHAOS_EFFECT_LOW_GRAVITY);

			g_config.chaosMode = prompt != 0;
			g_config.multiplayerAdventure = players != 0;
			s_setupEntries = 0;
			sdata->ptrDesiredMenu = NULL;
			// Clearing only armed here would leave the previous file's custom settings.
			MM_NewLoad_Begin(&MM_MENU_ADVENTURE);
			CheckFreshSettings();
			CTR_TEST_CHECK(MM_NewLoad_InChain() == (prompt || players), "New follows only the enabled creation steps");
		}
	}
}

static void TestNewAfterLoadedChaosSettings(void)
{
	CTR_TEST_SECTION("New with no prompt discards the live settings of a loaded Chaos file");
	ResetFixture();
	// Reproduce the live state left by loading a slot; disk I/O is outside this suite.
	struct ChaosSettings saved = {
		.durationSeconds = 55,
		.triggerIntervalSeconds = 115,
		.trigger = CHAOS_TRIGGER_EXTERNAL,
		.armed = true,
	};
	saved.effects[CHAOS_EFFECT_HARD_CRATES] = true;
	saved.enabledCount = (u8)ChaosEffect_CountEnabled(saved.effects);
	sdata->chaosSettings = saved;

	MM_NewLoad_Begin(&MM_MENU_ADVENTURE);
	CheckFreshSettings();
	CTR_TEST_CHECK_EQ(MM_DESIRED_MENU_INDEX, MM_EXIT_ROUTE_ADV_NEW, "New continues through the normal creation route");
}

int main(void)
{
	TestNewAfterChaosCreation();
	TestNewAfterLoadedChaosSettings();
	return CtrTest_Summary();
}

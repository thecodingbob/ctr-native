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
static int s_titleReturns;

void MM_ChaosConfig_OnEnter(void)
{
	s_setupEntries++;
}

void MM_JumpTo_Title_Returning(void)
{
	s_titleReturns++;
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
	s_titleReturns = 0;
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

static void ReachCharacterSelect(bool prompt, bool chaos, bool players, int playerCount, bool extended)
{
	ResetFixture();
	g_config.chaosMode = prompt;
	g_config.multiplayerAdventure = players;
	g_config.extendedAdventureCharacterSelect = extended;
	MM_NewLoad_Begin(&MM_MENU_ADVENTURE);
	if (prompt)
	{
		MM_MENU_CHAOS_PROMPT.funcState = RECTMENU_FUNC_STATE_INPUT;
		MM_MENU_CHAOS_PROMPT.rowSelected = chaos ? 0 : 1;
		MM_MenuProc_Chaos(&MM_MENU_CHAOS_PROMPT);
	}
	if (players)
	{
		MM_MENU_PLAYERS_1P2P.rowSelected = playerCount - 1;
		MM_NewLoad_Advance(&MM_MENU_PLAYERS_1P2P);
	}
	if (chaos)
	{
		MM_NewLoad_FinishChaosSetup();
	}
	// MM_Title captures the player count before character selection starts.
	sdata->advMultiplayer.numPlayers = playerCount;
}

static void TestCharacterSelectBackPreservesChoices(void)
{
	CTR_TEST_SECTION("character-select Back restores Mode and Players after an overlay reset");
	for (int chaos = 0; chaos <= 1; chaos++)
	{
		for (int players = 1; players <= 2; players++)
		{
			ReachCharacterSelect(true, chaos != 0, true, players, true);
			memset(&D230, 0, sizeof(D230));
			MM_NewLoad_OnCharacterSelectExit(false);
			CTR_TEST_CHECK_EQ(s_titleReturns, 1, "Back starts the title return transition once");
			CTR_TEST_CHECK_EQ(MM_MENU_CHAOS_PROMPT.rowSelected, chaos ? 0 : 1, "the collapsed Mode row keeps the chosen mode");
			CTR_TEST_CHECK_EQ(MM_MENU_PLAYERS_1P2P.rowSelected, players - 1, "Players keeps the chosen player count");
			CTR_TEST_CHECK(MM_MENU_ADVENTURE.ptrNextBox_InHierarchy == &MM_MENU_CHAOS_PROMPT,
			               "the restored chain includes Mode before Players");
			CTR_TEST_CHECK(MM_MENU_CHAOS_PROMPT.ptrNextBox_InHierarchy == &MM_MENU_PLAYERS_1P2P,
			               "Players is attached below the chosen mode");
			CTR_TEST_CHECK(MM_MENU_PLAYERS_1P2P.ptrPrevBox_InHierarchy == &MM_MENU_CHAOS_PROMPT,
			               "Players can retreat without depending on a render pass");
			CTR_TEST_CHECK((MM_MENU_PLAYERS_1P2P.state & ONLY_DRAW_TITLE) == 0, "the last choice is editable");
			CTR_TEST_CHECK(sdata->chaosSettings.armed == (chaos != 0), "Back preserves the actual mode as well as its display");

			MM_NewLoad_Retreat(&MM_MENU_PLAYERS_1P2P);
			CTR_TEST_CHECK_EQ(MM_MENU_CHAOS_PROMPT.rowSelected, chaos ? 0 : 1, "retreat to Mode keeps its previous selection");
			CTR_TEST_CHECK(MM_MENU_CHAOS_PROMPT.ptrNextBox_InHierarchy == NULL, "retreat removes the Players link");
			MM_NewLoad_Retreat(&MM_MENU_CHAOS_PROMPT);
			CTR_TEST_CHECK(!MM_NewLoad_InChain(), "retreat past Mode finishes the chain");
			CTR_TEST_CHECK(MM_MENU_ADVENTURE.ptrNextBox_InHierarchy == NULL, "no stale child remains on New/Load");
		}
	}
}

static void TestCharacterSelectBackWithOptionalSteps(void)
{
	CTR_TEST_SECTION("Back returns to the last enabled step");
	ReachCharacterSelect(false, false, true, 2, true);
	MM_NewLoad_OnCharacterSelectExit(false);
	CTR_TEST_CHECK(MM_MENU_ADVENTURE.ptrNextBox_InHierarchy == &MM_MENU_PLAYERS_1P2P, "a disabled prompt is not reconstructed");
	CTR_TEST_CHECK_EQ(MM_MENU_PLAYERS_1P2P.rowSelected, 1, "2P survives a return with no Mode step");
	MM_NewLoad_Retreat(&MM_MENU_PLAYERS_1P2P);

	ReachCharacterSelect(true, false, false, 1, true);
	MM_NewLoad_OnCharacterSelectExit(false);
	CTR_TEST_CHECK(MM_MENU_ADVENTURE.ptrNextBox_InHierarchy == &MM_MENU_CHAOS_PROMPT, "Mode is the last choice when Players is disabled");
	CTR_TEST_CHECK_EQ(MM_MENU_CHAOS_PROMPT.rowSelected, 1, "Regular remains selected without a Players step");
	MM_NewLoad_Retreat(&MM_MENU_CHAOS_PROMPT);

	ReachCharacterSelect(false, false, false, 1, true);
	MM_NewLoad_OnCharacterSelectExit(false);
	CTR_TEST_CHECK(!MM_NewLoad_InChain(), "no chain is invented when both steps are disabled");
	CTR_TEST_CHECK_EQ(s_titleReturns, 1, "an ordinary Back still returns to the title");
}

static void TestCharacterSelectForwardConsumesReturn(void)
{
	CTR_TEST_SECTION("finishing creation cannot redirect a later character-select Back");
	ReachCharacterSelect(true, true, true, 2, true);
	MM_NewLoad_OnCharacterSelectExit(true);
	CTR_TEST_CHECK_EQ(s_titleReturns, 0, "proceeding does not start a title return");
	MM_NewLoad_OnCharacterSelectExit(false);
	CTR_TEST_CHECK(!MM_NewLoad_InChain(), "a later Back cannot restore the completed Adventure chain");
	CTR_TEST_CHECK(MM_MENU_ADVENTURE.ptrNextBox_InHierarchy == NULL, "a later mode does not gain an Adventure submenu");
	CTR_TEST_CHECK_EQ(s_titleReturns, 1, "a later Back follows the ordinary title return");

	ReachCharacterSelect(true, false, true, 2, false);
	MM_NewLoad_OnCharacterSelectExit(false);
	CTR_TEST_CHECK(!MM_NewLoad_InChain(), "the garage route leaves no extended-character-select return context");
}

int main(void)
{
	TestNewAfterChaosCreation();
	TestNewAfterLoadedChaosSettings();
	TestCharacterSelectBackPreservesChoices();
	TestCharacterSelectBackWithOptionalSteps();
	TestCharacterSelectForwardConsumesReturn();
	return CtrTest_Summary();
}

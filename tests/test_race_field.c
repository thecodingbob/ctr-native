// Pins the race-field rules in include/race_field.h: how many racers each kind
// of race fields, and how many of those are bots.

#include <shared/ctr_test.h>
#include <common.h>
#include <race_field.h>
#include <game/MixRNG/RngDeadCoed.c>
#include <game/BOTS_AdventureGrid.h>

// regionsEXE.h defines sdata as a pointer into this, so the linker wants the
// storage even though nothing here reads it.
struct sData sdata_static;

// A race is Arcade or Adventure; every other mode only modifies one of the two,
// so a modifier bit on its own is not a race. Naming the combination once keeps
// every call site from having to remember that.
#define RACE_ADVENTURE_MODE(modifier) (ADVENTURE_MODE | (modifier))

#define RACE_GEM_CUP RACE_ADVENTURE_MODE(ADVENTURE_CUP)
#define RACE_BOSS    RACE_ADVENTURE_MODE(ADVENTURE_BOSS)
#define RACE_RELIC   RACE_ADVENTURE_MODE(RELIC_RACE)
#define RACE_CRYSTAL RACE_ADVENTURE_MODE(CRYSTAL_CHALLENGE)
#define RACE_CLOCK   RACE_ADVENTURE_MODE(TIME_TRIAL)
#define RACE_HUB     RACE_ADVENTURE_MODE(ADVENTURE_ARENA)

// === A normal race ===

static void TestNormalRaceGrid(void)
{
	CTR_TEST_SECTION("normal races: a lone racer fills eight, a pair shares six");

	// 1P always gets the full grid, option or not: that is retail's own roster.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 1, 0), RACE_FIELD_FULL_GRID_RACERS,
	           "1P arcade fills all eight slots");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ADVENTURE_MODE, LOAD_CUP_ID_NONE, 1, 0), RACE_FIELD_FULL_GRID_RACERS,
	           "1P adventure fills all eight slots");

	// A pair is the interesting one, and the extended option is the whole
	// difference between the short six-racer grid and a full one.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 2, 0), RACE_FIELD_PAIR_GRID_RACERS,
	           "2P arcade shares six racers without the option");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ADVENTURE_MODE, LOAD_CUP_ID_NONE, 2, 0), RACE_FIELD_PAIR_GRID_RACERS,
	           "2P adventure shares six racers without the option");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 2, 1), RACE_FIELD_FULL_GRID_RACERS,
	           "2P arcade fills eight with the option");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ADVENTURE_MODE, LOAD_CUP_ID_NONE, 2, 1), RACE_FIELD_FULL_GRID_RACERS,
	           "2P adventure fills eight with the option");
}

static void TestBotCountFollowsTheGrid(void)
{
	CTR_TEST_SECTION("bots: every grid leaves one bot per racer past the humans");

	for (int extended = 0; extended <= 1; extended++)
	{
		for (int playerCount = 1; playerCount <= 2; playerCount++)
		{
			const int racers = RaceField_RacerCount(ADVENTURE_MODE, LOAD_CUP_ID_NONE, playerCount, extended);
			const int bots = RaceField_BotCount(ADVENTURE_MODE, LOAD_CUP_ID_NONE, playerCount, extended);

			CTR_TEST_CHECK_EQ(bots, racers - playerCount, "bots are the racers past the players");
			CTR_TEST_CHECK(bots >= 0, "a race never fields fewer racers than it has players");
			CTR_TEST_CHECK(racers <= RACE_FIELD_FULL_GRID_RACERS, "a grid never exceeds the eight-racer maximum");
		}
	}
}

// === Player counts beyond a pair ===

static void TestHighPlayerCountsNeedTheOption(void)
{
	CTR_TEST_SECTION("player counts: battle goes to four, everything else needs the option");

	for (int extended = 0; extended <= 1; extended++)
	{
		for (int playerCount = 2; playerCount <= 4; playerCount++)
		{
			CTR_TEST_CHECK_EQ(RaceField_BotCount(BATTLE_MODE, LOAD_CUP_ID_NONE, playerCount, 0), 0,
					   "battle mode spawns no bots");
			CTR_TEST_CHECK_EQ(RaceField_RacerCount(BATTLE_MODE, LOAD_CUP_ID_NONE, playerCount, 0), playerCount,
					   "battle mode seats only its players");
		}
	}

	// So with the option, a third or fourth racer shares the same eight-racer grid
	// a lone racer fills, and the bots are whatever is left over.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 3, 1), RACE_FIELD_FULL_GRID_RACERS,
	           "3P arcade fills eight with the option on");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 4, 1), RACE_FIELD_FULL_GRID_RACERS,
	           "4P arcade fills eight with the option on");
	// Eight racers in total, so four players leave four bots rather than the
	// eight a player-count addition would give.
	CTR_TEST_CHECK_EQ(RaceField_BotCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 4, 1), RACE_FIELD_FULL_GRID_RACERS - 4,
	           "4P arcade leaves four bots on the grid");
}

// === Cup races ===

static void TestGemCupRaces(void)
{
	CTR_TEST_SECTION("gem cups: only the Purple cup adds bosses");

	// A normal gem cup is an ordinary race as far as the count goes.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_GEM_CUP, 0, 1, 0), RACE_FIELD_FULL_GRID_RACERS, "1P turquoise cup fills eight");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_GEM_CUP, 1, 2, 0), RACE_FIELD_PAIR_GRID_RACERS, "2P blue cup shares six");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_GEM_CUP, 3, 2, 1), RACE_FIELD_FULL_GRID_RACERS, "2P yellow cup fills eight with the option");

	// The Purple cup always adds its four bosses, whoever is driving, so the
	// extended option does not get to widen it.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, 1, 0), 1 + LOAD_2P_AI_SET_RACER_COUNT,
	           "1P purple cup adds four bosses");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, 1, 1), 1 + LOAD_2P_AI_SET_RACER_COUNT,
	           "the option does not widen the purple cup");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, 2, 0), 2 + LOAD_2P_AI_SET_RACER_COUNT,
	           "2P purple cup adds four bosses");
	CTR_TEST_CHECK_EQ(RaceField_BotCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, 2, 0), LOAD_2P_AI_SET_RACER_COUNT,
	           "2P purple cup fields exactly its four bosses");

	CTR_TEST_CHECK(RaceField_UsesExtendedGrid(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, 1) == 0,
	               "the purple cup keeps its roster off the extended grid");
}

// === Boss races ===

static void TestBossRaceFieldsTheBoss(void)
{
	CTR_TEST_SECTION("boss races: the player plus the boss, whatever the option");

	// IS_BOSS_RACE is a signed test on gameMode1's sign bit, so reading the mode
	// as unsigned is the mutation that makes every boss race look like a normal
	// one and silently drops the boss from the field.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_BOSS, LOAD_CUP_ID_NONE, 1, 0), 1 + RACE_FIELD_BOSS_RACERS,
	           "1P boss race is the player and the boss");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_BOSS, LOAD_CUP_ID_NONE, 1, 1), 1 + RACE_FIELD_BOSS_RACERS,
	           "the option cannot add a bot to a solo boss race");
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_BOSS, LOAD_CUP_ID_NONE, 2, 0), 2 + RACE_FIELD_BOSS_RACERS,
	           "2P boss race is the two players and the boss");
	CTR_TEST_CHECK_EQ(RaceField_BotCount(RACE_BOSS, LOAD_CUP_ID_NONE, 2, 0), RACE_FIELD_BOSS_RACERS,
	           "a boss race fields exactly one bot");

	// The boss occupies a slot of its own, so the extended option must not add
	// racers behind it however many players there are.
	CTR_TEST_CHECK_EQ(RaceField_RacerCount(RACE_BOSS, LOAD_CUP_ID_NONE, 2, 1), 3, "the option cannot widen a boss race");
}

// === Rounds with no grid ===

static void TestRoundsWithNoGridFieldNoBots(void)
{
	CTR_TEST_SECTION("no-grid rounds: only the players take part");

	// Each of these races without a starting grid: a clock, a set of crystals,
	// the hub, or no race at all. Retail spawns no AI for any of them, and
	// filling a grid for one would queue racers nobody drives.
	static const struct
	{
		const char *name;
		int gameMode1;
	} kNoGridRaces[] = {
		{"relic race", RACE_RELIC},
		{"crystal challenge", RACE_CRYSTAL},
		{"time trial", RACE_CLOCK},
		{"adventure hub", RACE_HUB},
		{"battle mode", BATTLE_MODE},
		{"main menu", MAIN_MENU},
		{"cutscene", GAME_CUTSCENE},
	};

	for (size_t i = 0; i < sizeof(kNoGridRaces) / sizeof(kNoGridRaces[0]); i++)
	{
		for (int playerCount = 1; playerCount <= 4; playerCount++)
		{
			// The option is the axis too: it must not turn a clock run into a
			// grid race by filling a grid the round does not have.
			CTR_TEST_CHECK_EQ(RaceField_BotCount(kNoGridRaces[i].gameMode1, LOAD_CUP_ID_NONE, playerCount, 0), 0,
			           kNoGridRaces[i].name);
			CTR_TEST_CHECK_EQ(RaceField_BotCount(kNoGridRaces[i].gameMode1, LOAD_CUP_ID_NONE, playerCount, 1), 0,
			           kNoGridRaces[i].name);
			CTR_TEST_CHECK_EQ(RaceField_RacerCount(kNoGridRaces[i].gameMode1, LOAD_CUP_ID_NONE, playerCount, 0), playerCount,
			           "a no-grid round has only its players");
		}
	}
}

// === Cross-checks ===

static void TestRetailConstantsAreUnchanged(void)
{
	CTR_TEST_SECTION("constants: the masks still read the way retail wrote them");

	// Retail writes this mask as the literal 0x2c122020 in MainInit_Drivers. If
	// a mode bit moves, races silently start or stop fielding bots. The header
	// asserts it at compile time; this pins it as a reported failure too.
	CTR_TEST_CHECK_EQ(RACE_FIELD_NO_BOT_MODES, 0x2c122020, "the no-bot mask is retail's literal");

	// Every mode the mask claims must actually be a round with no grid.
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & RELIC_RACE) != 0, "a relic race is in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & CRYSTAL_CHALLENGE) != 0, "a crystal challenge is in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & TIME_TRIAL) != 0, "a clock run is in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & ADVENTURE_ARENA) != 0, "the hub is in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & BATTLE_MODE) != 0, "a battle is in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & MAIN_MENU) != 0, "the main menu is in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & GAME_CUTSCENE) != 0, "a cutscene is in the mask");

	// And the modes that do race must stay out of it.
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & ARCADE_MODE) == 0, "an arcade race is not in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & ADVENTURE_MODE) == 0, "an adventure race is not in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & ADVENTURE_CUP) == 0, "a cup race is not in the mask");
	CTR_TEST_CHECK((RACE_FIELD_NO_BOT_MODES & (int)ADVENTURE_BOSS) == 0, "a boss race is not in the mask");

	CTR_TEST_CHECK_EQ(RACE_FIELD_FULL_GRID_RACERS, 8, "the full grid is eight racers");
	CTR_TEST_CHECK_EQ(RACE_FIELD_PAIR_GRID_RACERS, 6, "the pair grid is six racers");
	CTR_TEST_CHECK_EQ(RACE_FIELD_BOSS_RACERS, 1, "a boss race fields one bot");
}

static void TestExtendedOptionScope(void)
{
	CTR_TEST_SECTION("option scope: the extended grid applies to Arcade and Adventure races only");

	for (int extended = 0; extended <= 1; extended++)
	{
		// The option is checked before the managed-roster test, so a mode that
		// never races cannot pick up a grid from it.
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(ARCADE_MODE, LOAD_CUP_ID_NONE, extended), extended, "arcade follows the option");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(ADVENTURE_MODE, LOAD_CUP_ID_NONE, extended), extended,
		           "adventure follows the option");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_GEM_CUP, 0, extended), extended, "a gem cup follows the option");

		// These pick their own roster, so the option must leave their slots alone.
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, extended), 0,
		           "the purple cup keeps its own roster");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_BOSS, LOAD_CUP_ID_NONE, extended), 0, "a boss race keeps its own roster");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_RELIC, LOAD_CUP_ID_NONE, extended), 0, "a relic race has no grid to widen");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_CRYSTAL, LOAD_CUP_ID_NONE, extended), 0,
		           "a crystal challenge has no grid to widen");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_CLOCK, LOAD_CUP_ID_NONE, extended), 0, "a clock run has no grid to widen");

		// Not a race at all.
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(RACE_HUB, LOAD_CUP_ID_NONE, extended), 0, "the hub is not a race");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(MAIN_MENU, LOAD_CUP_ID_NONE, extended), 0, "the main menu is not a race");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(BATTLE_MODE, LOAD_CUP_ID_NONE, extended), 0, "a battle is not a race");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(GAME_CUTSCENE, LOAD_CUP_ID_NONE, extended), 0, "a cutscene is not a race");
		CTR_TEST_CHECK_EQ(RaceField_UsesExtendedGrid(0, LOAD_CUP_ID_NONE, extended), 0, "no mode at all is not a race");
	}
}

// No mode bit means "a race" by itself: a race is Arcade or Adventure, and the
// rest only modify one. So a caller that forgets to pass the base gets the safe
// answer (no bots) rather than a grid for a race that is not happening.
static void TestAModeBitAloneIsNotARace(void)
{
	CTR_TEST_SECTION("coverage: a mode bit on its own is not a race");

	static const int kAloneBits[] = {
		ADVENTURE_CUP,
		(int)ADVENTURE_BOSS,
		END_OF_RACE,
		RELIC_RACE,
		CRYSTAL_CHALLENGE,
		TIME_TRIAL,
		ADVENTURE_ARENA,
		BATTLE_MODE,
		MAIN_MENU,
		GAME_CUTSCENE,
		LOADING,
	};

	for (size_t i = 0; i < sizeof(kAloneBits) / sizeof(kAloneBits[0]); i++)
	{
		for (int extended = 0; extended <= 1; extended++)
		{
			CTR_TEST_CHECK_EQ(RaceField_BotCount(kAloneBits[i], LOAD_CUP_ID_NONE, 2, extended), 0, "a lone mode bit fields no bots");
		}
	}
}

static void TestConfiguredGrid(void)
{
	CTR_TEST_SECTION("configured grids: capacity, multiplayer and managed rosters");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 1, false, 16, 15), 16,
	                  "a solo race can request sixteen distinct racers");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 1, false, 16, 7), 8,
	                  "the eligible roster limits the field");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 2, false, 16, 15), 6,
	                  "retail two-player races cap the field at six");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 2, true, 16, 15), 16,
	                  "extended multiplayer permits the configured field");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 4, false, 16, 15), 4,
	                  "retail four-player races gain no bots");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 4, true, 2, 15), 4,
	                  "the setting never removes a human player");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 1, true, 0, 15), 2,
	                  "the configured minimum is two racers");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 1, true, 100, 100), 16,
	                  "the configured maximum is sixteen racers");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, 2, true, 16, -1), 2,
	                  "an unavailable roster never removes human players");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(RACE_BOSS, LOAD_CUP_ID_NONE, 1, true, 16, 15), 2,
	                  "boss races keep their managed roster");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, 1, true, 16, 15), 5,
	                  "the purple gem cup keeps its four bosses");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(TIME_TRIAL, LOAD_CUP_ID_NONE, 1, true, 16, 15), 1,
	                  "time trials never gain opponents");
}

static void TestConfiguredGridModeMatrix(void)
{
	static const int normalModes[] = {ARCADE_MODE, ADVENTURE_MODE, RACE_GEM_CUP};
	static const int managedModes[] = {
	    ARCADE_MODE | TIME_TRIAL, RACE_RELIC, RACE_CRYSTAL, RACE_HUB,
	    ARCADE_MODE | BATTLE_MODE, MAIN_MENU, GAME_CUTSCENE, RACE_BOSS,
	};

	CTR_TEST_SECTION("configured grids: default and managed mode matrix");
	for (int players = 1; players <= 4; players++)
	{
		for (int extended = 0; extended <= 1; extended++)
		{
			for (size_t mode = 0; mode < sizeof(normalModes) / sizeof(normalModes[0]); mode++)
			{
				int flags = normalModes[mode];
				CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(flags, LOAD_CUP_ID_NONE, players, extended, 8, 16),
				                  RaceField_RacerCount(flags, LOAD_CUP_ID_NONE, players, extended),
				                  "the default field matches retail when enough characters are available");
			}
			for (size_t mode = 0; mode < sizeof(managedModes) / sizeof(managedModes[0]); mode++)
			{
				int flags = managedModes[mode];
				CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(flags, LOAD_CUP_ID_NONE, players, extended, 16, 0),
				                  RaceField_RacerCount(flags, LOAD_CUP_ID_NONE, players, extended),
				                  "a managed mode ignores the configured field and bot availability");
			}
			CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, players, extended, 16, 0),
			                  RaceField_RacerCount(RACE_GEM_CUP, CUP_ID_PURPLE_GEM, players, extended),
			                  "the purple gem cup retains its managed field");
		}
	}

	CTR_TEST_SECTION("configured grids: all supported field sizes and player counts");
	for (int players = 1; players <= 4; players++)
	{
		for (int configured = 2; configured <= 16; configured++)
		{
			for (int available = 0; available <= 15; available++)
			{
				int expected = configured < players ? players : configured;
				if (expected > players + available)
				{
					expected = players + available;
				}
				CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, LOAD_CUP_ID_NONE, players, true, configured, available),
				                  expected, "the extended grid respects the setting, humans and available characters");
			}
		}
	}
}

static void TestCompactSpawnOrder(void)
{
	CTR_TEST_SECTION("grid compaction preserves relative order across a smaller round");
	char grid[RACE_FIELD_CONFIG_MAX_RACERS] = {15, 14, 0, 4, 1, 8};
	RaceField_CompactSpawnOrder(grid, 6);
	const char reduced[] = {5, 4, 0, 2, 1, 3};
	for (int i = 0; i < (int)len(reduced); i++)
	{
		CTR_TEST_CHECK_EQ(grid[i], reduced[i], "surviving drivers keep their order and both humans still start at the back");
	}
	CTR_TEST_CHECK_EQ((unsigned char)grid[6], 255, "an inactive driver cannot reuse an old rank");
	RaceField_CompactSpawnOrder(grid, 9);
	for (int i = 6; i < 9; i++)
	{
		CTR_TEST_CHECK_EQ(grid[i], i, "bots returning in a later round take the open positions");
	}
	grid[0] = grid[1];
	RaceField_CompactSpawnOrder(grid, 9);
	CTR_TEST_CHECK_EQ(grid[1], 8, "duplicate ranks do not place two karts in the same slot");
	for (int i = 0; i < RACE_FIELD_CONFIG_MAX_RACERS; i++)
	{
		grid[i] = RACE_FIELD_CONFIG_MAX_RACERS - 1 - i;
	}
	RaceField_CompactSpawnOrder(grid, RACE_FIELD_CONFIG_MAX_RACERS);
	for (int i = 0; i < RACE_FIELD_CONFIG_MAX_RACERS; i++)
	{
		CTR_TEST_CHECK_EQ(grid[i], RACE_FIELD_CONFIG_MAX_RACERS - 1 - i, "a valid expanded grid is unchanged");
	}
}

static void TestInitialAdventureGrid(void)
{
	CTR_TEST_SECTION("Adventure seeds the grid from the admitted roster, not the requested size");
	char grid[RACE_FIELD_CONFIG_MAX_RACERS];
	struct RngDeadCoedState rng = {.state0 = 0x30215400, .state1 = 0x493583fe};
	for (int racers = 2; racers <= RACE_FIELD_CONFIG_MAX_RACERS; racers++)
	{
		memset(grid, -1, sizeof(grid));
		BOTS_SeedAdventureGrid(grid, 2, racers, -1, &rng);
		CTR_TEST_CHECK(grid[0] == racers - 1 && grid[1] == racers - 2,
		               "both humans start at the back even when the playable grid is smaller");
		bool occupied[RACE_FIELD_CONFIG_MAX_RACERS] = {0};
		for (int i = 2; i < racers; i++)
		{
			const unsigned int slot = (unsigned char)grid[i];
			CTR_TEST_CHECK(slot < (unsigned int)(racers - 2) && !occupied[slot], "bots have distinct places ahead of the humans");
			if (slot < len(occupied))
			{
				occupied[slot] = true;
			}
		}
		CTR_TEST_CHECK(racers == RACE_FIELD_CONFIG_MAX_RACERS || grid[racers] == (char)-1,
		               "a rejected driver is not assigned a starting slot");
	}
	BOTS_SeedAdventureGrid(grid, 2, 6, 3, &rng);
	const char championGrid[] = {5, 4, 1, 0, 3, 2};
	CTR_TEST_CHECK(memcmp(grid, championGrid, sizeof(championGrid)) == 0, "the champion takes pole and the last bot fills the gap");
	BOTS_SeedAdventureGrid(grid, 2, 6, 5, &rng);
	CTR_TEST_CHECK(grid[2] == 1 && grid[3] == 2 && grid[4] == 3 && grid[5] == 0,
	               "the champion can take pole even as the last eligible bot");
}

int main(void)
{
	TestNormalRaceGrid();
	TestBotCountFollowsTheGrid();
	TestHighPlayerCountsNeedTheOption();
	TestGemCupRaces();
	TestBossRaceFieldsTheBoss();
	TestRoundsWithNoGridFieldNoBots();
	TestRetailConstantsAreUnchanged();
	TestExtendedOptionScope();
	TestAModeBitAloneIsNotARace();
	TestConfiguredGrid();
	TestConfiguredGridModeMatrix();
	TestCompactSpawnOrder();
	TestInitialAdventureGrid();

	return CtrTest_Summary();
}

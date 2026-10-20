#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
NativeConfig g_config;
static struct GameTracker s_tracker;
static bool s_teleported;

#include <game/231/RB_Chaos.c>
#include <game/MAIN/MainGameStart.c>

void VehBirth_TeleportAll(struct GameTracker *gGT, u32 spawnFlags)
{
	(void)gGT;
	(void)spawnFlags;
	s_teleported = true;
	CTR_TEST_CHECK(!sdata->chaosRuntime.raceInitialized, "race setup runs after vehicle initialization");
}

void Music_Stop(void) {}
void howl_StopAudio(b32 backup, b32 music, b32 fx)
{
	(void)backup;
	(void)music;
	(void)fx;
}

static void SetupRace(int trigger)
{
	memset(&sdata_static, 0, sizeof(sdata_static));
	memset(&s_tracker, 0, sizeof(s_tracker));
	memset(&g_config, 0, sizeof(g_config));
	sdata->gGT = &s_tracker;
	sdata->Loading.stage = LOAD_IDLE;
	s_tracker.gameMode1 = ADVENTURE_MODE;
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	s_tracker.numPlyrCurrGame = 2;
	sdata->chaosSettings = (struct ChaosSettings){
		.armed = true, .trigger = (u8)trigger, .durationSeconds = 30,
		.triggerIntervalSeconds = 5, .enabledCount = 1,
	};
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	Chaos_BeginRace(&s_tracker);
}

static void Tick(int elapsedMS)
{
	s_tracker.elapsedTimeMS = elapsedMS;
	Chaos_Update(&s_tracker);
}

static void TestIntervalCadence(void)
{
	CTR_TEST_SECTION("Interval starts after GO, carries frame overshoot and gives new entries full duration");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	s_teleported = false;
	Chaos_Reset();
	MainGameStart_Initialize(&s_tracker, true);
	CTR_TEST_CHECK(s_teleported && sdata->chaosRuntime.raceInitialized, "the actual race-start callback initializes Chaos after teleporting");
	CTR_TEST_CHECK(!g_config.chaosMode, "the creation prompt is disabled in this fixture");
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the countdown cannot activate an effect");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "the countdown does not consume trigger time");
	s_tracker.trafficLightsTimer = 0;
	Tick(4992);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "Interval waits for the whole first gap");
	Tick(32);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "an armed file activates despite the disabled creation prompt");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].effect, CHAOS_EFFECT_LOW_GRAVITY, "Interval chooses from selected effects only");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 30000, "a new activation starts with its full duration");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 4976, "frame overshoot is retained rather than drifting the cadence");
	Tick(4976);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "Interval does not wait for the earlier effect to expire");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 25024, "the original timer keeps its own age");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].remainingMS, 30000, "repeated selection appends a distinct timer");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "the second boundary returns to the original cadence");
}

static void TestIndependentExpiryAndOrder(void)
{
	CTR_TEST_SECTION("expiry preserves activation order and duplicate lifetimes");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount++;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 200), "the first selected effect can activate");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 100), "an unrelated later effect can overlap");
	Tick(50);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 300), "a repeated activation gets another entry");
	Tick(50);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "only the entry reaching its exact expiry is removed");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 100, "compaction keeps the oldest surviving timer");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].remainingMS, 250, "compaction preserves the later duplicate and its timer");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SUPER_SPEED), "an expired effect no longer reports active");
	Tick(100);
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "expiry of one duplicate does not stop the remaining activation");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 150, "the surviving duplicate is not refreshed or shortened");
	Tick(150);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the last timer expires exactly at zero");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "no active status remains after final expiry");

	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 100);
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 300);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 200);
	Tick(100);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].effect, CHAOS_EFFECT_SUPER_SPEED, "removing the oldest entry preserves the next effect's order");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].effect, CHAOS_EFFECT_LOW_GRAVITY, "the newest effect remains last after compaction");
}

static void CheckSuspended(void)
{
	const struct ChaosRuntime before = sdata->chaosRuntime;
	const u32 rng = sdata->chaosRng;
	Tick(1000);
	CTR_TEST_CHECK(memcmp(&before, &sdata->chaosRuntime, sizeof(before)) == 0, "ineligible updates preserve timers and cadence");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "ineligible updates do not draw Chaos RNG");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000), "ineligible frames reject activation requests");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "effect hooks see no active modifier outside eligible gameplay");
}

static void TestGatesAndPause(void)
{
	CTR_TEST_SECTION("pause, menus, transitions and adjacent modes cannot advance or activate Chaos");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 10000);
	const int blocked[] = {PAUSE_1, DEBUG_MENU, ADVENTURE_ARENA, MAIN_MENU, GAME_CUTSCENE, LOADING, END_OF_RACE};
	for (int i = 0; i < (int)len(blocked); i++)
	{
		s_tracker.gameMode1 = ADVENTURE_MODE | blocked[i];
		CheckSuspended();
	}
	s_tracker.gameMode1 = ARCADE_MODE;
	CheckSuspended();
	s_tracker.gameMode1 = BATTLE_MODE;
	CheckSuspended();
	s_tracker.gameMode1 = ADVENTURE_MODE;
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_ADV_HUB;
	CheckSuspended();
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	sdata->ptrActiveMenu = &sdata->chaosMenu.menu;
	CheckSuspended();
	sdata->ptrActiveMenu = NULL;
	sdata->Loading.stage = LOAD_REQUESTED;
	CheckSuspended();
	sdata->Loading.stage = LOAD_IDLE;
	sdata->chaosSettings.armed = false;
	CheckSuspended();
	sdata->chaosSettings.armed = true;
	Tick(1000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 9000, "resuming consumes only the next gameplay delta");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 4000, "the trigger resumes its remaining gap");
	const int variants[] = {RELIC_RACE, CRYSTAL_CHALLENGE, ADVENTURE_CUP, (int)ADVENTURE_BOSS};
	for (int i = 0; i < (int)len(variants); i++)
	{
		s_tracker.gameMode1 = ADVENTURE_MODE | variants[i];
		CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "every Adventure race type can consume active effects");
	}
}

static void TestCapacityAndValidation(void)
{
	CTR_TEST_SECTION("capacity rejects new entries without eviction, refresh or RNG consumption");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_NONE, 100), "the none sentinel cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_COUNT, 100), "out-of-range effects cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 100), "disabled catalogue effects cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 0), "zero duration cannot create an entry");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, -1), "negative duration cannot create an entry");
	for (int i = 0; i < CHAOS_MAX_ACTIVATIONS; i++)
	{
		CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 5000 + i), "each available slot accepts an independent activation");
	}
	const struct ChaosRuntime full = sdata->chaosRuntime;
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 9999), "a full queue rejects a new request");
	CTR_TEST_CHECK(memcmp(&full, &sdata->chaosRuntime, sizeof(full)) == 0, "overflow does not evict or refresh existing activations");
	sdata->chaosRuntime.intervalRemainingMS = 1;
	const u32 rng = sdata->chaosRng;
	Tick(1);
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "an Interval event rejected at capacity does not consume a random choice");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "a rejected Interval event still consumes its scheduled gap");
	Tick(4999);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, CHAOS_MAX_ACTIVATIONS - 1, "expiring an entry makes room without disturbing the others");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000), "a freed slot can accept a later activation");
}

static void TestResetAndInvalidSettings(void)
{
	CTR_TEST_SECTION("race reset preserves file settings and handles empty or invalid timed settings");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 10000);
	const struct ChaosSettings settings = sdata->chaosSettings;
	const u32 rng = sdata->chaosRng;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "retry cannot inherit running activations");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "retry starts a fresh first interval");
	CTR_TEST_CHECK(memcmp(&settings, &sdata->chaosSettings, sizeof(settings)) == 0, "race setup does not modify creation settings");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "race reset does not reseed the dedicated generator");
	Chaos_Reset();
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000), "a race must initialize before accepting activations");
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "teardown cannot restart the engine through an update");
	sdata->chaosSettings.armed = false;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK(!sdata->chaosRuntime.raceInitialized, "a Regular file never initializes Chaos");
	sdata->chaosSettings.armed = true;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK(!sdata->chaosRuntime.raceInitialized, "entering a hub does not initialize a Chaos race");

	SetupRace(CHAOS_TRIGGER_INTERVAL);
	memset(sdata->chaosSettings.effects, 0, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.enabledCount = 0;
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "an empty armed file produces no activation");
	CTR_TEST_CHECK(sdata->chaosSettings.armed, "an empty selection never reclassifies the file as Regular");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, 0, "empty selection consumes no random choice");
	sdata->chaosSettings.triggerIntervalSeconds = 0;
	Chaos_BeginRace(&s_tracker);
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "a zero saved interval cannot spin a scheduling loop");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	sdata->chaosSettings.durationSeconds = 0;
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "zero saved duration cannot create expired entries");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, 0, "invalid duration does not draw RNG");
	for (int trigger = 0; trigger < CHAOS_TRIGGER_COUNT; trigger++)
	{
		if (trigger == CHAOS_TRIGGER_INTERVAL)
		{
			continue;
		}
		SetupRace(trigger);
		Tick(5000);
		CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "other trigger modes do not fall back to Interval scheduling");
	}
}

static void TestResidentRestore(void)
{
	CTR_TEST_SECTION("restoring resident state resumes timers, activation order and RNG choices");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount++;
	Tick(5000);
	Tick(1234);
	static struct sData saved;
	saved = sdata_static;
	const int deltas[] = {3766, 5000, 5000, 5000, 5000, 5000};
	for (int i = 0; i < (int)len(deltas); i++)
	{
		Tick(deltas[i]);
	}
	const struct ChaosRuntime expected = sdata->chaosRuntime;
	const u32 expectedRng = sdata->chaosRng;
	Chaos_Reset();
	sdata_static = saved;
	for (int i = 0; i < (int)len(deltas); i++)
	{
		Tick(deltas[i]);
	}
	CTR_TEST_CHECK(memcmp(&expected, &sdata->chaosRuntime, sizeof(expected)) == 0, "resident restore reproduces timer expiry, queue order and trigger cadence");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, expectedRng, "resident restore reproduces subsequent effect choices");
	CTR_TEST_CHECK_EQ(sdata->randomNumber, saved.randomNumber, "Chaos scheduling leaves retail RNG untouched");
}

int main(void)
{
	TestIntervalCadence();
	TestIndependentExpiryAndOrder();
	TestGatesAndPause();
	TestCapacityAndValidation();
	TestResetAndInvalidSettings();
	TestResidentRestore();
	return CtrTest_Summary();
}

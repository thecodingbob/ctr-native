#include <shared/ctr_test.h>
#include <common.h>
#include <widescreen.h>
#include <string.h>

struct sData sdata_static;
struct rData rdata;
struct Data data;
struct GameTracker tracker;
NativeConfig g_config;
static int lapRows;
static char firstLapText[9];
static char lastLapText[9];
static unsigned int relicTargetsDrawn;
static int textCalls;
static s16 timeTextX;
static s16 targetTextX;
static int relicTier = -1;
static char *language[LNG_RETAIL_STRING_COUNT];

void DecalFont_DrawLine(char *str, s16 x, s16 y, s16 font, s16 flags)
{
	(void)y; (void)font; (void)flags;
	if (str != NULL && strlen(str) == 8 && str[0] == ' ' && str[2] == ':' && str[5] == ':' && textCalls > 1)
	{
		if (lapRows++ == 0)
		{
			memcpy(firstLapText, str, sizeof(firstLapText));
		}
		memcpy(lastLapText, str, sizeof(lastLapText));
	}
	if (textCalls++ == 1)
	{
		timeTextX = x;
	}
	if (textCalls == 4)
	{
		targetTextX = x;
	}
}

s32 DecalFont_GetLineWidth(char *str, s16 font)
{
	// Stand-in metrics keep the fixture independent of font assets.
	return Widescreen_ScaleWidth((int)strlen(str) * (font == FONT_BIG ? 17 : 13));
}

int GAMEPROG_AdventureTimeLimit(int time) { return time; }
int GAMEPROG_RelicTargetTime(int levelID, int tier) { (void)levelID; relicTargetsDrawn |= 1u << tier; return 1000; }
b32 MainGameEnd_GotAllTimeCrates(struct GameTracker *gGT) { (void)gGT; return false; }
int MainGameEnd_RelicTier(struct GameTracker *gGT, struct Driver *driver) { (void)gGT; (void)driver; return relicTier; }
char *RECTMENU_DrawTime(int time) { (void)time; return ""; }
void MainGameEnd_Initialize(void) {}

#include <game/widescreen.c>
#include <game/UI/UI_Clock.c>

int main(void)
{
	struct Driver driver = {0};
	sdata->gGT = &tracker;
	sdata->lngStrings = language;
	tracker.gameMode1 = TIME_TRIAL;
	tracker.numLaps = 7;
	driver.driverID = 2;
	driver.lapIndex = 6;
	tracker.elapsedEventTime = 62000;
	driver.lapTime = tracker.elapsedEventTime - 9600;
	driver.lapHistory.completed[0] = 28800;
	memcpy(rdata.s_timeString_empty, " 0:00:00", 9);
	memcpy(sdata->s_int, "%d", 3);

	CTR_TEST_SECTION("Live and finished seven-lap race clocks");
	UI_DrawRaceClock(0, 0, UI_RACE_CLOCK_SHOW_CURRENT_TIME, &driver);
	CTR_TEST_CHECK(lapRows == 7 && strcmp(lastLapText, " 0:10:00") == 0 && driver.lapHistory.completed[6] == 0,
	               "the live seventh lap displays the current split without storing it as completed");

	for (int player = 0; player < 4; player++)
	{
		lapRows = 0;
		driver.driverID = player;
		driver.lapIndex = 7;
		driver.lapHistory.completed[6] = 11520;
		UI_DrawRaceClock(0, 0, UI_RACE_CLOCK_SHOW_RESULTS, &driver);
		CTR_TEST_CHECK(lapRows == 7 && strcmp(firstLapText, " 0:30:00") == 0 && strcmp(lastLapText, " 0:12:00") == 0,
		               "finished results show only the kart's seven completed times, regardless of controller slot");
	}

	tracker.numLaps = 3;
	driver.lapIndex = 3;
	lapRows = 0;
	UI_DrawRaceClock(0, 0, UI_RACE_CLOCK_SHOW_RESULTS, &driver);
	CTR_TEST_CHECK_EQ(lapRows, 3, "ordinary finished races show only their three lap rows");

	CTR_TEST_SECTION("Relic race targets");
	const u32 relicModes[] = {ADVENTURE_MODE | RELIC_RACE, ARCADE_MODE | RELIC_RACE};
	for (int i = 0; i < (int)len(relicModes); i++)
	{
		tracker.gameMode1 = relicModes[i];
		relicTargetsDrawn = 0;
		UI_DrawRaceClock(0, 0, UI_RACE_CLOCK_SHOW_CURRENT_TIME, &driver);
		CTR_TEST_CHECK_EQ(relicTargetsDrawn, 7, "the race HUD displays all three relic targets");
	}
	tracker.gameMode1 = ADVENTURE_MODE | RELIC_RACE;
	relicTargetsDrawn = 0;
	UI_DrawRaceClock(0, 0, UI_RACE_CLOCK_SHOW_RESULTS, &driver);
	CTR_TEST_CHECK_EQ(relicTargetsDrawn, 0, "Adventure results retain their existing reward display");

	CTR_TEST_SECTION("Result time spacing follows the aspect ratio");
	const int timeOffsets[] = {17, 12, 14, 9};
	for (int aspect = 0; aspect < 4; aspect++)
	{
		g_config.aspectRatio = aspect;
		textCalls = 0;
		UI_DrawRaceClock(100, 60, UI_RACE_CLOCK_SHOW_RESULTS, &driver);
		CTR_TEST_CHECK_EQ(timeTextX, 100 + timeOffsets[aspect], "normal results compress the label-to-time offset");
		textCalls = 0;
		UI_DrawRaceClock(100, 60, UI_RACE_CLOCK_SHOW_RESULTS | UI_RACE_CLOCK_COMPACT_RESULTS, &driver);
		CTR_TEST_CHECK_EQ(timeTextX, 100, "compact results retain the shared time column");
	}

	CTR_TEST_SECTION("Relic result columns stay aligned");
	relicTier = 0;
	for (int aspect = 0; aspect < 4; aspect++)
	{
		g_config.aspectRatio = aspect;
		for (int mode = 0; mode < (int)len(relicModes); mode++)
		{
			tracker.gameMode1 = relicModes[mode];
			textCalls = 0;
			targetTextX = -1;
			UI_DrawRaceClock(100, 60, UI_RACE_CLOCK_SHOW_RESULTS, &driver);
			CTR_TEST_CHECK_EQ(textCalls, 4, "relic results draw the total and target rows");
			CTR_TEST_CHECK_EQ(targetTextX, timeTextX, "Adventure and Arcade targets share the total time column");
		}
	}

	CTR_TEST_SECTION("Result field width contains both total and lap times");
	const int lapCounts[] = {1, 3, 5, 7, EXTENDED_LAP_HISTORY_CAPACITY + 1};
	for (int aspect = 0; aspect < 4; aspect++)
	{
		g_config.aspectRatio = aspect;
		for (int laps = 0; laps < (int)len(lapCounts); laps++)
		{
			tracker.numLaps = lapCounts[laps];
			for (int compact = 0; compact <= 1; compact++)
			{
				const int font = compact ? FONT_SMALL : FONT_BIG;
				const int offset = compact ? 0 : timeOffsets[aspect];
				const int totalWidth = DecalFont_GetLineWidth(compact || tracker.numLaps >= 7 ? "00:00:00" : "0:00:00", font);
				const int width = UI_GetRaceClockResultsWidth(compact);
				CTR_TEST_CHECK(width >= offset + totalWidth, "the field contains the full total including its column offset");
				if (tracker.numLaps <= EXTENDED_LAP_HISTORY_CAPACITY)
				{
					const int lapFont = compact || tracker.numLaps > 3 ? FONT_SMALL : FONT_BIG;
					CTR_TEST_CHECK(width >= DecalFont_GetLineWidth("00:00:00", lapFont), "the field also contains the widest lap row");
				}
			}
		}
	}

	return CtrTest_Summary();
}

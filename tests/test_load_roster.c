#include <shared/ctr_test.h>
#include <common.h>
#include <game/LOAD/LOAD_Roster.h>
#include <race_field.h>

static b32 IsUnlocked(s16 characterID)
{
	return characterID < 8;
}

int main(void)
{
	const s16 humans[] = {0, 7, 12, 12};

	CTR_TEST_SECTION("bot roster availability");
	CTR_TEST_CHECK(!LOAD_RosterCharacterAvailable(0, humans, 4, false, IsUnlocked), "a human character is never available to a bot");
	CTR_TEST_CHECK(!LOAD_RosterCharacterAvailable(12, humans, 4, true, IsUnlocked), "a locked human character is unavailable");
	CTR_TEST_CHECK(!LOAD_RosterCharacterAvailable(9, humans, 4, true, IsUnlocked), "Random Unlocked excludes locked characters");
	CTR_TEST_CHECK(LOAD_RosterCharacterAvailable(9, humans, 4, false, IsUnlocked), "Vanilla and Random All permit locked characters");
	CTR_TEST_CHECK(LOAD_RosterCharacterAvailable(1, humans, 4, true, IsUnlocked), "an unlocked character unused by humans is available");
	CTR_TEST_CHECK_EQ(LOAD_RosterAvailableBotCount(16, humans, 4, false, IsUnlocked), 13,
	                  "duplicate human selections exclude only one character from the full roster");
	CTR_TEST_CHECK_EQ(LOAD_RosterAvailableBotCount(16, humans, 4, true, IsUnlocked), 6,
	                  "locked human selections do not reduce the unlocked candidate count");
	CTR_TEST_CHECK_EQ(LOAD_RosterAvailableBotCount(16, humans, 0, true, IsUnlocked), 8,
	                  "all unlocked characters are available without human selections");
	CTR_TEST_CHECK_EQ(LOAD_RosterAvailableBotCount(0, humans, 4, false, IsUnlocked), 0,
	                  "an empty roster has no candidates");

	CTR_TEST_SECTION("configured field with available characters");
	int unlockedBots = LOAD_RosterAvailableBotCount(16, humans, 4, true, IsUnlocked);
	int allBots = LOAD_RosterAvailableBotCount(16, humans, 4, false, IsUnlocked);
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 4, true, 16, unlockedBots), 10,
	                  "the unlocked field includes all humans and only the six available bots");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 4, true, 16, allBots), 16,
	                  "the full roster fills the configured field even with duplicate human selections");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 4, false, 16, allBots), 4,
	                  "retail four-player races remain bot-free even when characters are available");
	unlockedBots = LOAD_RosterAvailableBotCount(16, humans, 2, true, IsUnlocked);
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ADVENTURE_MODE, 0, 2, true, 16, unlockedBots), 8,
	                  "two humans and six unlocked candidates limit the extended field to eight");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ADVENTURE_MODE, 0, 2, false, 16, unlockedBots), 6,
	                  "the retail two-player cap applies even when more characters are available");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 1, true, 16, 0), 1,
	                  "a player is retained when the roster supplies no bots");
	return CtrTest_Summary();
}

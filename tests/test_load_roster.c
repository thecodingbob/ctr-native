#include <shared/ctr_test.h>
#include <common.h>
#include <game/LOAD/LOAD_Roster.h>

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
	return CtrTest_Summary();
}

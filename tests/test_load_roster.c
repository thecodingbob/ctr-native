#include <shared/ctr_test.h>
#include <common.h>
#include <game/LOAD/LOAD_Roster.h>
#include <race_field.h>

struct sData sdata_static;
struct Data data;
static struct NativeCharacterMetadata s_characters[CHARACTER_SELECT_EXTRA_SLOT_CAPACITY + 2];
static size_t s_characterCount;

size_t NativeCharacterLibrary_Count(void) { return s_characterCount; }
const struct NativeCharacterMetadata *NativeCharacterLibrary_Get(size_t index)
{
	return index < s_characterCount ? &s_characters[index] : NULL;
}

static b32 IsUnlocked(s16 characterID)
{
	return characterID < 8;
}

static int FindEntry(const struct LOAD_Roster *roster, int baseID, const char *identity)
{
	for (int i = 0; i < roster->count; i++)
	{
		const struct LOAD_RosterEntry *entry = &roster->entries[i];
		if (entry->baseID == baseID && (identity == NULL ? entry->custom == NULL :
		    entry->custom != NULL && strcmp(entry->custom->identity, identity) == 0)) { return i; }
	}
	return -1;
}

static void TestRetailRoster(void)
{
	CTR_TEST_SECTION("retail roster availability and field prediction");
	const s16 humans[] = {0, 7, 12, 12};
	memcpy(data.characterIDs, humans, sizeof(humans));
	struct LOAD_Roster roster;
	LOAD_RosterPrepareAvailable(&roster, len(humans), true, false, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, 13, "duplicate human selections exclude only one retail candidate");
	CTR_TEST_CHECK(FindEntry(&roster, 0, NULL) < 0 && FindEntry(&roster, 12, NULL) < 0,
	               "human characters are unavailable to bots");
	CTR_TEST_CHECK(FindEntry(&roster, 9, NULL) >= 0, "Random All permits locked retail characters");
	int allBots = roster.count;
	LOAD_RosterPrepareAvailable(&roster, len(humans), true, true, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, 6, "locked humans do not reduce the unlocked pool");
	CTR_TEST_CHECK(FindEntry(&roster, 9, NULL) < 0 && FindEntry(&roster, 1, NULL) >= 0,
	               "Random Unlocked filters only locked characters");
	int unlockedBots = roster.count;
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 4, true, 16, unlockedBots), 10,
	                  "the unlocked field includes four humans and six bots");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 4, true, 16, allBots), 16,
	                  "the full roster fills the configured field");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 4, false, 16, allBots), 4,
	                  "retail four-player races remain bot-free");
	LOAD_RosterPrepareAvailable(&roster, 2, true, true, IsUnlocked);
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ADVENTURE_MODE, 0, 2, true, 16, roster.count), 8,
	                  "the available pool also limits Adventure fields");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ADVENTURE_MODE, 0, 2, false, 16, roster.count), 6,
	                  "the retail two-player cap remains unchanged");
	LOAD_RosterPrepareAvailable(&roster, 0, true, true, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, 8, "all unlocked characters are available without humans");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 1, true, 16, 0), 1,
	                  "a player is retained when the pool supplies no bots");
}

static void TestCustomRoster(void)
{
	CTR_TEST_SECTION("shared active roster and identity-aware random pools");
	memset(sdata->customCharacters, 0, sizeof(sdata->customCharacters));
	data.MetaDataCharacters[POLAR].name_Debug = "polar";
	data.MetaDataCharacters[PAPU_PAPU].name_Debug = "papu";
	s_characterCount = len(s_characters);
	for (size_t i = 0; i < s_characterCount; i++)
	{
		snprintf(s_characters[i].identity, sizeof(s_characters[i].identity), "custom-%u", (unsigned)i);
		strcpy(s_characters[i].base, "papu");
		strcpy(s_characters[i].modelHi, "model.ctr");
		strcpy(s_characters[i].directory, "voices");
	}
	strcpy(s_characters[0].base, "invalid");
	const struct NativeCharacterMetadata *active[CHARACTER_SELECT_EXTRA_SLOT_CAPACITY];
	CTR_TEST_CHECK_EQ(CustomCharacter_PrepareAvailableEntries(active, len(active)), len(active),
	                  "invalid bases are skipped before applying the active roster capacity");
	CTR_TEST_CHECK(active[0] == &s_characters[1] && active[len(active) - 1] == &s_characters[len(active)],
	               "menu and opponents share the first eight valid entries, not every installed character");
	struct LOAD_Roster roster;
	LOAD_RosterPrepareAvailable(&roster, 0, true, true, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, 8 + len(active), "all active customs are unlocked even with a locked retail base");
	CTR_TEST_CHECK(FindEntry(&roster, PAPU_PAPU, NULL) < 0 && FindEntry(&roster, PAPU_PAPU, active[0]->identity) >= 0,
	               "custom unlock eligibility does not inherit the base's locked state");
	data.characterIDs[0] = PAPU_PAPU;
	CustomCharacter_Select(0, active[0]);
	data.characterIDs[1] = POLAR;
	data.characterIDs[2] = PAPU_PAPU;
	CustomCharacter_Select(2, active[0]);
	LOAD_RosterPrepareAvailable(&roster, 3, true, false, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, GAME_CHARACTER_COUNT - 1 + len(active) - 1,
	                  "duplicate human customs exclude one identity, not their retail base");
	CTR_TEST_CHECK(FindEntry(&roster, PAPU_PAPU, NULL) >= 0 && FindEntry(&roster, PAPU_PAPU, active[0]->identity) < 0 &&
	               FindEntry(&roster, PAPU_PAPU, active[1]->identity) >= 0,
	               "retail bases and different customs sharing a base coexist in the pool");
	LOAD_RosterPrepareAvailable(&roster, 3, true, true, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, 7 + len(active) - 1, "Random Unlocked excludes human customs by identity too");
	CTR_TEST_CHECK_EQ(RaceField_ConfiguredRacerCount(ARCADE_MODE, 0, 3, true, 16, roster.count), 16,
	                  "field prediction includes custom opponents instead of shrinking to the retail pool");

	CTR_TEST_SECTION("assignment, cup retention and stale opponent cleanup");
	CustomCharacter_BackupSelections();
	int index = FindEntry(&roster, PAPU_PAPU, active[1]->identity);
	LOAD_RosterSelectDriver(&roster.entries[index], 3);
	CTR_TEST_CHECK(CustomCharacter_IsSelected(3) &&
	               strcmp(sdata->customCharacters[3].modelPath, "model.ctr") == 0,
	               "assigning an AI prepares its identity and complete resource snapshot");
	int before = roster.count;
	CTR_TEST_CHECK(LOAD_RosterRetainDriver(&roster, 3) && roster.count == before - 1,
	               "later cup rounds retain the exact opponent and consume its candidate");
	CTR_TEST_CHECK(!LOAD_RosterRetainDriver(&roster, 3), "a retained custom cannot be selected again for another bot");
	CustomCharacter_RestoreSelections();
	CTR_TEST_CHECK(!CustomCharacter_IsSelected(3) && CustomCharacter_IsSelected(0) && CustomCharacter_IsSelected(2),
	               "returning to selection restores the backed-up human choices instead of AI assignments");
	LOAD_RosterPrepareAvailable(&roster, 3, false, false, IsUnlocked);
	CTR_TEST_CHECK_EQ(roster.count, GAME_CHARACTER_COUNT - 1, "Vanilla never adds custom opponents");
	for (int i = 0; i < roster.count; i++)
	{
		CTR_TEST_CHECK(roster.entries[i].custom == NULL, "every Vanilla candidate is retail");
	}
	LOAD_RosterSelectDriver(&roster.entries[0], 3);
	CTR_TEST_CHECK(!CustomCharacter_IsSelected(3),
	               "retail reassignment clears custom resources");
	CustomCharacter_Select(3, active[1]);
	CustomCharacter_Select(len(sdata->customCharacters) - 1, active[2]);
	LOAD_RosterFinalize(3, 0);
	CTR_TEST_CHECK(!CustomCharacter_IsSelected(3) && sdata->customCharacters[len(sdata->customCharacters) - 1].identity[0] == '\0' &&
	               CustomCharacter_IsSelected(0) && sdata->loadedRaceFieldSize == 0,
	               "managed rosters discard stale custom opponents and reset the configured field without clearing humans");

	CTR_TEST_SECTION("roster finalization commits the admitted field once");
	struct CustomCharacterSelection saved = sdata->customCharacters[0];
	for (int driver = 3; driver < (int)len(sdata->customCharacters); driver++)
	{
		data.characterIDs[driver] = PAPU_PAPU;
		CustomCharacter_Select(driver, active[1]);
	}
	LOAD_RosterFinalize(3, 6);
	CTR_TEST_CHECK(sdata->loadedRaceFieldSize == 6 && CustomCharacter_IsSelected(3) && CustomCharacter_IsSelected(5),
	               "admitted opponents retain their custom resources in the published field");
	CTR_TEST_CHECK(!CustomCharacter_IsSelected(6) && !CustomCharacter_IsSelected(len(sdata->customCharacters) - 1),
	               "opponents excluded by the model queue are pruned using the final field, not the requested count");
	CTR_TEST_CHECK(memcmp(&saved, &sdata->customCharacters[0], sizeof(saved)) == 0,
	               "finalization leaves human resource snapshots unchanged");
	LOAD_RosterFinalize(3, 3);
	CTR_TEST_CHECK(sdata->loadedRaceFieldSize == 3 && !CustomCharacter_IsSelected(3) && CustomCharacter_IsSelected(2),
	               "a configured human-only field clears all former AI selections");
	LOAD_RosterFinalize(2, 0);
	CTR_TEST_CHECK(sdata->loadedRaceFieldSize == 0 && CustomCharacter_IsSelected(0) && !CustomCharacter_IsSelected(2),
	               "unconfigured modes preserve their explicit human count, including co-op hub slots");
}

int main(void)
{
	TestRetailRoster();
	TestCustomRoster();
	return CtrTest_Summary();
}

#include <shared/ctr_test.h>
#include <game/custom-characters/character_policy.h>

struct sData sdata_static;
struct Data data;
static struct GameTracker s_tracker;
static struct NativeCharacterMetadata s_character;
static bool s_hasCharacter;
static int s_libraryReads;
static int s_voiceIndex;

const char *NativeCharacterLibrary_GetVoicePath(const char *directory, int index, int xa)
{
	(void)xa;
	s_voiceIndex = index;
	return directory;
}

const struct NativeCharacterMetadata *NativeCharacterLibrary_Get(size_t index)
{
	s_libraryReads++;
	return s_hasCharacter && index == 0 ? &s_character : NULL;
}

static void ResetSelection(void)
{
	memset(&s_tracker, 0, sizeof(s_tracker));
	memset(sdata->customCharacters, 0, sizeof(sdata->customCharacters));
	sdata->customCharacterAudioDriver = 1;
	sdata->gGT = &s_tracker;
	s_tracker.gameMode1 = ARCADE_MODE;
	s_tracker.numPlyrNextGame = 1;
	s_tracker.numPlyrCurrGame = 1;
	data.characterIDs[0] = POLAR;
	sdata->customCharacters[0].baseID = POLAR;
	data.MetaDataCharacters[POLAR].name_Debug = "polar";
	data.MetaDataCharacters[CRASH_BANDICOOT].name_Debug = "crash";
	data.MetaDataCharacters[PAPU_PAPU].name_Debug = "papu";
	data.MetaDataCharacters[RIPPER_ROO].name_Debug = "roo";
	strcpy(sdata->customCharacters[0].identity, "fixture");
	strcpy(sdata->customCharacters[0].audioDirectory, "fixture-voices");
	strcpy(s_character.base, "polar");
	s_hasCharacter = true;
	s_libraryReads = 0;
}

int main(void)
{
	CTR_TEST_SECTION("character availability is independent of mode and player count");
	const struct
	{
		u32 mode;
		int players;
	} cases[] = {
		{ARCADE_MODE, 1},
		{ARCADE_MODE | MAIN_MENU, 1},
		{ARCADE_MODE, 2},
		{ARCADE_MODE, 3},
		{ARCADE_MODE, 4},
		{MAIN_MENU, 1},
		{TIME_TRIAL, 1},
		{ADVENTURE_MODE, 1},
		{BATTLE_MODE, 1},
	};
	for (u32 i = 0; i < len(cases); i++)
	{
		ResetSelection();
		s_tracker.gameMode1 = cases[i].mode;
		s_tracker.numPlyrNextGame = cases[i].players;
		CTR_TEST_CHECK(CustomCharacter_GetMenuEntry(0) != NULL,
		               "an installed character is available in every menu layout");
		CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) != NULL,
		               "selected character audio does not depend on mode or player count");
		CTR_TEST_CHECK(s_libraryReads == 1, "only menu entry lookup accesses the custom library");
	}

	CTR_TEST_SECTION("only selectable entries expand the menu and load icons");
	ResetSelection();
	CTR_TEST_CHECK(CustomCharacter_GetMenuEntry(0) == &s_character, "a Polar-based entry is selectable");
	strcpy(s_character.base, "crash");
	CTR_TEST_CHECK(CustomCharacter_GetMenuEntry(0) == &s_character, "a Crash-based entry is also selectable");
	strcpy(s_character.base, "papu");
	CTR_TEST_CHECK(CustomCharacter_GetMenuEntry(0) == &s_character, "a boss-based entry is selectable");
	strcpy(s_character.base, "unknown");
	CTR_TEST_CHECK(CustomCharacter_GetMenuEntry(0) == NULL, "an unknown retail base cannot index retail tables");
	s_hasCharacter = false;
	CTR_TEST_CHECK(CustomCharacter_GetMenuEntry(0) == NULL, "a missing entry does not expand the roster or load an icon");

	CTR_TEST_SECTION("audio follows the selected identity, not a library scan");
	ResetSelection();
	s_hasCharacter = false;
	CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) == sdata->customCharacters[0].audioDirectory && s_libraryReads == 0,
	               "a restored selection resolves audio without consulting the library");
	sdata->customCharacters[0].identity[0] = '\0';
	CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) == NULL, "retail Polar does not use custom audio");
	sdata->gGT = NULL;
	CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) == NULL, "an empty selection needs no initialized game tracker to use retail audio");
	ResetSelection();
	sdata->customCharacters[0].audioDirectory[0] = '\0';
	CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) == NULL, "a selection without a voice directory uses retail audio");
	ResetSelection();
	data.characterIDs[0] = CRASH_BANDICOOT;
	CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) == NULL, "other retail characters ignore a retained custom identity");
	ResetSelection();
	s_tracker.numPlyrCurrGame = 4;
	CTR_TEST_CHECK(CustomCharacter_GetAudioDirectory(0) != NULL, "audio is independent of the previous race's player count");

	CTR_TEST_SECTION("model admission and audio follow the selected retail base");
	data.characterIDs[0] = PAPU_PAPU;
	sdata->customCharacters[0].baseID = PAPU_PAPU;
	CTR_TEST_CHECK(CustomCharacter_IsSelected(0), "a non-Polar base retains the custom selection");
	CTR_TEST_CHECK(CustomCharacter_GetShortVoiceIndex(28 + PAPU_PAPU) == 0 &&
	               CustomCharacter_GetShortVoiceIndex(44 + PAPU_PAPU) == 1,
	               "short voices map the selected base to local indices");
	CTR_TEST_CHECK(CustomCharacter_GetShortVoiceIndex(28 + POLAR) == -1, "other bases do not inherit these custom short voices");
	s16 papuLines[] = {171, 172};
	data.voiceData[PAPU_PAPU].voiceSet[0].ptr = papuLines;
	data.voiceData[PAPU_PAPU].voiceSet[0].num = 2;
	CTR_TEST_CHECK(CustomCharacter_ResolveXAVoice(2, 172) != NULL && s_voiceIndex == 1,
	               "XA requests map through the selected base's voice sets");
	CTR_TEST_CHECK(CustomCharacter_ResolveXAVoice(2, 114) == NULL && CustomCharacter_ResolveXAVoice(1, 172) == NULL,
	               "other characters and XA categories keep their normal source");
	data.characterIDs[0] = RIPPER_ROO;
	sdata->customCharacters[0].baseID = RIPPER_ROO;
	s16 rooLines[] = {200, 201, 192, 193};
	data.voiceData[RIPPER_ROO].voiceSet[0].ptr = rooLines;
	data.voiceData[RIPPER_ROO].voiceSet[0].num = 2;
	data.voiceData[RIPPER_ROO].voiceSet[1].ptr = rooLines + 2;
	data.voiceData[RIPPER_ROO].voiceSet[1].num = 2;
	CTR_TEST_CHECK(CustomCharacter_ResolveXAVoice(2, 200) != NULL && s_voiceIndex == 0,
	               "Ripper Roo's out-of-order XA IDs map by voice role, not ID subtraction");
	CTR_TEST_CHECK(CustomCharacter_ResolveXAVoice(2, 192) != NULL && s_voiceIndex == 2,
	               "local indices continue across voice sets");

	CTR_TEST_SECTION("selection owns the complete resource snapshot");
	strcpy(s_character.identity, "selected-character");
	strcpy(s_character.base, "papu");
	strcpy(s_character.modelHi, "fixture/models/hi.ctr");
	strcpy(s_character.iconPixels, "fixture/icon/pixels.bin");
	strcpy(s_character.iconClut, "fixture/icon/palette.bin");
	strcpy(s_character.directory, "fixture/character");
	data.characterIDs[0] = PAPU_PAPU;
	CustomCharacter_Select(0, &s_character);
	CTR_TEST_CHECK(CustomCharacter_IsSelected(0) && sdata->customCharacters[0].baseID == PAPU_PAPU &&
	               strcmp(sdata->customCharacters[0].identity, s_character.identity) == 0,
	               "selecting an entry stores its identity and retail base together");
	CTR_TEST_CHECK(strcmp(sdata->customCharacters[0].iconPixels, s_character.iconPixels) == 0 &&
	               strcmp(sdata->customCharacters[0].iconClut, s_character.iconClut) == 0,
	               "selection snapshots its icon and palette paths independently of future library refreshes");
	CTR_TEST_CHECK(strcmp(sdata->customCharacters[0].modelPath, s_character.modelHi) == 0 &&
	               strcmp(sdata->customCharacters[0].audioDirectory, s_character.directory) == 0,
	               "selection stores the model and audio resource paths together");
	CustomCharacter_Select(0, NULL);
	CTR_TEST_CHECK(!CustomCharacter_IsSelected(0) && sdata->customCharacters[0].baseID == -1 &&
	               sdata->customCharacters[0].modelPath[0] == '\0' && sdata->customCharacters[0].audioDirectory[0] == '\0',
	               "retail selection clears the complete custom snapshot");
	CustomCharacter_Select(0, &s_character);
	strcpy(s_character.base, "invalid");
	CustomCharacter_Select(0, &s_character);
	CTR_TEST_CHECK(sdata->customCharacters[0].identity[0] == '\0' && sdata->customCharacters[0].baseID == -1 &&
	               sdata->customCharacters[0].modelPath[0] == '\0' && sdata->customCharacters[0].audioDirectory[0] == '\0',
	               "an invalid retail base cannot leave stale custom resources selected");

	CTR_TEST_SECTION("multiplayer selections and speaker ownership are independent");
	ResetSelection();
	s_tracker.numPlyrCurrGame = len(sdata->customCharacters);
	strcpy(s_character.base, "papu");
	for (int player = 0; player < s_tracker.numPlyrCurrGame; player++)
	{
		snprintf(s_character.identity, sizeof(s_character.identity), "character-%d", player);
		snprintf(s_character.directory, sizeof(s_character.directory), "voices-%d", player);
		data.characterIDs[player] = PAPU_PAPU;
		CustomCharacter_Select(player, &s_character);
		CTR_TEST_CHECK(CustomCharacter_IsSelected(player) &&
		               strcmp(sdata->customCharacters[player].identity, s_character.identity) == 0,
		               "each human keeps a separate custom identity even with the same retail base");
	}
	sdata->customCharacterAudioDriver = 0;
	CTR_TEST_CHECK(CustomCharacter_GetShortVoiceIndex(28 + PAPU_PAPU) == -1 &&
	               CustomCharacter_ResolveXAVoice(2, 172) == NULL,
	               "an ambiguous unowned voice never borrows an arbitrary player's assets");
	for (int player = 0; player < s_tracker.numPlyrCurrGame; player++)
	{
		sdata->customCharacterAudioDriver = player + 1;
		CTR_TEST_CHECK(CustomCharacter_GetAudioPlayer(PAPU_PAPU) == player &&
		               CustomCharacter_GetShortVoiceIndex(28 + PAPU_PAPU) == 0 &&
		               CustomCharacter_ResolveXAVoice(2, 172) == sdata->customCharacters[player].audioDirectory,
		               "owned short and XA voices resolve the actual speaking player's directory");
	}
	CustomCharacter_Select(1, NULL);
	sdata->customCharacterAudioDriver = 2;
	CTR_TEST_CHECK(CustomCharacter_ResolveXAVoice(2, 172) == NULL && CustomCharacter_IsSelected(0) && CustomCharacter_IsSelected(3),
	               "a retail speaker sharing the custom base leaves the other selections and audio untouched");
	sdata->customCharacterAudioDriver = 5;
	CTR_TEST_CHECK(CustomCharacter_GetShortVoiceIndex(28 + PAPU_PAPU) == -1,
	               "AI speakers do not inherit human custom voices");
	struct CustomCharacterSelection saved = sdata->customCharacters[0];
	CustomCharacter_Select(-1, NULL);
	CustomCharacter_Select(len(sdata->customCharacters), NULL);
	CTR_TEST_CHECK(memcmp(&saved, &sdata->customCharacters[0], sizeof(saved)) == 0 &&
	               !CustomCharacter_IsSelected(-1) && !CustomCharacter_IsSelected(len(sdata->customCharacters)),
	               "out-of-range player slots cannot change a real player's selection");
	return CtrTest_Summary();
}

#ifndef CTR_CHARACTER_POLICY_H
#define CTR_CHARACTER_POLICY_H

#include <common.h>
#include <platform/custom-characters/native_character_library.h>
#include <string.h>

static inline int CustomCharacter_GetBaseID(const char *base)
{
	if (base == NULL) { return -1; }
	for (int i = 0; i < GAME_CHARACTER_COUNT; i++)
	{
		const char *name = GAME_CHARACTER_METADATA[i].name_Debug;
		if (name != NULL && strcmp(base, name) == 0) { return i; }
	}
	return -1;
}

static inline const struct NativeCharacterMetadata *CustomCharacter_GetMenuEntry(size_t index)
{
	const struct NativeCharacterMetadata *entry = NativeCharacterLibrary_Get(index);
	return entry != NULL && CustomCharacter_GetBaseID(entry->base) >= 0 ? entry : NULL;
}

static inline void CustomCharacter_Select(int playerIndex, const struct NativeCharacterMetadata *entry)
{
	if (playerIndex < 0 || playerIndex >= (int)len(sdata->customCharacters)) { return; }
	struct CustomCharacterSelection *selection = &sdata->customCharacters[playerIndex];
	int baseID = entry != NULL ? CustomCharacter_GetBaseID(entry->base) : -1;
	if (baseID < 0)
	{
		memset(selection, 0, sizeof(*selection));
		selection->baseID = -1;
		return;
	}

	memcpy(selection->identity, entry->identity, sizeof(selection->identity));
	selection->baseID = baseID;
	memcpy(selection->modelPath, entry->modelHi, sizeof(selection->modelPath));
	memcpy(selection->iconPixels, entry->iconPixels, sizeof(selection->iconPixels));
	memcpy(selection->iconClut, entry->iconClut, sizeof(selection->iconClut));
	memcpy(selection->audioDirectory, entry->directory, sizeof(selection->audioDirectory));
}

static inline bool CustomCharacter_IsSelected(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= (int)len(sdata->customCharacters)) { return false; }
	const struct CustomCharacterSelection *selection = &sdata->customCharacters[playerIndex];
	return selection->identity[0] != '\0' && selection->baseID >= 0 &&
	       selection->baseID < GAME_CHARACTER_COUNT && data.characterIDs[playerIndex] == selection->baseID;
}

static inline const char *CustomCharacter_GetAudioDirectory(int playerIndex)
{
	if (!CustomCharacter_IsSelected(playerIndex) || sdata->customCharacters[playerIndex].audioDirectory[0] == '\0')
	{
		return NULL;
	}
	return sdata->customCharacters[playerIndex].audioDirectory;
}

// A retail sound ID alone cannot distinguish custom players from their base.
// Only explicitly owned requests may resolve custom voice resources.
static inline int CustomCharacter_GetAudioPlayer(int baseID)
{
	int player = sdata->customCharacterAudioDriver - 1;
	return CustomCharacter_IsSelected(player) && sdata->customCharacters[player].baseID == baseID ? player : -1;
}

static inline int CustomCharacter_GetShortVoiceIndex(int soundID)
{
	enum { FIRST_VOICE = 28, SECOND_VOICE = 44 };
	int index = soundID >= SECOND_VOICE ? 1 : 0;
	int baseID = soundID - (index != 0 ? SECOND_VOICE : FIRST_VOICE);
	return baseID >= 0 && baseID < GAME_CHARACTER_COUNT && CustomCharacter_GetAudioPlayer(baseID) >= 0 ? index : -1;
}

static inline const char *CustomCharacter_ResolveXAVoice(int categoryID, int xaID)
{
	enum { GAME_VOICES = 2 };
	if (categoryID != GAME_VOICES || xaID < 0)
	{
		return NULL;
	}
	for (int baseID = 0; baseID < GAME_CHARACTER_COUNT; baseID++)
	{
		int player = CustomCharacter_GetAudioPlayer(baseID);
		const char *directory = CustomCharacter_GetAudioDirectory(player);
		if (directory == NULL) { continue; }
		int localIndex = 0;
		// Retail does not order every character's XA IDs alike.
		for (size_t set = 0; set < len(data.voiceData[baseID].voiceSet); set++)
		{
			const s16 *ids = data.voiceData[baseID].voiceSet[set].ptr;
			int count = data.voiceData[baseID].voiceSet[set].num;
			for (int i = 0; ids != NULL && i < count; i++)
			{
				if (ids[i] == xaID)
				{
					return NativeCharacterLibrary_GetVoicePath(directory, localIndex + i, 1);
				}
			}
			localIndex += count;
		}
	}
	return NULL;
}

#endif

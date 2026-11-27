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

static inline const struct NativeCharacterMetadata *CustomCharacter_GetMenuEntry(void)
{
	const struct NativeCharacterMetadata *entry = NativeCharacterLibrary_Get(0);
	return entry != NULL && CustomCharacter_GetBaseID(entry->base) >= 0 ? entry : NULL;
}

static inline void CustomCharacter_Select(const struct NativeCharacterMetadata *entry)
{
	int baseID = entry != NULL ? CustomCharacter_GetBaseID(entry->base) : -1;
	if (baseID < 0)
	{
		sdata->customPlayerIdentity[0] = '\0';
		sdata->customPlayerBaseID = -1;
		sdata->customPlayerModelPath[0] = '\0';
		sdata->customPlayerAudioDirectory[0] = '\0';
		return;
	}

	memcpy(sdata->customPlayerIdentity, entry->identity, sizeof(sdata->customPlayerIdentity));
	sdata->customPlayerBaseID = baseID;
	memcpy(sdata->customPlayerModelPath, entry->modelHi, sizeof(sdata->customPlayerModelPath));
	memcpy(sdata->customPlayerAudioDirectory, entry->directory, sizeof(sdata->customPlayerAudioDirectory));
}

static inline bool CustomCharacter_IsSelected(void)
{
	return sdata->customPlayerIdentity[0] != '\0' && sdata->customPlayerBaseID >= 0 &&
	       sdata->customPlayerBaseID < GAME_CHARACTER_COUNT && data.characterIDs[0] == sdata->customPlayerBaseID;
}

static inline const char *CustomCharacter_GetAudioDirectory(void)
{
	if (!CustomCharacter_IsSelected() || sdata->customPlayerAudioDirectory[0] == '\0')
	{
		return NULL;
	}
	return sdata->customPlayerAudioDirectory;
}

static inline int CustomCharacter_GetShortVoiceIndex(int soundID)
{
	enum { FIRST_VOICE = 28, SECOND_VOICE = 44 };
	if (!CustomCharacter_IsSelected()) { return -1; }
	int baseID = sdata->customPlayerBaseID;
	return soundID == FIRST_VOICE + baseID ? 0 : soundID == SECOND_VOICE + baseID ? 1 : -1;
}

static inline const char *CustomCharacter_ResolveXAVoice(int categoryID, int xaID)
{
	enum { GAME_VOICES = 2 };
	if (categoryID != GAME_VOICES || xaID < 0)
	{
		return NULL;
	}
	const char *directory = CustomCharacter_GetAudioDirectory();
	if (directory == NULL) { return NULL; }
	int baseID = sdata->customPlayerBaseID;
	int localIndex = 0;
	// Use the voice sets: retail does not order every character's XA IDs alike.
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
	return NULL;
}

#endif

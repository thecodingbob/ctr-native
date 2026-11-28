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

// Shared active roster for selection, portraits and random opponents. Library
// activation policy belongs here, not in the consumers of the prepared entries.
static inline int CustomCharacter_PrepareAvailableEntries(const struct NativeCharacterMetadata **entries, int capacity)
{
	int count = 0;
	for (size_t index = 0; index < NativeCharacterLibrary_Count() && count < capacity && count < CHARACTER_SELECT_EXTRA_SLOT_CAPACITY; index++)
	{
		const struct NativeCharacterMetadata *entry = CustomCharacter_GetMenuEntry(index);
		if (entry != NULL) { entries[count++] = entry; }
	}
	return count;
}

static inline void CustomCharacter_Select(int driverID, const struct NativeCharacterMetadata *entry)
{
	if (driverID < 0 || driverID >= (int)len(sdata->customCharacters)) { return; }
	struct CustomCharacterSelection *selection = &sdata->customCharacters[driverID];
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

static inline bool CustomCharacter_IsSelected(int driverID)
{
	if (driverID < 0 || driverID >= (int)len(sdata->customCharacters)) { return false; }
	const struct CustomCharacterSelection *selection = &sdata->customCharacters[driverID];
	return selection->identity[0] != '\0' && selection->baseID >= 0 &&
	       selection->baseID < GAME_CHARACTER_COUNT && data.characterIDs[driverID] == selection->baseID;
}

static inline void CustomCharacter_ClearDriversFrom(int driverID)
{
	for (; driverID < (int)len(sdata->customCharacters); driverID++)
	{
		CustomCharacter_Select(driverID, NULL);
	}
}

static inline void CustomCharacter_BackupSelections(void)
{
	memcpy(sdata->characterIDs_backup, data.characterIDs, sizeof(sdata->characterIDs_backup));
	memcpy(sdata->customCharactersBackup, sdata->customCharacters, sizeof(sdata->customCharactersBackup));
}

static inline void CustomCharacter_RestoreSelections(void)
{
	memcpy(data.characterIDs, sdata->characterIDs_backup, sizeof(sdata->characterIDs_backup));
	memcpy(sdata->customCharacters, sdata->customCharactersBackup, sizeof(sdata->customCharactersBackup));
	CustomCharacter_ClearDriversFrom(len(sdata->customCharactersBackup));
}

static inline const char *CustomCharacter_GetAudioDirectory(int driverID)
{
	if (!CustomCharacter_IsSelected(driverID) || sdata->customCharacters[driverID].audioDirectory[0] == '\0')
	{
		return NULL;
	}
	return sdata->customCharacters[driverID].audioDirectory;
}

// A retail sound ID alone cannot distinguish custom drivers from their base.
// Only explicitly owned requests may resolve custom voice resources.
static inline int CustomCharacter_GetAudioDriver(int baseID)
{
	int driver = sdata->customCharacterAudioDriver - 1;
	return CustomCharacter_IsSelected(driver) && sdata->customCharacters[driver].baseID == baseID ? driver : -1;
}

static inline int CustomCharacter_GetShortVoiceIndex(int soundID)
{
	enum { FIRST_VOICE = 28, SECOND_VOICE = 44 };
	int index = soundID >= SECOND_VOICE ? 1 : 0;
	int baseID = soundID - (index != 0 ? SECOND_VOICE : FIRST_VOICE);
	return baseID >= 0 && baseID < GAME_CHARACTER_COUNT && CustomCharacter_GetAudioDriver(baseID) >= 0 ? index : -1;
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
		int driver = CustomCharacter_GetAudioDriver(baseID);
		const char *directory = CustomCharacter_GetAudioDirectory(driver);
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

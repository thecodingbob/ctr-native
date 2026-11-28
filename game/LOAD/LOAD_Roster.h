#ifndef LOAD_ROSTER_H
#define LOAD_ROSTER_H

#include <game/custom-characters/character_policy.h>

struct LOAD_RosterEntry
{
	s16 baseID;
	const struct NativeCharacterMetadata *custom;
};

struct LOAD_Roster
{
	struct LOAD_RosterEntry entries[GAME_CHARACTER_COUNT + CHARACTER_SELECT_EXTRA_SLOT_CAPACITY];
	int count;
};

static inline bool LOAD_RosterEntryMatchesDriver(const struct LOAD_RosterEntry *entry, int driverID)
{
	if (entry->baseID != data.characterIDs[driverID]) { return false; }
	bool custom = CustomCharacter_IsSelected(driverID);
	return entry->custom != NULL ? custom && strcmp(entry->custom->identity, sdata->customCharacters[driverID].identity) == 0 : !custom;
}

static inline void LOAD_RosterAddAvailable(struct LOAD_Roster *roster, struct LOAD_RosterEntry entry, int occupiedDrivers)
{
	for (int driver = 0; driver < occupiedDrivers; driver++)
	{
		if (LOAD_RosterEntryMatchesDriver(&entry, driver)) { return; }
	}
	roster->entries[roster->count++] = entry;
}

// Retail unlock rules never apply to customs. Exclusion is by identity, so a
// custom and its compatibility base remain independent candidates.
static inline void LOAD_RosterPrepareAvailable(struct LOAD_Roster *roster, int occupiedDrivers,
                                               bool includeCustom, bool unlockedOnly, b32 (*isUnlocked)(s16))
{
	roster->count = 0;
	for (int id = 0; id < GAME_CHARACTER_COUNT; id++)
	{
		if (!unlockedOnly || isUnlocked(id))
		{
			LOAD_RosterAddAvailable(roster, (struct LOAD_RosterEntry){.baseID = id}, occupiedDrivers);
		}
	}
	if (includeCustom)
	{
		const struct NativeCharacterMetadata *customs[CHARACTER_SELECT_EXTRA_SLOT_CAPACITY];
		int count = CustomCharacter_PrepareAvailableEntries(customs, len(customs));
		for (int index = 0; index < count; index++)
		{
			LOAD_RosterAddAvailable(roster, (struct LOAD_RosterEntry){
				.baseID = CustomCharacter_GetBaseID(customs[index]->base), .custom = customs[index],
			}, occupiedDrivers);
		}
	}
}

// Consume an existing cup opponent without reassigning its resource snapshot.
static inline bool LOAD_RosterRetainDriver(struct LOAD_Roster *roster, int driverID)
{
	for (int index = 0; index < roster->count; index++)
	{
		if (LOAD_RosterEntryMatchesDriver(&roster->entries[index], driverID))
		{
			memmove(&roster->entries[index], &roster->entries[index + 1],
			        (roster->count - index - 1) * sizeof(roster->entries[0]));
			roster->count--;
			return true;
		}
	}
	return false;
}

static inline void LOAD_RosterSelectDriver(const struct LOAD_RosterEntry *entry, int driverID)
{
	CustomCharacter_Select(driverID, entry->custom);
	data.characterIDs[driverID] = entry->baseID;
}

// Publish only the admitted configured field. Managed/retail rosters pass zero
// and retain custom resources only for their humans (including hub partners).
static inline void LOAD_RosterFinalize(int humanCount, int admittedRacers)
{
	sdata->loadedRaceFieldSize = admittedRacers;
	CustomCharacter_ClearDriversFrom(admittedRacers > humanCount ? admittedRacers : humanCount);
}

#endif

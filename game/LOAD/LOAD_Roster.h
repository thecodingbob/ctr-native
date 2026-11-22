#ifndef LOAD_ROSTER_H
#define LOAD_ROSTER_H

// A candidate is available to a bot only if no human already uses it. Unlocks
// restrict Random Unlocked, but not Vanilla or Random All.
static inline b32 LOAD_RosterCharacterAvailable(s16 characterID, const s16 *humanCharacters, int humanCount,
                                                b32 unlockedOnly, b32 (*isUnlocked)(s16))
{
	for (int i = 0; i < humanCount; i++)
	{
		if (humanCharacters[i] == characterID)
		{
			return false;
		}
	}

	return !unlockedOnly || isUnlocked(characterID);
}

static inline int LOAD_RosterAvailableBotCount(int characterCount, const s16 *humanCharacters, int humanCount,
                                               b32 unlockedOnly, b32 (*isUnlocked)(s16))
{
	int available = 0;
	for (int id = 0; id < characterCount; id++)
	{
		if (LOAD_RosterCharacterAvailable((s16)id, humanCharacters, humanCount, unlockedOnly, isUnlocked))
		{
			available++;
		}
	}
	return available;
}

#endif

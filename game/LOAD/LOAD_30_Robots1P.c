#include <common.h>

void LOAD_Robots1P(int characterID)
{
	int newCharacterID = 0;
	for (int i = 1; i < 8; i++, newCharacterID++)
	{
		if (newCharacterID == characterID)
			newCharacterID++;

		data.characterIDs[i] = newCharacterID;
	}
}

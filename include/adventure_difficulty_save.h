#ifndef CTR_NATIVE_ADVENTURE_DIFFICULTY_SAVE_H
#define CTR_NATIVE_ADVENTURE_DIFFICULTY_SAVE_H

#include <namespace_Memcard.h>

static inline enum AdventureDifficulty AdvDifficultySave_Decode(u8 saved)
{
	if (saved >= ADV_DIFFICULTY_COUNT)
	{
		return ADV_DIFFICULTY_NORMAL;
	}
	return (enum AdventureDifficulty)saved;
}

static inline u8 AdvDifficultySave_Encode(enum AdventureDifficulty difficulty)
{
	return (unsigned int)difficulty < ADV_DIFFICULTY_COUNT ? (u8)difficulty : ADV_DIFFICULTY_NORMAL;
}

#endif

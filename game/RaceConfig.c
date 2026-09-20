#include <common.h>

#ifndef RACECONFIG_OPTIONS_LOADED
#define RACECONFIG_OPTIONS_LOADED sdata->boolHasLoadedOptions
#endif

void RaceConfig_LoadGameOptions(void)
{
	struct GameSave *save;
	s16 i;

	if (RACECONFIG_OPTIONS_LOADED != 0)
	{
		return;
	}

	RACECONFIG_OPTIONS_LOADED = 1;
	i = 0;
	save = &GAME_SAVE;

	for (; i < GAME_OPTIONS_VOLUME_COUNT; i++)
	{
		// NOTE(aalhendi): Index-first addition preserves the retail address operands.
		howl_VolumeSet(i, (u8) * (i + save->options.volumes));
		// Retail refreshes the wheel settings after each volume channel.
		memcpy(&data.rwd[0], &save->options.rwd[0], sizeof(data.rwd));
	}

	GAME_TRACKER->gameMode1 |= GAME_SAVE.options.gameMode1_vibrationFlags & GAME_MODE_VIBRATION_MASK;
	howl_ModeSet((u8)GAME_SAVE.options.audioMode & 1);
}

void RaceConfig_SaveGameOptions(void)
{
	s16 i = 0;
	struct GameSave *save = &GAME_SAVE;

	for (; i < GAME_OPTIONS_VOLUME_COUNT; i++)
	{
		*(i + save->options.volumes) = howl_VolumeGet(i) & 0xff;
	}

	memcpy(&GAME_SAVE.options.rwd[0], &data.rwd[0], sizeof(data.rwd));
	GAME_SAVE.options.gameMode1_vibrationFlags = GAME_TRACKER->gameMode1 & GAME_MODE_VIBRATION_MASK;
	GAME_SAVE.options.audioMode = (u8)howl_ModeGet() != 0;
}

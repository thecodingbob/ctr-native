#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029c40-0x80029ca4
int howl_SetSong(int songID)
{
	if (sdata->boolAudioEnabled == 0)
	{
		// Stage 3: Finished
		sdata->songLoadStage = 3;

		return 1;
	}

	// === Reset Song ===

	howl_ErasePtrCseqHeader();

	// Stage 0: Start Loading
	sdata->songLoadStage = 0;

	sdata->songSectorOffset = sdata->howl_songOffsets[songID & 0xffff];
	return 1;
}

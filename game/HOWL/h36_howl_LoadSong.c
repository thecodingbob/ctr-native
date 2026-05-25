#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029ca4-0x80029dc0
// similar to h23_Bank_AssignSpuAddrs, and h34_howl_LoadHeader
int howl_LoadSong()
{
	int ret;

	// Stage 3: Finished
	if (sdata->songLoadStage == 3)
	{
		return 1;
	}

	// Stage 0: Load 1/2
	if (sdata->songLoadStage == 0)
	{
		ret = LOAD_HowlSectorChainStart(&sdata->KartHWL_CdFile,  // CdLoc of HOWL
		                                sdata->sampleBlock1,     // destination in RAM for songs
		                                sdata->songSectorOffset, // song offset on disc, from CdLoc
		                                1                        // one sector
		);

		if (ret != 0)
		{
			// go to next stage
			sdata->songLoadStage++;
		}

		return 0;
	}

	// Stage 1: Load 2/2
	if (sdata->songLoadStage == 1)
	{
		if (LOAD_HowlSectorChainEnd() == 0)
			return 0;

		// CseqHeader->songSize, aligned up to sector size
		int numSector = (*(int *)&sdata->sampleBlock1[0] + 0x7ff) >> 0xb;

		ret = LOAD_HowlSectorChainStart(&sdata->KartHWL_CdFile,      // CdLoc of HOWL
		                                sdata->tenSampleBlocks,      // (sampleBlock1+0x800) RAM destination
		                                sdata->songSectorOffset + 1, // song offset on disc, from CdLoc
		                                numSector - 1);

		if (ret != 0)
		{
			// go to next stage
			sdata->songLoadStage++;
		}

		return 0;
	}

	// Stage 2: Parsing Song
	if (sdata->songLoadStage == 2)
	{
		if (LOAD_HowlSectorChainEnd() == 0)
			return 0;

		howl_ParseCseqHeader((struct CseqHeader *)sdata->sampleBlock1);

		// go to next stage
		sdata->songLoadStage++;
		return 1;
	}

	return 0;
}

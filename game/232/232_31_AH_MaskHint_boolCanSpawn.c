#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3f88-0x800b3f98.
int AH_MaskHint_boolCanSpawn()
{
	// 0 - aku is gone,
	// 1 - aku is speaking

	// return 0, if aku is speaking -- can't spawn
	// return 1, if aku is gone -- can spawn

	return sdata->AkuAkuHintState == 0;
}

#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029a50-0x80029ab4
void howl_ParseHeader(struct HowlHeader *hh)
{
	u32 addr = (u32)hh;

	sdata->ptrHowlHeader = (struct HowlHeader *)addr;
	addr += sizeof(struct HowlHeader);

	sdata->howl_spuAddrs = (struct SpuAddrEntry *)addr;
	addr += sizeof(struct SpuAddrEntry) * hh->numSpuAddrs;

	sdata->howl_metaOtherFX = (struct OtherFX *)addr;
	addr += sizeof(struct OtherFX) * hh->numOtherFX;

	sdata->howl_metaEngineFX = (struct EngineFX *)addr;
	addr += sizeof(struct EngineFX) * hh->numEngineFX;

	sdata->howl_bankOffsets = (u16 *)addr;
	addr += sizeof(s16) * hh->numBanks;

	sdata->howl_songOffsets = (u16 *)addr;
	addr += sizeof(s16) * hh->numSequences;

	sdata->howl_endOfHowl = addr;
}

#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029ab4-0x80029b2c
void howl_ParseCseqHeader(struct CseqHeader *ch)
{
	u32 addr = (u32)ch;

	sdata->ptrCseqHeader = (struct CseqHeader *)addr;
	addr += sizeof(struct CseqHeader);

	sdata->ptrCseqLongSamples = (struct SampleInstrument *)addr;
	addr += sizeof(struct SampleInstrument) * ch->numLongSamples;

	sdata->ptrCseqShortSamples = (struct SampleDrums *)addr;
	addr += sizeof(struct SampleDrums) * ch->numShortSamples;

	sdata->ptrCseqSongStartOffset = (s16 *)addr;
	addr += sizeof(s16) * ch->numSongs;

	addr = (addr + 3) & ~3;

	sdata->ptrCseqSongData = (char *)addr;
}

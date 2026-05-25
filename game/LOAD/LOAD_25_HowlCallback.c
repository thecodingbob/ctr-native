#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003254c-0x80032594.
void LOAD_HowlCallback(CdlIntrResult result, u8 *unk)
{
	// disable callback
	CdReadCallback(0);

	if (result == CdlComplete)
		sdata->howlChainState = 0;
	else
		sdata->howlChainState = -1;
}

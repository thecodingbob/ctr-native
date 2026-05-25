#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003266c-0x800326b4.
int LOAD_HowlSectorChainEnd()
{
	int *howlChainParams;
	int howlChainState = sdata->howlChainState;
	howlChainParams = sdata->howlChainParams;

	if (howlChainState == -1)
	{
		LOAD_HowlSectorChainStart((CdlFILE *)howlChainParams[0], (void *)howlChainParams[1], howlChainParams[2], howlChainParams[3]);

		return 0;
	}

	return (howlChainState == 0);
}

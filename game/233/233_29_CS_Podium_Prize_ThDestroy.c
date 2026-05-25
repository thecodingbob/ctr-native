#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afe58-0x800afe90
void CS_Podium_Prize_ThDestroy(struct Thread *t)
{
	// remove bits
	sdata->gGT->gameMode2 &= ~(INC_RELIC | INC_KEY | INC_TROPHY);
	PROC_DestroyInstance(t);
}

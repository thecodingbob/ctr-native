#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae81c-0x800ae834
void CS_LoadBossCallback(struct LoadQueueSlot *lqs)
{
	void *ptr = lqs->ptrDestination;
	sdata->load_inProgress = 0;
	D233.ptrModelBossHead = ptr;
}

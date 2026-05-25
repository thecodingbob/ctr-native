#include <common.h>

void PROC_DestroyTracker(struct Thread *t)
{
	struct GameTracker *gGT = sdata->gGT;

	if (gGT->numMissiles > 0)
		gGT->numMissiles--;

	PROC_DestroyInstance(t);
}

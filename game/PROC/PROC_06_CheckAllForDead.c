#include <common.h>

void PROC_CheckAllForDead()
{
	int i;

	struct ThreadBucket *tb = &sdata->gGT->threadBuckets[0];

	for (i = 0; i < NUM_BUCKETS; i++)
	{
		PROC_CheckBloodlineForDead(&tb[i].thread, tb[i].thread);
	}
}

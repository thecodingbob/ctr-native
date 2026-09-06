#include <common.h>

void CS_BoxScene_InstanceSplitLines(void)
{
	s16 split = D233.VertSplitLine;
	struct Thread *t = sdata->gGT->threadBuckets[GHOST].thread;

	while (t != NULL)
	{
		t->inst->vertSplit = split;
		t = t->siblingThread;
	}
}

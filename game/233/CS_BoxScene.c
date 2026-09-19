#include <common.h>

void CS_BoxScene_InstanceSplitLines(void)
{
	s16 split;
	struct Thread *t = GAME_TRACKER->threadBuckets[GHOST].thread;

	if (t == NULL)
		return;
	split = CS_VERT_SPLIT;
	do
	{
		t->inst->vertSplit = split;
		t = t->siblingThread;
	} while (t != NULL);
}

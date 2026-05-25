#include <common.h>

void AH_SaveObj_ThDestroy(struct Thread *t)
{
	struct SaveObj *save;
	save = t->object;

	if (save->inst != NULL)
	{
		INSTANCE_Death(save->inst);
		save->inst = NULL;
	}
	return;
}

#include <common.h>

void PROC_DestroyInstance(struct Thread *t)
{
	INSTANCE_Death(t->inst);
}

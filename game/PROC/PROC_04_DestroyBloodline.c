#include <common.h>

// was in proc_03 for bytebudget reasons
void PROC_DestroyBloodline(struct Thread *t)
{
	while (t != 0)
	{
		// recursively find all children
		if (t->childThread != 0)
			PROC_DestroyBloodline(t->childThread);

		PROC_DestroySelf(t);
		t = t->siblingThread;
	}
}

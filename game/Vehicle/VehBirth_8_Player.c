#include <common.h>

struct Driver *VehBirth_Player(int index)
{
	struct Thread *t = PROC_BirthWithObject(0x62c0100, 0, 0, 0);

	struct Driver *d = t->object;
	memset(d, 0, 0x62c);

	VehBirth_NonGhost(t, index);

	d->funcPtrs[0] = VehPhysProc_Driving_Init;

	d->BattleHUD.teamID = sdata->gGT->battleSetup.teamOfEachPlayer[index];

	return d;
}

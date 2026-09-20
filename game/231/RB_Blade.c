#include <common.h>

void RB_Blade_ThTick(struct Thread *t)
{
	struct Blade *bladeObj;
	struct Instance *bladeInst;
	SVec3 rot;

	bladeObj = (struct Blade *)t->object;
	bladeInst = t->inst;

	for (;;)
	{
		rot.x = bladeInst->instDef->rot.x;
		rot.y = bladeInst->instDef->rot.y + 0x400;
		rot.z = bladeObj->angle;

		bladeObj->angle += 0x100;

		ConvertRotToMatrix(&bladeInst->matrix, &rot);

		bladeInst->scale.z = 0x1000;
		bladeInst->scale.y = 0x1000;
		bladeInst->scale.x = 0x1000;

		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native dispatch calls each tick anew; retail resumes after the yield.
		return;
#endif
	}
}

void RB_Blade_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct Blade *bladeObj;

	// Hot Air Skyway has four blade groups across two blimps, three fins per group.
	if (inst->thread != NULL)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Blade), NONE, SMALL, STATIC),

	    RB_Blade_ThTick, // behavior
	    "blade",         // debug name
	    0                // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}

	bladeObj = t->object;
	t->inst = inst;
	bladeObj->angle = 0;
}

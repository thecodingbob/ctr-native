#include <common.h>


struct ParticleEmitter emSet_OrcaSplash[7] = {
    {
        .flags = 1,
        .initOffset = 0xC,
        .InitTypes = {.FuncInit =
                          {
                              .particle_funcPtr = 0,
                              .particle_colorFlags = 0x4A0,
                              .particle_lifespan = 0xF,
                              .particle_Type = 0,
                          }},
    },

    {
        .flags = 0x12,
        .initOffset = 0,
        .InitTypes = {.AxisInit =
                          {
                              .baseValue = {.startVal = 0, .velocity = -0x1400, .accel = 0},
                              .rngSeed = {.startVal = 0, .velocity = 0x2800, .accel = 0},
                          }},
    },

    {
        .flags = 0x12,
        .initOffset = 2,
        .InitTypes = {.AxisInit =
                          {
                              .baseValue = {.startVal = 0, .velocity = -0x1400, .accel = 0},
                              .rngSeed = {.startVal = 0, .velocity = 0x2800, .accel = 0},
                          }},
    },

    {
        .flags = 0x3E,
        .initOffset = 1,
        .InitTypes = {.AxisInit =
                          {
                              .baseValue = {.startVal = 0, .velocity = 0x4C00, .accel = -0xDAC},
                              .rngSeed = {.startVal = 0x1000, .velocity = 0x4000, .accel = -0xC8},
                          }},
    },

    {
        .flags = 0xB,
        .initOffset = 5,
        .InitTypes = {.AxisInit =
                          {
                              .baseValue = {.startVal = 0x3E8, .velocity = 0x28, .accel = 0},
                              .rngSeed = {.startVal = 0x64},
                          }},
    },

    {
        .flags = 3,
        .initOffset = 7,
        .InitTypes = {.AxisInit =
                          {
                              .baseValue = {.startVal = 0x8000, .velocity = -0x700, .accel = 0},
                          }},
    },

    {0},
};

static inline void RB_Orca_SpawnSplash(struct Instance *orcaInst)
{
	struct Particle *particle;
	s32 i;

	for (i = 0xF; i != 0; i--)
	{
		particle = Particle_Init(0, GAME_TRACKER->iconGroup[1], &emSet_OrcaSplash[0]);

		if (particle == 0)
		{
			continue;
		}

		particle->axis[0].startVal += ((u32)orcaInst->matrix.t[0] << 8) + ((u32)particle->axis[0].velocity << 4);
		particle->axis[1].startVal += ((u32)orcaInst->matrix.t[1] << 8) + ((u32)particle->axis[1].velocity << 1);
		particle->axis[2].startVal += ((u32)orcaInst->matrix.t[2] << 8) + ((u32)particle->axis[2].velocity << 4);
		particle->renderDepthLimit = 0x1000;
	}
}

void RB_Orca_ThTick(struct Thread *t)
{
	struct Orca *orcaObj;
	struct Instance *orcaInst;
	struct GameTracker *gGT;
	s32 numFrames;
	s16 pathFrame;
	s32 denominator;
	s32 signedFrame;
	s32 nextFrame;
	s32 direction;

	orcaObj = t->object;
	pathFrame = orcaObj->cooldown;
	orcaInst = t->inst;

	if (pathFrame != 0)
	{
		orcaObj->cooldown = pathFrame - 1;

		if (orcaObj->cooldown != 0)
		{
			return;
		}

		orcaInst->flags &= ~HIDE_MODEL;
		return;
	}

	numFrames = orcaObj->numFrames;
	pathFrame = orcaObj->animIndex;
	// Clamp travel separately from the animation's lead-in and splash frames.
	denominator = numFrames - 0x14;

	if (denominator < pathFrame)
	{
		pathFrame = denominator;
	}

	direction = orcaObj->direction;
	denominator = numFrames - 0x17;

	if (direction != 0)
	{
		pathFrame -= 3;
	}
	else
	{
		signedFrame = pathFrame;
		if ((numFrames - 0x1A) < signedFrame)
		{
			pathFrame = numFrames - 0x1A;
		}
	}

	if (pathFrame < 0)
	{
		pathFrame = 0;
	}

	orcaInst->matrix.t[0] = orcaObj->startPos.x - ((pathFrame * orcaObj->pathDelta.x) / denominator);
	orcaInst->matrix.t[1] = orcaObj->startPos.y - ((pathFrame * orcaObj->pathDelta.y) / denominator);
	orcaInst->matrix.t[2] = orcaObj->startPos.z - ((pathFrame * orcaObj->pathDelta.z) / denominator);

	nextFrame = orcaInst->animFrame + 1;

	if (nextFrame < orcaObj->numFrames)
	{
		gGT = GAME_TRACKER;

		if ((gGT->numPlyrCurrGame < 2) && ((nextFrame == 5) || (nextFrame == 0x31)))
		{
			RB_Orca_SpawnSplash(orcaInst);
		}

		orcaInst->animFrame = nextFrame;

		if (direction != 0)
		{
			orcaObj->animIndex++;
		}
		else
		{
			orcaObj->animIndex--;
		}

		return;
	}

	orcaInst->flags |= HIDE_MODEL;
	orcaObj->cooldown = 0x5A;
	orcaInst->animFrame = 0;
	orcaObj->direction = direction ^ 1;
	orcaObj->instDefRot.y += 0x800;
	ConvertRotToMatrix(&orcaInst->matrix, &orcaObj->instDefRot);
}

int RB_Orca_ThCollide(struct Thread *orcaThread, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	(void)orcaThread;
	(void)driverTh;
	(void)funcThCollide;

	return sps->Input1.modelID == DYNAMIC_PLAYER;
}

void RB_Orca_LInB(struct Instance *inst)
{
	struct Orca *orcaObj;
	struct Thread *t;
	void **pointers;
	s16 *metaArray;
	int orcaID;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Orca), NONE, SMALL, STATIC),

	    RB_Orca_ThTick, // behavior
	    "orca",         // debug name
	    0               // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}

	orcaObj = t->object;
	t->funcThCollide = (void *)RB_Orca_ThCollide;
	t->inst = inst;

	inst->scale.x = 0xC00;
	inst->scale.y = 0xC00;
	inst->scale.z = 0xC00;
	inst->flags |= DRAW_HUGE;

	orcaObj->animIndex = -10;
	orcaObj->direction = 1;
	orcaObj->instDefRot.x = inst->instDef->rot.x;
	orcaObj->instDefRot.y = inst->instDef->rot.y;
	orcaObj->instDefRot.z = inst->instDef->rot.z;

	orcaID = inst->name[strlen(inst->name) - 1] - '0';
	orcaObj->orcaID = orcaID;

	if (GAME_TRACKER->level1->numSpawnType2 != 0)
	{
		orcaObj->startPos = GAME_TRACKER->level1->ptrSpawnType2[orcaID + 4].coords.positions[0];
		orcaObj->endPos = GAME_TRACKER->level1->ptrSpawnType2[orcaObj->orcaID + 4].coords.positions[1];
	}

	orcaObj->pathDelta.x = orcaObj->startPos.x - orcaObj->endPos.x;
	orcaObj->pathDelta.y = orcaObj->startPos.y - orcaObj->endPos.y;
	orcaObj->pathDelta.z = orcaObj->startPos.z - orcaObj->endPos.z;

	orcaObj->numFrames = INSTANCE_GetNumAnimFrames(inst, 0);

	if (GAME_TRACKER->level1->ptrSpawnType1->count <= 0)
	{
		return;
	}

	pointers = ST1_GETPOINTERS(GAME_TRACKER->level1->ptrSpawnType1);
	metaArray = (s16 *)pointers[ST1_SPAWN];
	metaArray += orcaObj->orcaID;
	orcaObj->cooldown = *metaArray;

	if (orcaObj->cooldown != 0)
	{
		inst->flags |= HIDE_MODEL;
	}
}

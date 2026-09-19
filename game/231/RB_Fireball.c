#include "RB_Collision.h"


int RB_Fireball_ThCollide(struct Thread *fireballThread, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	(void)fireballThread;
	(void)driverTh;
	(void)funcThCollide;

	return sps->Input1.modelID == DYNAMIC_PLAYER;
}

struct ParticleEmitter emSet_Fireball[10] = {
    {
        .flags = 1,
        .initOffset = 0xC,
        .InitTypes = {.FuncInit =
                          {
                              .particle_funcPtr = 0,
                              .particle_colorFlags = 0x4A1,
                              .particle_lifespan = 4,
                              .particle_Type = 0,
                          }},
    },
    // Position and rotation.
    {.flags = 1, .initOffset = 0, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}}},
    {.flags = 1, .initOffset = 2, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}}},
    {.flags = 3, .initOffset = 1, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1, .velocity = 1}}}},
    {.flags = 9,
     .initOffset = 4,
     .InitTypes = {.AxisInit =
                       {
                           .baseValue = {.startVal = 1},
                           .rngSeed = {.startVal = 0x1000},
                       }}},
    // Shrinking scale and fading RGB channels.
    {.flags = 3, .initOffset = 5, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0xA00, .velocity = -0xB0}}}},
    {.flags = 0xB,
     .initOffset = 7,
     .InitTypes = {.AxisInit =
                       {
                           .baseValue = {.startVal = 0xFF00, .velocity = 0xC000},
                           .rngSeed = {.startVal = 0x5F00},
                       }}},
    {.flags = 3, .initOffset = 8, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x8000, .velocity = 0xE000}}}},
    {.flags = 3, .initOffset = 9, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x4000, .velocity = 0xF000}}}},
    {0},
};

void RB_Fireball_ThTick(struct Thread *t)
{
	struct Instance *fireInst;
	struct Fireball *fireObj;
	struct Particle *particle;
	s32 particleVelY;
	s16 oldVelY = 0;

	fireObj = t->object;
	fireInst = t->inst;

	if (fireObj->cooldown != 0)
	{
		fireObj->cooldown -= GAME_TRACKER->elapsedTimeMS;

		if (fireObj->cooldown < 0)
		{
			fireObj->cooldown = 0;
		}

		return;
	}

	fireInst->flags |= HIDE_MODEL;

	// if fireball isn't below the lava,
	// handle all particle spawning
	if (fireInst->matrix.t[1] >= fireInst->instDef->pos.y - 0x440)
	{
		// move based on velocity
		oldVelY = fireObj->velY;
		fireInst->matrix.t[1] += (fireObj->velY * GAME_TRACKER->elapsedTimeMS) >> 5;

		// reduce velocity (gravity)
		fireObj->velY -= ((GAME_TRACKER->elapsedTimeMS * 10) >> 5);

		// terminal velocity
		if (fireObj->velY < -200)
		{
			fireObj->velY = -200;
		}

		// fire particles
		particle = Particle_Init(0, GAME_TRACKER->iconGroup[0xA], &emSet_Fireball[0]);

		if (particle != 0)
		{
			// adjust positions,
			// dont bitshift, must multiply, or negatives break
			particle->axis[0].startVal += fireInst->matrix.t[0] * 0x100;
			particle->axis[1].startVal += fireInst->matrix.t[1] * 0x100;
			particle->axis[2].startVal += fireInst->matrix.t[2] * 0x100;

			particle->renderDepthLimit = 0x1e00;

			// Launch particles against the fireball's current vertical motion.
			particleVelY = -fireObj->velY * 0x180;

			// range check
			if (particleVelY < -0x7fff)
			{
				particleVelY = -0x7fff;
			}
			if (particleVelY > 0x7fff)
			{
				particleVelY = 0x7fff;
			}
			particle->axis[1].velocity = particleVelY;
		}

		RB_CheckHazardCollisions(fireInst, t, 4, 0x10000, 0);
	}

	// === rest of movement behavior ===

	fireObj->cycleTimer -= GAME_TRACKER->elapsedTimeMS;

	if ((oldVelY >= 0) && (fireObj->velY < 0))
	{
		fireObj->direction = 1;
	}

	// if animation is not over
	if ((fireInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(fireInst, 0))
	{
		// increment frame
		fireInst->animFrame = fireInst->animFrame + 1;
	}

	// if animation ended
	else
	{
		// reset
		fireInst->animFrame = 0;
	}

	// if cycle is over
	if (fireObj->cycleTimer < 1)
	{
		// == first frame of fireball rising ==

		// reset position under lava
		fireInst->matrix.t[1] = fireInst->instDef->pos.y - 0x440;

		// upward velocity
		fireObj->velY = 200;
		// reset animation
		fireInst->animFrame = 0;
		fireObj->direction = 0;
		fireObj->cycleTimer = 0xb40;

		// fwooooossssssssshhhh
		PlaySound3D(0x81, fireInst);
	}
}

void RB_Fireball_LInB(struct Instance *inst)
{
	struct Fireball *fireObj;
	struct Thread *t;
	u32 fireballID;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Fireball), NONE, SMALL, STATIC),

	    RB_Fireball_ThTick, // behavior
	    "fireball",         // debug name
	    0                   // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	fireObj = t->object;
	t->inst = inst;
	t->funcThCollide = (void *)RB_Fireball_ThCollide;

	inst->scale.x = 0x4000;
	inst->scale.y = 0x4000;
	inst->scale.z = 0x4000;

	inst->animIndex = 0;

	fireObj->fireballID = inst->name[strlen(inst->name) - 1] - '0';
	fireballID = (u16)fireObj->fireballID;
	fireObj->rot_unused.x = 0;
	fireObj->velY = 96;
	fireObj->direction = 0;
	fireObj->cycleTimer = 0;

	if ((fireballID & 1) != 0)
	{
		// 1.44s, this is a ms-based timer, not a frame-based
		// counter, so t->cooldownFrameCount is not allowed
		fireObj->cooldown = 1440;
	}
	else
	{
		fireObj->cooldown = 0;
	}
}

#include <common.h>

// budget: 2584

enum PlantAnim
{
	PlantAnim_Rest,
	PlantAnim_TransitionRestHungry,
	PlantAnim_Hungry,
	PlantAnim_GrabDriver,
	PlantAnim_GrabMine, // unused
	PlantAnim_StartEat,
	PlantAnim_Chew,
	PlantAnim_Spit
};

extern struct BoundingBox plantBounds;

extern struct ParticleEmitter emSet_PlantTires[8];

void RB_Plant_ThTick_Eat(struct Thread *t)
{
	int i;
	struct Particle *particle;
	struct Instance *plantInst;
	struct Plant *plantObj;

	plantObj = t->object;
	plantInst = t->inst;

	do
	{
		if (plantInst->animIndex == PlantAnim_StartEat)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_StartEat))
			{
				plantInst->animFrame = plantInst->animFrame + 1;
			}

			else
			{
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_Chew;

				goto PlayChewSound;
			}
		}

		else if (plantInst->animIndex == PlantAnim_Chew)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_Chew))
			{
				plantInst->animFrame = plantInst->animFrame + 1;

				if (plantInst->animFrame == 0xf)
				{
				PlayChewSound:
					if (plantObj->boolEatingPlayer != 0)
					{
						OtherFX_Play(0x6e, 0);
					}
				}
			}

			else
			{
				plantInst->animFrame = 0;

				// One chew cycle precedes the spit animation.
				plantObj->cycleCount++;
				if (plantObj->cycleCount == 1)
				{
					plantObj->cycleCount = 0;

					plantInst->animFrame = 0;
					plantInst->animIndex = PlantAnim_Spit;
				}
			}
		}

		else if (plantInst->animIndex == PlantAnim_Spit)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_Spit))
			{
				plantInst->animFrame = plantInst->animFrame + 1;

				if (plantInst->animFrame == 0x19)
				{
					if (plantObj->boolEatingPlayer != 0)
					{
						OtherFX_Play(0x6f, 0);
					}

					for (i = 0; i < 4; i++)
					{
						particle = Particle_Init(0, GAME_TRACKER->iconGroup[0], &emSet_PlantTires[0]);

						if (particle == 0)
						{
							continue;
						}

						particle->axis[0].startVal += (plantInst->matrix.t[0] + (plantInst->matrix.m[0][2] * 9 >> 7)) * 0x100;

						particle->axis[1].startVal += (plantInst->matrix.t[1] + 0x20) * 0x100;

						particle->axis[2].startVal += (plantInst->matrix.t[2] + (plantInst->matrix.m[2][2] * 9 >> 7)) * 0x100;

						// Keep vertical velocity; give each horizontal axis its own launch variation.
						particle->axis[0].velocity += ((((MixRNG_Scramble() % 10) + 0x10) * plantInst->matrix.m[0][2]) >> 0xC) * 0x100;
						particle->axis[2].velocity += ((((MixRNG_Scramble() % 10) + 0x10) * plantInst->matrix.m[2][2]) >> 0xC) * 0x100;
						particle->funcPtr = Particle_FuncPtr_SpitTire;
						particle->owner.plantInst = plantInst;
					}
				}
			}

			else
			{
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_Rest;

				// Clear the player-only sound state before the next feeding cycle.
				plantObj->boolEatingPlayer = 0;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Rest);
			}
		}
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Plant_ThTick_Grab(struct Thread *t)
{
	struct Instance *plantInst;
	struct
	{
		struct HitboxDesc query;
		struct ThreadCollisionArgs collision;
	} hitbox;
	struct Instance *hitInst;

	plantInst = t->inst;
	hitbox.query.inst = plantInst;
	hitbox.query.thread = t;
	hitbox.query.bbox = plantBounds;
	hitbox.collision.other = t;
	hitbox.collision.sps = NULL;

	do
	{
		if (plantInst->animIndex == PlantAnim_GrabDriver)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_GrabDriver))
			{
				plantInst->animFrame++;
				hitbox.query.bucket = GAME_TRACKER->threadBuckets[MINE].thread;
				hitInst = LinkedCollide_Hitbox_Desc(&hitbox.query);
				if (hitInst != NULL)
				{
					hitbox.collision.self = hitInst->thread;
					hitbox.collision.funcThCollide = hitInst->thread->funcThCollide;
					RB_Hazard_ThCollide_Generic_Alt(&hitbox.collision);
				}
			}
			else
			{
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_StartEat;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Eat);
			}
		}
		else if (plantInst->animIndex == PlantAnim_GrabMine)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_GrabMine))
			{
				plantInst->animFrame++;
			}
			else
			{
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_Rest;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Rest);
			}
		}
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Plant_ThTick_Transition_HungryToRest(struct Thread *t)
{
	struct Instance *plantInst = t->inst;

	do
	{
		// Play the rest-to-hungry animation backwards.
		if ((plantInst->animFrame - 1) > 0)
		{
			plantInst->animFrame--;
		}
		else
		{
			plantInst->animFrame = 0;
			plantInst->animIndex = PlantAnim_Rest;
			ThTick_SetAndExec(t, RB_Plant_ThTick_Rest);
		}
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Plant_ThTick_Hungry(struct Thread *t)
{
	struct Instance *plantInst;
	struct Plant *plantObj;
	struct HitboxDesc hitbox;
	struct Instance *hitInst;
	struct Driver *hitDriver;

	plantObj = t->object;
	plantInst = t->inst;
	hitbox.inst = plantInst;
	hitbox.thread = t;
	hitbox.bbox = plantBounds;

	do
	{
		if (plantInst->animIndex != PlantAnim_Hungry)
			goto yield;

		if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_Hungry))
		{
			plantInst->animFrame++;
		}
		else
		{
			plantInst->animFrame = 0;
			plantObj->cycleCount++;
			if (plantObj->cycleCount == 4)
			{
				plantInst->animFrame = INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_TransitionRestHungry);
				plantInst->animIndex = PlantAnim_TransitionRestHungry;
				plantObj->cycleCount = 0;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Transition_HungryToRest);
			}
		}

		hitbox.bucket = GAME_TRACKER->threadBuckets[PLAYER].thread;
		hitInst = LinkedCollide_Hitbox_Desc(&hitbox);
		if (hitInst != NULL)
		{
			hitDriver = hitInst->thread->object;
			if (RB_Hazard_HurtDriver(hitDriver, 5, NULL, 0) != 0)
			{
				OtherFX_Play(0x6d, 0);
				hitDriver->plantEatingMe = t;
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_GrabDriver;
				plantObj->cycleCount = 0;
				plantObj->boolEatingPlayer = 1;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Grab);
			}
		}
		else if ((GAME_TRACKER->gameMode1 & ADVENTURE_BOSS) == 0)
		{
			hitbox.bucket = GAME_TRACKER->threadBuckets[ROBOT].thread;
			hitInst = LinkedCollide_Hitbox_Desc(&hitbox);
			if (hitInst != NULL)
			{
				RB_Hazard_HurtDriver(hitInst->thread->object, 5, NULL, 0);
				// NOTE(aalhendi): Damage can change the driver object; resolve it again before attaching.
				((struct Driver *)hitInst->thread->object)->plantEatingMe = t;
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_GrabDriver;
				plantObj->cycleCount = 0;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Grab);
			}
		}
	yield:
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Plant_ThTick_Rest(struct Thread *t)
{
	struct Instance *plantInst;
	struct Plant *plantObj;

	plantObj = t->object;
	plantInst = t->inst;

	do
	{
		if (plantObj->cooldown != 0)
		{
			plantObj->cooldown--;
			goto yield;
		}

		if (plantInst->animIndex == PlantAnim_Rest)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_Rest))
			{
				plantInst->animFrame = plantInst->animFrame + 1;
			}
			else
			{
				plantInst->animFrame = 0;

				// After 3 cycles, transition to hungry
				plantObj->cycleCount++;
				if (plantObj->cycleCount == 3)
				{
					plantInst->animIndex = PlantAnim_TransitionRestHungry;
					plantObj->cycleCount = 0;
				}
			}
		}

		else if (plantInst->animIndex == PlantAnim_TransitionRestHungry)
		{
			if ((plantInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(plantInst, PlantAnim_TransitionRestHungry))
			{
				plantInst->animFrame = plantInst->animFrame + 1;
			}
			else
			{
				plantInst->animFrame = 0;
				plantInst->animIndex = PlantAnim_Hungry;
				ThTick_SetAndExec(t, RB_Plant_ThTick_Hungry);
			}
		}
	yield:
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Plant_LInB(struct Instance *inst)
{
	struct Plant *plantObj;
	s16 *metaArray;
	void **pointers;
	s32 plantID;
	struct Thread *t;

	if (inst->thread != NULL)
	{
		return;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Plant), NONE, SMALL, STATIC), RB_Plant_ThTick_Rest, "plant", NULL);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}

	plantObj = t->object;
	t->inst = inst;

	inst->scale.x = 0x2800;
	inst->scale.y = 0x2800;
	inst->scale.z = 0x2800;
	plantObj->cycleCount = 0;
	plantObj->boolEatingPlayer = 0;
	inst->animFrame = 0;
	inst->animIndex = PlantAnim_Rest;

	plantBounds.max.x = 0x40;
	plantBounds.min.x = -0x40;
	plantBounds.max.y = 0x80;
	plantBounds.min.y = -0x40;
	plantBounds.max.z = 0x1e0;
	plantBounds.min.z = 0;

	if (GAME_TRACKER->level1->ptrSpawnType1->count > 0)
	{
		// Level metadata supplies each plant's cycle delay and track side.
		plantID = inst->name[strlen(inst->name) - 1] - '0';
		pointers = ST1_GETPOINTERS(GAME_TRACKER->level1->ptrSpawnType1);
		metaArray = (s16 *)pointers[ST1_SPAWN];
		metaArray = (s16 *)((u32)metaArray + plantID * 4);

		plantObj->cooldown = metaArray[0];
		plantObj->side = metaArray[1];
	}
}

struct ParticleEmitter emSet_PlantTires[8] = {{.flags = 1,

                                               // invalid axis, assume FuncInit
                                               .initOffset = 0xC,
                                               .InitTypes =
                                                   {

                                                       .FuncInit =
                                                           {
                                                               .particle_funcPtr = 0,
                                                               .particle_colorFlags = 0x121,
                                                               .particle_lifespan = 0x50,
                                                               .particle_Type = 0,
                                                           }

                                                       // last 0x10 bytes are blank
                                                   }},

                                              {.flags = 0x13,

                                               // posX
                                               .initOffset = 0,
                                               .InitTypes =
                                                   {

                                                       .AxisInit = {.baseValue =
                                                                        {
                                                                            .startVal = 1,
                                                                            .velocity = -0x320,
                                                                            .accel = 0,
                                                                        },

                                                                    .rngSeed =
                                                                        {
                                                                            .startVal = 0,
                                                                            .velocity = 0x640,
                                                                            .accel = 0,
                                                                        }}

                                                       // last 0x10 are blank
                                                   }},

                                              {.flags = 0x13,

                                               // posZ
                                               .initOffset = 2,
                                               .InitTypes =
                                                   {

                                                       .AxisInit = {.baseValue =
                                                                        {
                                                                            .startVal = 1,
                                                                            .velocity = -0x320,
                                                                            .accel = 0,
                                                                        },

                                                                    .rngSeed =
                                                                        {
                                                                            .startVal = 0,
                                                                            .velocity = 0x640,
                                                                            .accel = 0,
                                                                        }}

                                                       // last 0x10 are blank
                                                   }},

                                              {.flags = 0x17,

                                               // posY
                                               .initOffset = 1,
                                               .InitTypes =
                                                   {

                                                       .AxisInit = {.baseValue =
                                                                        {
                                                                            .startVal = 1,
                                                                            .velocity = -0x640,
                                                                            .accel = -0x320,
                                                                        },

                                                                    .rngSeed =
                                                                        {
                                                                            .startVal = 0,
                                                                            .velocity = 0x320,
                                                                            .accel = 0,
                                                                        }}

                                                       // last 0x10 are blank
                                                   }},

                                              {
                                                  .flags = 1,

                                                  // Scale
                                                  .initOffset = 5,
                                                  .InitTypes = {.AxisInit =
                                                                    {.baseValue =
                                                                         {

                                                                             // 100% scale
                                                                             .startVal = 0x1000}}},

                                                  // all the rest is untouched
                                              },

                                              {.flags = 0x1A,

                                               // RotX
                                               .initOffset = 4,
                                               .InitTypes =
                                                   {

                                                       .AxisInit =
                                                           {
                                                               .baseValue =
                                                                   {
                                                                       .startVal = 0,
                                                                       .velocity = 0xC0,
                                                                       .accel = 0,
                                                                   },

                                                               .rngSeed =
                                                                   {
                                                                       .startVal = 0x400,
                                                                       .velocity = 0x40,
                                                                       .accel = 0,
                                                                   }}}},

                                              {.flags = 0xA,

                                               // only for SpitTire
                                               .initOffset = 0xA,
                                               .InitTypes =
                                                   {

                                                       .AxisInit =
                                                           {
                                                               .baseValue =
                                                                   {
                                                                       .startVal = 0,
                                                                       .velocity = 0x100,
                                                                       .accel = 0,
                                                                   },

                                                               .rngSeed =
                                                                   {
                                                                       .startVal = 0xE00,
                                                                       .velocity = 0,
                                                                       .accel = 0,
                                                                   }}}},

                                              // null terminator
                                              {0}};

struct BoundingBox plantBounds = {0};

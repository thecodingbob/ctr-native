#include <common.h>

extern struct BoundingBox fjBounds;

struct FlameJetLight
{
	// NOTE(aalhendi): A real vector subobject keeps the padded copy alias-safe on native.
	SVec3 direction;
	s16 padding;
};

const struct FlameJetLight fjLightDir = {{0x8B8, 0xD6A, 0}, 0};

struct ParticleEmitter emSet_fjHeat[0xb] = {[0] =
                                                {
                                                    .flags = 1,

                                                    // invalid axis, assume FuncInit
                                                    .initOffset = 0xC,
.InitTypes = {

                                                    .FuncInit =
                                                        {
                                                            .particle_funcPtr = 0,
                                                            .particle_colorFlags = 0xA1,
                                                            .particle_lifespan = 6,
                                                            .particle_Type = 1,
                                                        }

                                                    // last 0x10 bytes are blank
                                                }},

                                            [1] =
                                                {
                                                    .flags = 0x1b,

                                                    // posX
                                                    .initOffset = 0,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = -0x40,
                                                    .velocity = -0x320},
.rngSeed = {

                                                    .startVal = 0x80,
                                                    .velocity = 0x640}}},
                                                },

                                            [2] =
                                                {
                                                    .flags = 0x1b,

                                                    // posZ
                                                    .initOffset = 2,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = -0x40,
                                                    .velocity = -0x320},
.rngSeed = {

                                                    .startVal = 0x80,
                                                    .velocity = 0x640}}},
                                                },

                                            [3] =
                                                {
                                                    .flags = 5,

                                                    // posY
                                                    .initOffset = 1,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = -0x40,
                                                    .accel = 1}}},
                                                },

                                            [4] =
                                                {
                                                    .flags = 0xb,

                                                    .initOffset = 3,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 0x1200,
                                                    .velocity = 1},
.rngSeed = {

                                                    .startVal = 0x1000}}},
                                                },

                                            [5] =
                                                {
                                                    .flags = 3,

                                                    .initOffset = 4,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 1,
                                                    .velocity = 1}}},
                                                },

                                            [6] =
                                                {
                                                    .flags = 3,

                                                    .initOffset = 5,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 1,
                                                    .velocity = 1}}},
                                                },

                                            [7] =
                                                {
                                                    .flags = 1,

                                                    .initOffset = 7,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 0x8000}}},
                                                },

                                            [8] =
                                                {
                                                    .flags = 1,

                                                    .initOffset = 8,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 0x8000}}},
                                                },

                                            [9] =
                                                {
                                                    .flags = 1,

                                                    .initOffset = 9,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 0x8000}}},
                                                },

                                            // null terminator
                                            {0}};

struct ParticleEmitter emSet_fjFire[0x8] = {[0] =
                                                {
                                                    .flags = 1,

                                                    // invalid axis, assume FuncInit
                                                    .initOffset = 0xC,
.InitTypes = {

                                                    .FuncInit =
                                                        {
                                                            .particle_funcPtr = 0,
                                                            .particle_colorFlags = 0xA1,
                                                            .particle_lifespan = 7,
                                                            .particle_Type = 0,
                                                        }

                                                    // last 0x10 bytes are blank
                                                }},

                                            [1] =
                                                {
                                                    .flags = 0x1b,

                                                    // posX
                                                    .initOffset = 0,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = -0x40,
                                                    .velocity = -0x320},
.rngSeed = {

                                                    .startVal = 0x80,
                                                    .velocity = 0x640}}},
                                                },

                                            [2] =
                                                {
                                                    .flags = 0x1b,

                                                    // posZ
                                                    .initOffset = 2,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = -0x40,
                                                    .velocity = -0x320},
.rngSeed = {

                                                    .startVal = 0x80,
                                                    .velocity = 0x640}}},
                                                },

                                            [3] =
                                                {
                                                    .flags = 5,

                                                    // posY
                                                    .initOffset = 1,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = -0x40,
                                                    .accel = 1}}},
                                                },

                                            [4] =
                                                {
                                                    .flags = 0x1a,

                                                    .initOffset = 4,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .velocity = 0x10},
.rngSeed = {

                                                    .startVal = 0x800,
                                                    .velocity = 0x10}}},
                                                },

                                            [5] =
                                                {
                                                    .flags = 0xb,

                                                    .initOffset = 5,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 0x4e2,
                                                    .velocity = 0x100},
.rngSeed = {

                                                    .startVal = 0x80}}},
                                                },

                                            [6] =
                                                {
                                                    .flags = 0xd,

                                                    .initOffset = 7,
.InitTypes = {
.AxisInit = {
.baseValue = {

                                                    .startVal = 0x8000,
                                                    .accel = -0x800},
.rngSeed = {

                                                    .startVal = 0x5f00}}},
                                                },

                                            // null terminator
                                            {0}};

void RB_FlameJet_Particles(struct Instance *inst, struct FlameJet *fjObj)
{
	int result;
	struct Particle *particle1;
	struct Particle *particle2;

	particle1 = Particle_Init(0, GAME_TRACKER->iconGroup[0xA], &emSet_fjFire[0]);

	// fire particle
	if (particle1 != 0)
	{
		particle1->axis[0].startVal += (inst->matrix.t[0]) * 0x100;
		particle1->axis[1].startVal += (inst->matrix.t[1] + 0x32) * 0x100;
		particle1->axis[2].startVal += (inst->matrix.t[2]) * 0x100;

		particle1->axis[0].velocity = (s16)fjObj->dirX;
		particle1->axis[1].velocity = 0;
		particle1->axis[2].velocity = (s16)fjObj->dirZ;

		result = RngDeadCoed(&GAME_TRACKER->deadcoed_struct);
		result = MATH_Sin((GAME_TRACKER->timer * 0x100 + (result >> 0x18)) & 0xfff);
		particle1->axis[1].accel = result >> 4;

		particle1->renderDepthLimit = 0x1e00;
		particle1->otIndexOffset = inst->depthBiasNormal - 1;

		if ((GAME_TRACKER->timer & 1) != 0)
		{
			particle1->axis[4].startVal = -particle1->axis[4].startVal;
			particle1->axis[4].velocity = -particle1->axis[4].velocity;
		}
	}

	// heat particle is 1P only
	if (GAME_TRACKER->numPlyrCurrGame > 1)
	{
		return;
	}

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Retail would read PS1 null-space if the fire particle allocation failed.
	if (particle1 == NULL)
	{
		return;
	}
#endif

	particle2 = Particle_Init(0, (struct IconGroup *)GAME_TRACKER->ptrSparkle, &emSet_fjHeat[0]);

	// heat particle
	if (particle2 != 0)
	{
		particle2->axis[0].startVal += particle1->axis[0].startVal;
		particle2->axis[1].startVal += particle1->axis[1].startVal + 0x1000;
		particle2->axis[2].startVal += particle1->axis[2].startVal;

		particle2->axis[0].velocity = (s16)fjObj->dirX;
		particle2->axis[1].velocity = 0;
		particle2->axis[2].velocity = (s16)fjObj->dirZ;

		particle2->axis[1].accel = particle1->axis[1].accel;

		{
			s32 size = particle2->axis[3].startVal;
			particle2->axis[4].startVal = size - 0x400;
			particle2->axis[5].startVal = size - 0x600;
		}
		particle2->renderDepthLimit = 0x1e00;

		particle2->axis[3].velocity = (0x4a00 - particle2->axis[3].startVal) / 7;
		particle2->axis[4].velocity = (0x4600 - particle2->axis[4].startVal) / 7;
		particle2->axis[5].velocity = (0x4400 - particle2->axis[5].startVal) / 7;

		particle2->otIndexOffset = inst->depthBiasNormal;
	}
}

void RB_FlameJet_ThTick(struct Thread *t)
{
	struct FlameJetLight light = fjLightDir;
	struct
	{
		struct HitboxDesc query;
		struct ThreadCollisionArgs collision;
	} collision;
	struct Instance *fjInst;
	struct FlameJet *fjObj;
	struct Instance *hitInst;

	fjObj = t->object;
	fjInst = t->inst;
	collision.query.inst = fjInst;
	collision.query.thread = t;
	collision.query.bbox = fjBounds;
	collision.collision.other = t;
	collision.collision.sps = NULL;

	while (fjObj->cooldown != 0)
	{
		fjObj->cooldown--;
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native callbacks yield by returning to the scheduler.
		return;
#endif
	}

	do
	{
		// Emit flame for the first 45 frames of each cycle.
		if (fjObj->cycleTimer < 0x2d)
		{
			PlaySound3D_Flags(&fjObj->soundIDCount, 0x68, fjInst);
			fjObj->unusedPhase += 0x100;
			RB_FlameJet_Particles(fjInst, fjObj);

			collision.query.bucket = GAME_TRACKER->threadBuckets[PLAYER].thread;
			hitInst = LinkedCollide_Hitbox_Desc(&collision.query);
			if (hitInst == NULL)
			{
				collision.query.bucket = GAME_TRACKER->threadBuckets[ROBOT].thread;
				hitInst = LinkedCollide_Hitbox_Desc(&collision.query);
			}
			if (hitInst != NULL)
			{
				RB_Hazard_HurtDriver(hitInst->thread->object, 4, NULL, 0);
			}
			else
			{
				collision.query.bucket = GAME_TRACKER->threadBuckets[MINE].thread;
				hitInst = LinkedCollide_Hitbox_Desc(&collision.query);
				if (hitInst != NULL)
				{
					collision.collision.self = hitInst->thread;
					collision.collision.funcThCollide = hitInst->thread->funcThCollide;
					RB_Hazard_ThCollide_Generic_Alt(&collision.collision);
				}
			}
		}
		else if (fjObj->cycleTimer == 0x2d)
		{
			if (fjObj->soundIDCount != 0)
			{
				OtherFX_RecycleMute(&fjObj->soundIDCount);
			}
		}
		else if (fjObj->cycleTimer > 0x69)
		{
			fjObj->cycleTimer = 0;
		}

		fjObj->cycleTimer++;
		Vector_SpecLightNoSpin3D(fjInst, &fjInst->instDef->rot, &light.direction);
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Retail resumes this loop after FastRET; native ticks return.
		return;
#endif
	} while (1);
}

void RB_FlameJet_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct FlameJet *fjObj;

	// color
	inst->colorRGBA = 0xdca6000;

	// yellow
	inst->flags |= (DRAW_TRANSPARENT | USE_SPECULAR_LIGHT);

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct FlameJet), NONE, SMALL, STATIC),

	    RB_FlameJet_ThTick, // behavior
	    "flamejet",         // debug name
	    0                   // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	fjObj = t->object;
	t->inst = inst;

	fjObj->cycleTimer = 0;
	fjObj->cooldown = 0;
	fjObj->dirX = -inst->matrix.m[0][2] * 0x4b >> 5;
	fjObj->dirZ = inst->matrix.m[2][2] * 0x4b >> 5;
	fjObj->soundIDCount = 0;

	fjBounds.max.x = 0x40;
	fjBounds.min.x = -0x40;
	fjBounds.max.y = 0x80;
	fjBounds.min.y = -0x40;
	fjBounds.max.z = 0x140;
	fjBounds.min.z = 0;

	if (GAME_TRACKER->level1->ptrSpawnType1->count > 0)
	{
		// NOTE(aalhendi): The name's final digit selects a packed halfword delay to stagger instances.
		s32 byteOffset = (inst->name[strlen(inst->name) - 1] - '0') * 2;
		void **pointers = ST1_GETPOINTERS(GAME_TRACKER->level1->ptrSpawnType1);
		u8 *delays = pointers[ST1_SPAWN];
		fjObj->cooldown = CTR_ReadU16AlignedLE(delays + byteOffset);
	}
}

struct BoundingBox fjBounds = {0};

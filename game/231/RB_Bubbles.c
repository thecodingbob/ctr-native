#include <common.h>

extern struct ParticleEmitter emSet_TubeBubbles[7];

void RB_Bubbles_RoosTubes(void)
{
	u32 timer;
	struct GameTracker *gGT;
	struct Level *level1;
	struct SpawnType2 *spawnType2;
	s32 numSpawnPosCoords;
	SVec3 *spawnPos;
	s32 numFreeParticles;
	struct Particle *p;
	struct Driver *d;
	s32 projectedX;
	s32 projectedZ;
	s32 posX;
	s32 posY;
	s32 posZ;

	// 1P mode Roo's Tubes only
	gGT = GAME_TRACKER;
	if (gGT->numPlyrCurrGame > 1)
	{
		return;
	}
	if (gGT->levelID != ROO_TUBES)
	{
		return;
	}

	level1 = gGT->level1;
	if (level1->numSpawnType2 < 2)
	{
		return;
	}

	// The first emitter is hidden by the level geometry; start at the second.
	spawnType2 = &level1->ptrSpawnType2[1];
	spawnPos = &spawnType2->coords.positions[1];
	if (spawnPos == NULL)
	{
		return;
	}
	numFreeParticles = gGT->JitPools.particle.free.count;
	numSpawnPosCoords = spawnType2->numCoords - 1;
	d = gGT->drivers[0];

	timer = gGT->timer;
	// NOTE(aalhendi): Capture the projected driver position once for this pass.
	// Unsigned subtraction preserves MIPS wrapping before the signed shifts.
	posX = d->posCurr.x;
	posZ = d->posCurr.z;
	projectedX = (u32)posX - (u32)d->posPrev.x;
	projectedZ = (u32)posZ - (u32)d->posPrev.z;
	projectedX >>= 4;
	projectedZ >>= 4;
	projectedX += posX >> 8;
	projectedZ += posZ >> 8;

	for (; (numSpawnPosCoords > 0) && (numFreeParticles >= 0x14); numSpawnPosCoords--, spawnPos++)
	{
		// each particle gets spawned once every 8 frames
		if (((timer + numSpawnPosCoords) & 7) != 0)
		{
			continue;
		}

		posX = spawnPos->x;
		posY = spawnPos->y;
		posZ = spawnPos->z;
		// skip emitters far from projected driver position
		if (abs(projectedX - posX) + abs(projectedZ - posZ) > 0x1680)
		{
			continue;
		}

		p = Particle_Init(0, GAME_TRACKER->iconGroup[7], &emSet_TubeBubbles[0]);

		if (p == 0)
		{
			return;
		}

		numFreeParticles--;

		p->renderDepthLimit = 0x7fff;
		p->otIndexOffset = 8;

		p->axis[0].startVal = (u32)p->axis[0].startVal + (u32)(posX * 0x100);
		p->axis[1].startVal = (u32)p->axis[1].startVal + (u32)(posY * 0x100);
		p->axis[2].startVal = (u32)p->axis[2].startVal + (u32)(posZ * 0x100);
	}
}

struct ParticleEmitter emSet_TubeBubbles[7] = {{.flags = 1,

                                                // invalid axis, assume FuncInit
                                                .initOffset = 0xC,
                                                .InitTypes =
                                                    {

                                                        .FuncInit =
                                                            {
                                                                .particle_funcPtr = 0,
                                                                .particle_colorFlags = 0x4A0,
                                                                .particle_lifespan = 0x32,
                                                                .particle_Type = 0,
                                                            }

                                                        // last 0x10 bytes are blank
                                                    }},

                                               {.flags = 0x1B,

                                                // posX
                                                .initOffset = 0,
                                                .InitTypes =
                                                    {

                                                        .AxisInit = {.baseValue =
                                                                         {
                                                                             .startVal = -0x200,
                                                                             .velocity = -0x200,
                                                                             .accel = 0,
                                                                         },

                                                                     .rngSeed =
                                                                         {
                                                                             .startVal = 0x400,
                                                                             .velocity = 0x400,
                                                                             .accel = 0,
                                                                         }}

                                                        // last 0x10 are blank
                                                    }},

                                               {.flags = 0x24,

                                                // posY
                                                .initOffset = 1,
                                                .InitTypes =
                                                    {

                                                        .AxisInit = {.baseValue =
                                                                         {
                                                                             .startVal = 0,
                                                                             .velocity = 0,
                                                                             .accel = 0x64,
                                                                         },

                                                                     .rngSeed =
                                                                         {
                                                                             .startVal = 0,
                                                                             .velocity = 0,
                                                                             .accel = 0xC8,
                                                                         }}

                                                        // last 0x10 are blank
                                                    }},

                                               {.flags = 0x1B,

                                                // posZ
                                                .initOffset = 2,
                                                .InitTypes =
                                                    {

                                                        .AxisInit = {.baseValue =
                                                                         {
                                                                             .startVal = -0x200,
                                                                             .velocity = -0x200,
                                                                             .accel = 0,
                                                                         },

                                                                     .rngSeed =
                                                                         {
                                                                             .startVal = 0x400,
                                                                             .velocity = 0x400,
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

                                                                              // 20% scale
                                                                              .startVal = 0x200}}},

                                                   // all the rest is untouched
                                               },

                                               {
                                                   .flags = 1,

                                                   // rotY
                                                   .initOffset = 4,
                                                   .InitTypes = {.AxisInit =
                                                                     {.baseValue =
                                                                          {

                                                                              .startVal = 1}}},

                                                   // all the rest is untouched
                                               },

                                               // null terminator
                                               {0}};

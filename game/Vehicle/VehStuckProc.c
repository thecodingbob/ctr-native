#include "VehCommon.h"

enum
{
	VEH_STUCK_MASK_BSP_PROBE_HEIGHT = 0x100,
	VEH_STUCK_RESPAWN_Y_OFFSET = 0x80,
	VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ = 0x2000,
	VEH_STUCK_MASK_HEAD_Y_OFFSET = 0x140,
	VEH_STUCK_REV_MASK_RELEASE_HEIGHT = 0x4000,
	VEH_STUCK_REV_MASK_GRAB_HEIGHT_TRIGGER = 0x1000,
	VEH_STUCK_REV_SMALL_BOOST_FIRE_LEVEL = 0x20,
	VEH_STUCK_REV_BIG_BOOST_FIRE_LEVEL = 0x80,
	VEH_STUCK_REV_START_BOOST_RESERVES = 960,
	VEH_STUCK_REV_MASK_DESCENT_STEP = 0x200,
	VEH_STUCK_REV_CAMERA_HEIGHT_OFFSET = 0x40,
	VEH_STUCK_REV_OVERREV_FX = 0xf,
	VEH_STUCK_REV_FILL_STEP_MAX = 5000,
	VEH_STUCK_REV_STEP_MIN = 0x100,
	VEH_STUCK_REV_OVERREV_TIMEOUT = 192,
	VEH_STUCK_REV_RELEASE_COOLDOWN = 0x100,
	VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL = 1000,
	VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED = 3000,
	VEH_STUCK_REV_EMPTY_COOLDOWN = 0xc0,
	VEH_STUCK_REV_TURBO_ROOM_SHIFT = 5,
	VEH_STUCK_REV_SQUISH_SHIFT = 6,
	VEH_STUCK_REV_SQUISH_LIMIT = 0x401,
	VEH_STUCK_REV_SQUISH_MAX = 0x400,
	VEH_STUCK_REV_MODEL_BASE_SCALE = 3276,
	VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR = 6,
	VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR = 10,
	VEH_STUCK_MASK_GRAB_WHISTLE_FX = 0x55,
	VEH_STUCK_MASK_GRAB_WHISTLE_TIME = 960,
	VEH_STUCK_MASK_GRAB_CRASH_TIME = 1296,
	VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_LIMIT = 0x3c1,
	VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_STEP = 800,
	VEH_STUCK_MASK_GRAB_SQUISH_INCREASE_STEP = 0x2d0,
	VEH_STUCK_MASK_GRAB_SQUISH_MAX = 8000,
	VEH_STUCK_MASK_GRAB_LIFT_SHIFT = 7,
	VEH_STUCK_MASK_GRAB_SCALE_FULL_TIME = 721,
	VEH_STUCK_MASK_GRAB_SCALE_START_TIME = VEH_STUCK_MASK_GRAB_WHISTLE_TIME,
	VEH_STUCK_MASK_GRAB_SCALE_RAMP_TIME = 0xf0,
	VEH_STUCK_MASK_GRAB_INITIAL_TIMER = 1440,
	VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD = 0x8000,
	VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX = 2,
	VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD = 3,
	VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET = 5,
	VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP = 7,
	VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME = 12,
	VEH_STUCK_PLANT_CAMERA_INIT_TIME = 2880,
	VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET = 250,
	VEH_STUCK_PLANT_CAMERA_FORWARD_OFFSET = 750,
	VEH_STUCK_PLANT_CAMERA_Y_OFFSET = 0xc0,
	VEH_STUCK_PLANT_CAMERA_PITCH_BASE = 0x800,
	VEH_TUMBLE_FORCED_JUMP_MS = 0x60,
	VEH_TUMBLE_JUMP_VELOCITY_TIMER_SHIFT = 1,
	VEH_TUMBLE_JUMP_VELOCITY_BASE = 6000,
	VEH_TUMBLE_STEERING_FRAME_SENTINEL = 10000,
	VEH_TUMBLE_ANGULAR_DAMP_SHIFT = 3,
	VEH_TUMBLE_TURN_WRAP_BIAS = 0x800,
	VEH_TUMBLE_TURN_MASK = 0xfff,
	VEH_TUMBLE_SPIN_RATE_SHIFT = 0xd,
	VEH_TUMBLE_ROT_W_INTERP_SHIFT = 5,
	VEH_TUMBLE_ANIM_FRAME_TIME_SHIFT = 5,
	VEH_TUMBLE_ANIM_REVERSE_OFFSET = 1,
	VEH_TUMBLE_WUMPA_PENALTY = 3,
	VEH_TUMBLE_BACKWARDS_RNG_MASK = 4,
	VEH_TUMBLE_RUMBLE_WEAK = 0x19,
	VEH_TUMBLE_RUMBLE_STRONG = 0x29,
	VEH_TUMBLE_RUMBLE_DURATION = 0x60,
};

typedef u32 VehStuckProcWord CTR_MAY_ALIAS;


static inline void VehStuckProc_MaskGrab_SearchBsp(struct Driver *d, struct ScratchpadStruct *sps)
{
	sps->Union.QuadBlockColl.pos.x = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);
	sps->Union.QuadBlockColl.pos.y = (s16)CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8);
	sps->Union.QuadBlockColl.pos.z = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);
	sps->Input1.pos.x = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);
	sps->Input1.pos.y = (s16)CTR_MipsSubLo(CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8), VEH_STUCK_MASK_BSP_PROBE_HEIGHT);
	sps->Input1.pos.z = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);

	sps->Union.QuadBlockColl.searchFlags = 0;
	if (GAME_TRACKER->numPlyrCurrGame < 3)
	{
		sps->Union.QuadBlockColl.searchFlags = COLL_SEARCH_HIGH_LOD;
	}

	sps->boolDidTouchQuadblock = 0;
	sps->numTrianglesTested = 0;
	sps->hitFraction = COLL_FRACTION_ONE;
	sps->collision.stepFlags = 0;

	{
		register u16 a0Value CTR_PSX_REGISTER("$4");
		register u16 a1Value CTR_PSX_REGISTER("$5");
		register u16 a2Value CTR_PSX_REGISTER("$6");
		register u16 a3Value CTR_PSX_REGISTER("$7");

		a1Value = (u16)sps->Union.QuadBlockColl.pos.x;
		if (sps->Input1.pos.x < (s16)a1Value)
		{
			a1Value = (u16)sps->Input1.pos.x;
		}

		a0Value = (u16)sps->Union.QuadBlockColl.pos.y;
		sps->bbox.min.x = (s16)a1Value;
		CTR_PSX_MEMORY_BARRIER();
		if ((s16)a0Value > sps->Input1.pos.y)
		{
			a0Value = (u16)sps->Input1.pos.y;
		}

		a1Value = (u16)sps->Union.QuadBlockColl.pos.z;
		sps->bbox.min.y = (s16)a0Value;
		CTR_PSX_MEMORY_BARRIER();
		if ((s16)a1Value > sps->Input1.pos.z)
		{
			a1Value = (u16)sps->Input1.pos.z;
		}

		a2Value = (u16)sps->Union.QuadBlockColl.pos.x;
		sps->bbox.min.z = (s16)a1Value;
		if ((s16)a2Value < sps->Input1.pos.x)
		{
			a2Value = (u16)sps->Input1.pos.x;
		}

		a0Value = (u16)sps->Union.QuadBlockColl.pos.y;
		sps->bbox.max.x = (s16)a2Value;
		if ((s16)a0Value < sps->Input1.pos.y)
		{
			a0Value = (u16)sps->Input1.pos.y;
		}

		a3Value = (u16)sps->Union.QuadBlockColl.pos.z;
		sps->bbox.max.y = (s16)a0Value;
		if ((s16)a3Value < sps->Input1.pos.z)
		{
			a3Value = (u16)sps->Input1.pos.z;
		}
		sps->bbox.max.z = (s16)a3Value;
	}

	{
		register u16 hitX CTR_PSX_REGISTER("$2");
		register u16 hitY CTR_PSX_REGISTER("$3");
		register u16 hitZ CTR_PSX_REGISTER("$4");
		register struct mesh_info *mesh CTR_PSX_REGISTER("$6");
		register struct BoundingBox *bbox CTR_PSX_REGISTER("$5");
		register struct BSP *root CTR_PSX_REGISTER("$4");
		register struct ScratchpadStruct *callSps CTR_PSX_REGISTER("$7");

		hitX = (u16)sps->Input1.pos.x;
		hitY = (u16)sps->Input1.pos.y;
		hitZ = (u16)sps->Input1.pos.z;
		mesh = sps->ptr_mesh_info;
		CTR_PSX_ADD_POINTER_IMMEDIATE(bbox, sps, offsetof(struct ScratchpadStruct, bbox), &sps->bbox);
		CTR_PSX_OBSERVE_VALUE(hitX);
		CTR_PSX_OBSERVE_VALUE(hitY);
		CTR_PSX_OBSERVE_VALUE(hitZ);
		CTR_PSX_OBSERVE_VALUE(mesh);
		CTR_PSX_OBSERVE_VALUE(bbox);
		callSps = sps;
		CTR_PSX_OBSERVE_VALUE(callSps);
		sps->Union.QuadBlockColl.hitPos.x = (s16)hitX;
		sps->Union.QuadBlockColl.hitPos.y = (s16)hitY;
		sps->Union.QuadBlockColl.hitPos.z = (s16)hitZ;
		root = mesh->bspRoot;
		CTR_PSX_OBSERVE_VALUE(root);
		COLL_SearchBSP_CallbackPARAM(root, bbox, COLL_FIXED_BSPLEAF_TestQuadblocks, callSps);
	}
}

void VehStuckProc_MaskGrab_FindDestPos(struct Driver *d, struct QuadBlock *quad)
{
	struct Driver *driver CTR_PSX_REGISTER("$17") = d;
	struct ScratchpadStruct *sps CTR_PSX_REGISTER("$16");
	struct CheckpointNode *nextRespawn CTR_PSX_REGISTER("$18");
	register struct CheckpointNode *respawn CTR_PSX_REGISTER("$5");
	register struct Thread *playerThread CTR_PSX_REGISTER("$4");
	register struct GameTracker *gGT CTR_PSX_REGISTER("$4");
	register struct QuadBlock *inputQuad CTR_PSX_REGISTER("$5") = quad;
	register struct QuadBlock *sourceQuad CTR_PSX_REGISTER("$6");
	register struct Level *initialLevel CTR_PSX_REGISTER("$3");
	register int restartCount CTR_PSX_REGISTER("$2");
	int searchDirection CTR_PSX_REGISTER("$19");

	gGT = GAME_TRACKER;
	CTR_PSX_KEEP_VALUE(gGT);
	CTR_PSX_KEEP_VALUE(driver);
	initialLevel = gGT->level1;
	restartCount = initialLevel->cnt_restart_points;
	CTR_PSX_OBSERVE_VALUE(restartCount);
	CTR_PSX_OBSERVE_VALUE(inputQuad);
	sourceQuad = inputQuad;
	if (restartCount < 1)
	{
		goto fallback;
	}
	if (initialLevel->ptr_restart_points == NULL)
	{
		goto fallback;
	}
	if (sourceQuad->checkpointIndex == 0xff)
	{
		goto fallback;
	}

	{
		sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
		searchDirection = 0;
		sps->Input1.hitRadius = driver->instSelf->thread->driverHitRadius;
		sps->Input1.hitRadiusSquared = driver->instSelf->thread->driverHitRadiusSquared;
		sps->Union.QuadBlockColl.hitRadius = driver->instSelf->thread->driverHitRadius;
		sps->Union.QuadBlockColl.hitRadiusSquared = driver->instSelf->thread->driverHitRadiusSquared;
		sps->ptr_mesh_info = gGT->level1->ptr_mesh_info;
		sps->Union.QuadBlockColl.quadFlagsIgnored = QUADBLOCK_FLAG_NO_CAMERA_RESPAWN_PROBE | QUADBLOCK_FLAG_NO_COLLISION_RESPONSE;
		sps->Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND;
		{
			register struct Level *level CTR_PSX_REGISTER("$2") = gGT->level1;
			register struct CheckpointNode *restartPoints CTR_PSX_REGISTER("$4");
			register u32 restartIndex CTR_PSX_REGISTER("$3") = sourceQuad->checkpointIndex;
			register size_t restartOffset CTR_PSX_REGISTER("$2");

			restartPoints = level->ptr_restart_points;
			restartOffset = CTR_MipsSll(CTR_MipsAddLo(CTR_MipsSll(restartIndex, 1), restartIndex), 2);
			CTR_PSX_KEEP_VALUE(level);
			CTR_PSX_KEEP_VALUE(restartPoints);
			CTR_PSX_KEEP_VALUE(restartIndex);
			CTR_PSX_KEEP_VALUE(restartOffset);
			driver->distanceDrivenBackwards = 0;
			CTR_PSX_MEMORY_BARRIER();
			respawn = (struct CheckpointNode *)((u8 *)restartPoints + restartOffset);
		}
		CTR_PSX_KEEP_VALUE(respawn);
		nextRespawn = respawn;

	searchRespawn:
	{
		CTR_PSX_KEEP_VALUE(respawn);
		if (searchDirection != 0)
		{
			register struct GameTracker *loopTracker CTR_PSX_REGISTER("$3") = GAME_TRACKER;
			register struct Level *loopLevel CTR_PSX_REGISTER("$3");
			register struct CheckpointNode *restartPoints CTR_PSX_REGISTER("$3");
			register u32 restartIndex CTR_PSX_REGISTER("$4") = respawn->nextIndex_backward;
			register size_t restartOffset CTR_PSX_REGISTER("$2");

			restartOffset = CTR_MipsSll(restartIndex, 1);
			loopLevel = loopTracker->level1;
			CTR_PSX_OBSERVE_VALUE(loopLevel);
			restartOffset = CTR_MipsAddLo(restartOffset, restartIndex);
			restartPoints = loopLevel->ptr_restart_points;
			CTR_PSX_OBSERVE_VALUE(restartPoints);
			restartOffset = CTR_MipsSll(restartOffset, 2);
			CTR_PSX_KEEP_VALUE(loopTracker);
			CTR_PSX_KEEP_VALUE(loopLevel);
			CTR_PSX_KEEP_VALUE(restartPoints);
			CTR_PSX_KEEP_VALUE(restartIndex);
			CTR_PSX_KEEP_VALUE(restartOffset);
			respawn = (struct CheckpointNode *)((u8 *)restartPoints + restartOffset);
		}
		else
		{
			register struct GameTracker *loopTracker CTR_PSX_REGISTER("$3") = GAME_TRACKER;
			register struct Level *loopLevel CTR_PSX_REGISTER("$3");
			register struct CheckpointNode *restartPoints CTR_PSX_REGISTER("$3");
			register u32 restartIndex CTR_PSX_REGISTER("$4") = respawn->nextIndex_forward;
			register size_t restartOffset CTR_PSX_REGISTER("$2");

			restartOffset = CTR_MipsSll(restartIndex, 1);
			loopLevel = loopTracker->level1;
			CTR_PSX_OBSERVE_VALUE(loopLevel);
			restartOffset = CTR_MipsAddLo(restartOffset, restartIndex);
			restartPoints = loopLevel->ptr_restart_points;
			CTR_PSX_OBSERVE_VALUE(restartPoints);
			restartOffset = CTR_MipsSll(restartOffset, 2);
			CTR_PSX_KEEP_VALUE(loopTracker);
			CTR_PSX_KEEP_VALUE(loopLevel);
			CTR_PSX_KEEP_VALUE(restartPoints);
			CTR_PSX_KEEP_VALUE(restartIndex);
			CTR_PSX_KEEP_VALUE(restartOffset);
			nextRespawn = (struct CheckpointNode *)((u8 *)restartPoints + restartOffset);
		}

		driver->posCurr.x = CTR_MipsSll(respawn->pos.x, FRACTIONAL_BITS_8);
		driver->posCurr.y = CTR_MipsSll(CTR_MipsAddLo(respawn->pos.y, VEH_STUCK_RESPAWN_Y_OFFSET), FRACTIONAL_BITS_8);
		driver->posCurr.z = CTR_MipsSll(respawn->pos.z, FRACTIONAL_BITS_8);

		driver->rotCurr.x = 0;
		driver->rotCurr.y = ratan2(CTR_MipsSubLo(nextRespawn->pos.x, respawn->pos.x), CTR_MipsSubLo(nextRespawn->pos.z, respawn->pos.z));
		driver->rotCurr.z = 0;

		VehStuckProc_MaskGrab_SearchBsp(driver, sps);
		if ((sps->boolDidTouchQuadblock == 0) || ((sps->collision.stepFlags & COLL_STEP_FLAG_KILL_PLANE) != 0))
		{
			respawn = nextRespawn;
			goto searchRespawn;
		}

		playerThread = GAME_TRACKER->threadBuckets[PLAYER].thread;
		while (playerThread != NULL)
		{
			register struct Driver *other CTR_PSX_REGISTER("$5") = playerThread->object;

			if (other != driver)
			{
				register int diffX CTR_PSX_REGISTER("$2") = driver->posCurr.x;
				register int otherX CTR_PSX_REGISTER("$3") = other->posCurr.x;
				int diffZ;

				CTR_PSX_OBSERVE_VALUE(diffX);
				CTR_PSX_OBSERVE_VALUE(otherX);
				CTR_PSX_SUBTRACT(diffX, diffX, otherX);
				if (diffX < 0)
				{
					diffX = CTR_MipsNegLo(diffX);
				}

				if (diffX < VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ)
				{
					break;
				}

				{
					register int driverZ CTR_PSX_REGISTER("$2") = driver->posCurr.z;
					register int otherZ CTR_PSX_REGISTER("$3") = other->posCurr.z;

					CTR_PSX_OBSERVE_VALUE(driverZ);
					CTR_PSX_OBSERVE_VALUE(otherZ);
					CTR_PSX_SUBTRACT(diffZ, driverZ, otherZ);
				}
				if (diffZ < 0)
				{
					diffZ = CTR_MipsNegLo(diffZ);
				}

				if (diffZ < VEH_STUCK_RESPAWN_PLAYER_CLEARANCE_XZ)
				{
					break;
				}
			}

			playerThread = playerThread->siblingThread;
		}

		if (playerThread != NULL)
		{
			respawn = nextRespawn;
			goto searchRespawn;
		}
	}
	}
	goto finish;

fallback:
{
	register struct GameTracker *fallbackTracker CTR_PSX_REGISTER("$5");

	fallbackTracker = GAME_TRACKER;
	CTR_PSX_KEEP_VALUE(fallbackTracker);
	driver->posCurr.x = CTR_MipsSll(CTR_MipsAddLo(fallbackTracker->level1->ptr_mesh_info->ptrVertexArray[(s16)sourceQuad->index[0]].pos.x,
	                                              fallbackTracker->level1->ptr_mesh_info->ptrVertexArray[(s16)sourceQuad->index[3]].pos.x),
	                                7);
	driver->posCurr.y = CTR_MipsSll(CTR_MipsAddLo(CTR_MipsAddLo(fallbackTracker->level1->ptr_mesh_info->ptrVertexArray[(s16)sourceQuad->index[0]].pos.y,
	                                                            fallbackTracker->level1->ptr_mesh_info->ptrVertexArray[(s16)sourceQuad->index[3]].pos.y),
	                                              VEH_STUCK_RESPAWN_Y_OFFSET),
	                                7);
	driver->posCurr.z = CTR_MipsSll(CTR_MipsAddLo(fallbackTracker->level1->ptr_mesh_info->ptrVertexArray[(s16)sourceQuad->index[0]].pos.z,
	                                              fallbackTracker->level1->ptr_mesh_info->ptrVertexArray[(s16)sourceQuad->index[3]].pos.z),
	                                7);
}

finish:
	GAME_TRACKER->cameraDC[driver->driverID].flags |= 1;
}


void VehStuckProc_MaskGrab_Particles(struct Driver *d)
{
	struct Particle *p;
	int i;

	for (i = 10; i != 0; i--)
	{
		// Create instance in particle pool
		p = Particle_Init(0, GAME_TRACKER->iconGroup[0], &data.emSet_Maskgrab[0]);

		if (p == NULL)
		{
			return;
		}

		// position variables
		p->axis[0].startVal = CTR_MipsAddLo(p->axis[0].startVal, d->posCurr.x);
		p->axis[1].startVal = CTR_MipsAddLo(p->axis[1].startVal, d->posCurr.y);
		p->axis[2].startVal = CTR_MipsAddLo(p->axis[2].startVal, d->posCurr.z);
	}
}


void VehStuckProc_MaskGrab_Update(struct Thread *t, struct Driver *d)
{
	struct MaskHeadWeapon *mask;

	d->NoInputTimer = (s16)CTR_MipsSubLo((u16)d->NoInputTimer, (u16)GAME_TRACKER->elapsedTimeMS);

	if (d->NoInputTimer < 0)
	{
		d->NoInputTimer = 0;
	}

	if (d->NoInputTimer != 0)
	{
		return;
	}

	// when input is allowed,
	// which is when driver is spawned back over track

	mask = d->KartStates.MaskGrab.maskObj;

	if (mask != NULL)
	{
		// mask rotZ
		mask->rot.z &= ~MASK_HEAD_ROT_WORLD_SPACE;

		// scale = 100%
		d->KartStates.MaskGrab.maskObj->scale = MASK_HEAD_SCALE_NORMAL;
	}


	// CameraDC flag
	GAME_TRACKER->cameraDC[d->driverID].flags |= CAMERA_FLAG_DIRECTION_CHANGED;


	VehStuckProc_MaskGrab_FindDestPos(d, d->lastValid);

	VehBirth_TeleportSelf(d, 0, VEH_STUCK_RESPAWN_Y_OFFSET);

	VehStuckProc_RevEngine_Init(t, d);
}


void VehStuckProc_MaskGrab_PhysLinear(struct Thread *t, struct Driver *d)
{
	u32 actionsFlagSet;
	u32 actionsClearMask;

	VehPhysProc_Driving_PhysLinear(t, d);
	actionsClearMask = ~(ACTION_REVERSING_ENGINE | ACTION_BRAKE_WITH_ACCEL | ACTION_JUMP_BUTTON_HELD);
	actionsFlagSet = d->actionsFlagSet;

	d->fireSpeed = 0;
	CTR_PSX_OBSERVE_MEMORY(d->fireSpeed);
	d->jump_TenBuffer = 0;
	d->fireSpeed = 0;
	d->baseSpeed = 0;

	// reset turning state
	d->simpTurnState = 0;

	d->actionsFlagSet = (actionsFlagSet & actionsClearMask) | ACTION_ACCEL_PREVENTION;
}


void VehStuckProc_MaskGrab_Animate(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int numFrames;
	struct MaskHeadWeapon *mask;

	if (d->KartStates.MaskGrab.boolStillFalling != false)
	{
		if ((d->KartStates.MaskGrab.boolWhistle == false) && (d->NoInputTimer < VEH_STUCK_MASK_GRAB_WHISTLE_TIME))
		{
			OtherFX_Play(VEH_STUCK_MASK_GRAB_WHISTLE_FX, 1);
			d->KartStates.MaskGrab.boolWhistle = true;
		}

		d->matrixArray = BAKED_GTE_MATRIX_CRASH_FALL;
		if (VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD <= d->KartStates.MaskGrab.animFrame)
		{
			d->matrixIndex = (u8)d->KartStates.MaskGrab.animFrame + VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET;
		}
		else
		{
			d->matrixIndex = VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP;
		}

		inst->animIndex = VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX;
		if (d->KartStates.MaskGrab.animFrame < VEH_STUCK_MASK_GRAB_CRASH_FRAME_THRESHOLD)
		{
			inst->animFrame = VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP;
		}
		else
		{
			inst->animFrame = (u16)d->KartStates.MaskGrab.animFrame + VEH_STUCK_MASK_GRAB_CRASH_FRAME_OFFSET;
		}

		d->KartStates.MaskGrab.animFrame = (u16)d->KartStates.MaskGrab.animFrame + 1;
		if (d->KartStates.MaskGrab.animFrame >= VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP + 1)
		{
			d->KartStates.MaskGrab.animFrame = VEH_STUCK_MASK_GRAB_CRASH_FRAME_CLAMP;
		}

		if (VEH_STUCK_MASK_GRAB_CRASH_TIME <= d->NoInputTimer)
		{
			register s32 previousX CTR_PSX_REGISTER("$2");
			register s32 previousY CTR_PSX_REGISTER("$3");
			register s32 previousZ CTR_PSX_REGISTER("$4");

			previousX = d->posPrev.x;
			previousY = d->posPrev.y;
			CTR_PSX_OBSERVE_VALUE(previousY);
			previousZ = d->posPrev.z;

			d->speed = 0;
			d->speedApprox = 0;
			d->posCurr.x = previousX;
			CTR_PSX_OBSERVE_MEMORY(d->posCurr.x);
			d->posCurr.y = previousY;
			d->posCurr.z = previousZ;
		}
		else
		{
			d->matrixArray = BAKED_GTE_MATRIX_CRASH_FALL;
			d->matrixIndex = VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME;
			inst->animIndex = VEH_STUCK_MASK_GRAB_CRASH_ANIM_INDEX;
			inst->animFrame = VEH_STUCK_MASK_GRAB_CRASH_FREEZE_FRAME;

			if (VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_LIMIT <= d->NoInputTimer)
			{
				if (d->KartStates.MaskGrab.boolParticlesSpawned == false)
				{
					VehStuckProc_MaskGrab_Particles(d);
					d->KartStates.MaskGrab.boolParticlesSpawned = true;
				}

				d->jumpSquishStretch = (s16)CTR_MipsAddLo((u16)d->jumpSquishStretch, VEH_STUCK_MASK_GRAB_SQUISH_INCREASE_STEP);
				if (d->jumpSquishStretch > VEH_STUCK_MASK_GRAB_SQUISH_MAX)
				{
					d->jumpSquishStretch = VEH_STUCK_MASK_GRAB_SQUISH_MAX;
				}
			}
			else
			{
				d->jumpSquishStretch = (s16)CTR_MipsSubLo((u16)d->jumpSquishStretch, VEH_STUCK_MASK_GRAB_SQUISH_DECREASE_STEP);
				if (d->jumpSquishStretch < 0)
				{
					d->jumpSquishStretch = 0;
				}
			}
		}
	}
	else
	{
		d->matrixArray = BAKED_GTE_MATRIX_NONE;
		d->matrixIndex = 0;
		inst->animIndex = 0;
		numFrames = VehFrameInst_GetNumAnimFrames(inst, 0);
		inst->animFrame = VehFrameInst_GetStartFrame(0, numFrames);
		d->AxisAngle2_normalVec.x = d->KartStates.MaskGrab.AngleAxis_NormalVec.x;
		d->AxisAngle2_normalVec.y = d->KartStates.MaskGrab.AngleAxis_NormalVec.y;
		d->AxisAngle2_normalVec.z = d->KartStates.MaskGrab.AngleAxis_NormalVec.z;
	}

	mask = d->KartStates.MaskGrab.maskObj;
	if (mask == NULL)
	{
		return;
	}

	d->KartStates.MaskGrab.maskObj->duration = MASK_HEAD_DURATION_NORMAL;
	if (d->NoInputTimer > VEH_STUCK_MASK_GRAB_SCALE_START_TIME)
	{
		d->KartStates.MaskGrab.maskObj->scale = 0;
		return;
	}

	if (d->KartStates.MaskGrab.boolLiftingPlayer != false)
	{
		register size_t work CTR_PSX_REGISTER("$2");
		register s32 elapsedTime CTR_PSX_REGISTER("$3");

		work = (size_t)GAME_TRACKER;
		elapsedTime = ((struct GameTracker *)work)->elapsedTimeMS;
		work = (u32)d->posCurr.y;
		d->speed = 0;
		elapsedTime = CTR_MipsSll(elapsedTime, VEH_STUCK_MASK_GRAB_LIFT_SHIFT);
		work = (u32)work + (u32)elapsedTime;
		d->posCurr.y = (s32)work;
		d->posPrev.y = (s32)work;
	}
	else
	{
		d->KartStates.MaskGrab.maskObj->pos.y = (s16)CTR_MipsSubLo((u16)d->KartStates.MaskGrab.maskObj->pos.y, (u16)GAME_TRACKER->elapsedTimeMS);
	}

	d->KartStates.MaskGrab.maskObj->pos.z = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);
	if (d->KartStates.MaskGrab.maskObj->pos.y < CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8))
	{
		d->KartStates.MaskGrab.maskObj->pos.y = (s16)CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8);
		d->KartStates.MaskGrab.boolLiftingPlayer = true;
	}
	d->KartStates.MaskGrab.maskObj->pos.x = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);

	if (VEH_STUCK_MASK_GRAB_SCALE_FULL_TIME <= d->NoInputTimer)
	{
		d->KartStates.MaskGrab.maskObj->scale =
		    (s16)(CTR_MipsSll(CTR_MipsSubLo(VEH_STUCK_MASK_GRAB_SCALE_START_TIME, d->NoInputTimer), FRACTIONAL_BITS) / VEH_STUCK_MASK_GRAB_SCALE_RAMP_TIME);
	}
	else
	{
		d->KartStates.MaskGrab.maskObj->scale = MASK_HEAD_SCALE_NORMAL;
	}
}


void VehStuckProc_MaskGrab_Init(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	struct MaskHeadWeapon *mask;
	int i;

	d->kartState = KS_MASK_GRABBED;

	d->KartStates.MaskGrab.boolParticlesSpawned = false;
	d->KartStates.MaskGrab.animFrame = 0;
	d->KartStates.MaskGrab.boolLiftingPlayer = false;
	d->KartStates.MaskGrab.boolWhistle = false;
	d->KartStates.MaskGrab.boolStillFalling = false;

	d->boolHadMaskBeforeOOB = (d->actionsFlagSet & ACTION_MASK_WEAPON) != 0;
	d->KartStates.MaskGrab.maskObj = VehPickupItem_MaskUseWeapon(d, true);

	d->turbo_MeterRoomLeft = 0;
	d->reserves = 0;
	d->turbo_outsideTimer = 0;
	d->matrixArray = BAKED_GTE_MATRIX_NONE;
	d->matrixIndex = 0;

	d->NoInputTimer = VEH_STUCK_MASK_GRAB_INITIAL_TIMER;

	d->actionsFlagSet &= ~(ACTION_AIRBORNE | ACTION_HIGH_JUMP);

	if (LOAD_IsOpen_RacingOrBattle() && ((GAME_TRACKER->gameMode1 & ADVENTURE_ARENA) == 0))
	{
		RB_Player_ModifyWumpa(d, -2);
	}

	if (CTR_MipsAddLo(d->quadBlockHeight, VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD) < d->posCurr.y)
	{
		d->numTimesMaskGrab++;

		if ((d->posCurr.y < -VEH_STUCK_MASK_GRAB_FALL_HEIGHT_THRESHOLD) && ((GAME_TRACKER->level1->configFlags & 2) != 0))
		{
			d->KartStates.MaskGrab.AngleAxis_NormalVec.x = d->AxisAngle2_normalVec.x;
			d->KartStates.MaskGrab.AngleAxis_NormalVec.y = d->AxisAngle2_normalVec.y;
			d->KartStates.MaskGrab.AngleAxis_NormalVec.z = d->AxisAngle2_normalVec.z;

			for (i = 10; i != 0; i--)
			{
				struct Particle *p = Particle_Init(0, GAME_TRACKER->iconGroup[9], &data.emSet_Falling[0]);
				if (p == NULL)
				{
					continue;
				}

				p->otIndexOffset = d->instSelf->depthBiasNormal;
				p->owner.driverInst = d->instSelf;
				p->driverID = d->driverID;
			}
		}
		else
		{
			d->KartStates.MaskGrab.boolStillFalling = true;
		}
	}
	else
	{
		d->KartStates.MaskGrab.AngleAxis_NormalVec.x = d->AxisAngle2_normalVec.x;
		d->KartStates.MaskGrab.AngleAxis_NormalVec.y = d->AxisAngle2_normalVec.y;
		d->KartStates.MaskGrab.AngleAxis_NormalVec.z = d->AxisAngle2_normalVec.z;
	}

	d->posCurr.x = CTR_MipsSll(inst->matrix.t[0], FRACTIONAL_BITS_8);
	d->posCurr.y = CTR_MipsSll(inst->matrix.t[1], FRACTIONAL_BITS_8);
	d->posCurr.z = CTR_MipsSll(inst->matrix.t[2], FRACTIONAL_BITS_8);

	d->posPrev.x = d->posCurr.x;
	d->posPrev.y = d->posCurr.y;
	d->posPrev.z = d->posCurr.z;

	mask = d->KartStates.MaskGrab.maskObj;
	if (mask != NULL)
	{
		mask->rot.z |= MASK_HEAD_ROT_WORLD_SPACE;

		d->KartStates.MaskGrab.maskObj->pos.x = (s16)CTR_MipsSra(d->posCurr.x, FRACTIONAL_BITS_8);
		d->KartStates.MaskGrab.maskObj->pos.y = (s16)CTR_MipsAddLo(CTR_MipsSra(d->posCurr.y, FRACTIONAL_BITS_8), VEH_STUCK_MASK_HEAD_Y_OFFSET);
		d->KartStates.MaskGrab.maskObj->pos.z = (s16)CTR_MipsSra(d->posCurr.z, FRACTIONAL_BITS_8);
	}

	d->funcPtrs[DRIVER_FUNC_UPDATE] = VehStuckProc_MaskGrab_Update;
	d->funcPtrs[DRIVER_FUNC_PHYS_LINEAR] = VehStuckProc_MaskGrab_PhysLinear;
	d->funcPtrs[DRIVER_FUNC_AUDIO] = VehPhysProc_Driving_Audio;
	d->funcPtrs[DRIVER_FUNC_PHYS_ANGULAR] = VehPhysGeneral_PhysAngular;
	d->funcPtrs[DRIVER_FUNC_APPLY_FORCES] = VehPhysForce_OnApplyForces;
	d->funcPtrs[DRIVER_FUNC_COLL_MOVED] = COLL_MOVED_PlayerSearch;
	d->funcPtrs[DRIVER_FUNC_COLLIDE_DRIVERS] = VehPhysForce_CollideDrivers;
	d->funcPtrs[DRIVER_FUNC_COLL_FIXED] = COLL_FIXED_PlayerSearch;
	d->funcPtrs[DRIVER_FUNC_JUMP_FRICTION] = VehPhysGeneral_JumpAndFriction;
	d->funcPtrs[DRIVER_FUNC_TRANSLATE_MATRIX] = VehPhysForce_TranslateMatrix;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = VehStuckProc_MaskGrab_Animate;
	d->funcPtrs[DRIVER_FUNC_INIT] = NULL;
	d->funcPtrs[DRIVER_FUNC_PARTICLES] = VehEmitter_DriverMain;
}


void VehStuckProc_PlantEaten_Update(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;

	d->NoInputTimer = (s16)CTR_MipsSubLo((u16)d->NoInputTimer, (u16)GAME_TRACKER->elapsedTimeMS);

	if (d->NoInputTimer < 0)
	{
		d->NoInputTimer = 0;
	}

	if (d->NoInputTimer != 0)
	{
		return;
	}

	// respawn driver at last valid quadblock
	VehStuckProc_MaskGrab_FindDestPos(d, d->lastValid);
	VehBirth_TeleportSelf(d, 0, VEH_STUCK_RESPAWN_Y_OFFSET);

	// enable collision, make visible
	t->flags &= ~THREAD_FLAG_DISABLE_COLLISION;
	inst->flags &= ~(HIDE_MODEL);

	// this lets you rev engine while falling
	VehStuckProc_RevEngine_Init(t, d);
}


void VehStuckProc_PlantEaten_PhysLinear(struct Thread *t, struct Driver *d)
{
	u32 actionsFlagSet;
	u32 actionsClearMask;

	VehPhysProc_Driving_PhysLinear(t, d);
	actionsClearMask = ~(ACTION_REVERSING_ENGINE | ACTION_BRAKE_WITH_ACCEL | ACTION_JUMP_BUTTON_HELD);
	actionsFlagSet = d->actionsFlagSet;

	d->fireSpeed = 0;
	CTR_PSX_OBSERVE_MEMORY(d->fireSpeed);
	d->jump_TenBuffer = 0;
	d->fireSpeed = 0;
	d->baseSpeed = 0;
	d->simpTurnState = 0;

	// acceleration prevention,
	// drop jump-button, gas+brake, and reversing engine bits.
	d->actionsFlagSet = (actionsFlagSet & actionsClearMask) | ACTION_ACCEL_PREVENTION;

	d->timeSpentEaten = CTR_MipsAddLo(d->timeSpentEaten, GAME_TRACKER->elapsedTimeMS);
}


void VehStuckProc_PlantEaten_Animate(struct Thread *t, struct Driver *d)
{
	s32 dist;
	s32 cameraPitch;
	struct GameTracker *gameTracker;
	register struct Instance *inst CTR_PSX_REGISTER("$20");
	SVECTOR plantVector;
	VECTOR camVec;
	s32 gteFlags[2];
	register struct Driver *driver CTR_PSX_REGISTER("$19") = d;
	register struct Thread *plant CTR_PSX_REGISTER("$16") = driver->plantEatingMe;
	register struct Thread *plantObjectThread CTR_PSX_REGISTER("$3");
	register int camY CTR_PSX_REGISTER("$18");
	register s32 plantSide CTR_PSX_REGISTER("$2");
	int camX;
	int camZ;
	u8 cameraYOffset;

	(void)t;

	// if any plant is eating me
	if (((plant != NULL) &&

	     // if not initialized
	     (driver->KartStates.EatenByPlant.boolInited == false)) &&

	    // if more than 0.5s since player death
	    (driver->NoInputTimer < VEH_STUCK_PLANT_CAMERA_INIT_TIME))
	{
		plantObjectThread = plant;
		CTR_PSX_KEEP_VALUE(plantObjectThread);

		// get instance from thread
		inst = plant->inst;

		// initialized, player eaten
		driver->KartStates.EatenByPlant.boolInited = true;

		plantSide = ((struct Plant *)plantObjectThread->object)->side;
		CTR_PSX_KEEP_VALUE(plantSide);
		if (plantSide != 0)
		{
			plantSide = -VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET;
		}
		else
		{
			plantSide = VEH_STUCK_PLANT_CAMERA_SIDE_OFFSET;
		}
		plantVector.vx = plantSide;
		CTR_PSX_OBSERVE_MEMORY(plantVector.vx);
		plantVector.vy = 0;
		plantVector.vz = VEH_STUCK_PLANT_CAMERA_FORWARD_OFFSET;

		SetRotMatrix(&inst->matrix);

		SetTransMatrix(&inst->matrix);

		RotTrans(&plantVector, &camVec, gteFlags);

		GAME_TRACKER->pushBuffer[driver->driverID].pos.x = camVec.vx;
		{
			register u32 driverID CTR_PSX_REGISTER("$3") = driver->driverID;
			u8 *driverGameTracker;

			// NOTE(aalhendi): Keep the driver stride separate from the field
			// offset so GCC preserves retail's base-first pointer addition.
			CTR_PSX_KEEP_VALUE(driverID);
			driverGameTracker = (u8 *)GAME_TRACKER + (driverID * sizeof(struct PushBuffer));
			*(s16 *)(driverGameTracker + offsetof(struct GameTracker, pushBuffer) + offsetof(struct PushBuffer, pos.y)) =
			    CTR_MipsAddLo((u16)inst->matrix.t[1], VEH_STUCK_PLANT_CAMERA_Y_OFFSET);
		}
		GAME_TRACKER->pushBuffer[driver->driverID].pos.z = camVec.vz;

		camX = camVec.vx - inst->matrix.t[0];
		camY = GAME_TRACKER->pushBuffer[driver->driverID].pos.y;
		camZ = camVec.vz - inst->matrix.t[2];
		camY = camY - inst->matrix.t[1];

		GAME_TRACKER->pushBuffer[driver->driverID].rot.y = (s16)ratan2(camX, camZ);

		// get distance between car and camera
		dist = SquareRoot0_stub(CTR_MipsAddLo(CTR_MipsMulLo(camX, camX), CTR_MipsMulLo(camZ, camZ)));

		// NOTE(aalhendi): The camera Y position was written from this matrix
		// Y plus 192, so their low-byte delta is the desired camera offset.
		cameraYOffset = camY;
		cameraPitch = ratan2(cameraYOffset, dist);
		gameTracker = GAME_TRACKER;
		gameTracker->pushBuffer[driver->driverID].rot.x = VEH_STUCK_PLANT_CAMERA_PITCH_BASE - cameraPitch;

		gameTracker->pushBuffer[driver->driverID].rot.z = 0;
	}
}


// when eaten by plant on papu pyramid
void VehStuckProc_PlantEaten_Init(struct Thread *t, struct Driver *d)
{
	// when this function executes, you are lifted
	// above the track by the mask, where you respawn

	struct Instance *inst = t->inst;

	// set state to mask grab, so nobody hits you with weapon
	d->kartState = KS_MASK_GRABBED;

	d->KartStates.EatenByPlant.boolInited = false;

	d->turbo_MeterRoomLeft = 0;
	d->reserves = 0;
	d->turbo_outsideTimer = 0;

	// drop bits for airborne and high-jump state
	d->actionsFlagSet &= ~(ACTION_AIRBORNE | ACTION_HIGH_JUMP);

	// "cloud" is the raincloud after hitting red potion

	// if thread of "cloud" exists
	if (d->thCloud != NULL)
	{
		((struct RainCloud *)d->thCloud->object)->timeMS = 0;

		d->thCloud->funcThTick = RB_RainCloud_FadeAway;
		d->thCloud = NULL;
	}

	if (LOAD_IsOpen_RacingOrBattle() && ((GAME_TRACKER->gameMode1 & ADVENTURE_ARENA) == 0))
	{
		RB_Player_ModifyWumpa(d, -2);
	}

	// allow this thread to ignore all collisions
	t->flags |= THREAD_FLAG_DISABLE_COLLISION;

	// make invisible
	inst->flags |= HIDE_MODEL;

	OtherFX_Stop1((int)d->driverAudioPtrs[1]);
	d->driverAudioPtrs[1] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[2]);
	d->driverAudioPtrs[2] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[0]);

	d->funcPtrs[DRIVER_FUNC_UPDATE] = VehStuckProc_PlantEaten_Update;
	d->funcPtrs[DRIVER_FUNC_PHYS_LINEAR] = VehStuckProc_PlantEaten_PhysLinear;
	d->funcPtrs[DRIVER_FUNC_AUDIO] = VehPhysProc_Driving_Audio;
	d->driverAudioPtrs[0] = 0;
	d->funcPtrs[DRIVER_FUNC_INIT] = NULL;
	d->funcPtrs[DRIVER_FUNC_PHYS_ANGULAR] = NULL;
	d->funcPtrs[DRIVER_FUNC_APPLY_FORCES] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_MOVED] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLLIDE_DRIVERS] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_FIXED] = NULL;
	d->funcPtrs[DRIVER_FUNC_JUMP_FRICTION] = NULL;
	d->funcPtrs[DRIVER_FUNC_TRANSLATE_MATRIX] = NULL;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = VehStuckProc_PlantEaten_Animate;
	d->funcPtrs[DRIVER_FUNC_PARTICLES] = NULL;
}


void VehStuckProc_RIP_Init(struct Thread *t, struct Driver *d)
{
	VehStuckProc_PlantEaten_Init(t, d);
	d->funcPtrs[DRIVER_FUNC_UPDATE] = NULL;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = NULL;
	d->invisibleTimer = 0;
}


void VehStuckProc_RevEngine_Update(struct Thread *t, struct Driver *d)
{
	// If a mask grab has not lowered you close enough to the track, wait.
	if (d->KartStates.RevEngine.boolMaskGrab != false)
	{
		if (CTR_MipsAddLo(d->quadBlockHeight, VEH_STUCK_REV_MASK_RELEASE_HEIGHT) <= d->posCurr.y)
		{
			return;
		}
	}
	// If this is the start of a race, wait for the traffic lights.
	else if (0 < GAME_TRACKER->trafficLightsTimer)
	{
		return;
	}

	// Assume it's time to transition out of being
	// frozen, and into driving, last iteration of
	// this function

	if ((d->KartStates.RevEngine.boolMaskGrab == true) && (d->KartStates.RevEngine.maskObj != NULL) && !(g_config.maskPersistsAfterOOB && d->boolHadMaskBeforeOOB))
	{
		d->KartStates.RevEngine.maskObj->duration = 0;
	}

	if ((d->const_AccelSpeed_ClassStat < d->KartStates.RevEngine.fireLevel) && (d->KartStates.RevEngine.lockoutFlags & REV_ENGINE_LOCKOUT_ALL) == 0)
	{
		// While not moving, if you rev'd your engine less than...
		if (CTR_MipsAddLo(d->const_AccelSpeed_ClassStat, d->const_SacredFireSpeed) <= d->KartStates.RevEngine.boostMeter)
		{
			// You get a big boost
			VehFire_Increment(d, VEH_STUCK_REV_START_BOOST_RESERVES, 0, VEH_STUCK_REV_BIG_BOOST_FIRE_LEVEL);
		}

		// if you rev'd your engine high
		else
		{
			// You get a small boost
			VehFire_Increment(d, VEH_STUCK_REV_START_BOOST_RESERVES, 0, VEH_STUCK_REV_SMALL_BOOST_FIRE_LEVEL);
		}
	}

	// full meter
	d->turbo_MeterRoomLeft = 0;
	d->revEngineState = 0;

	VehPhysProc_Driving_Init(t, d);
}


void VehStuckProc_RevEngine_PhysLinear(struct Thread *t, struct Driver *d)
{
	s32 cooldownTimer;

	cooldownTimer = (u16)d->KartStates.RevEngine.releaseCooldownTimerMS;
	cooldownTimer = CTR_MipsSubLo(cooldownTimer, (u16)GAME_TRACKER->elapsedTimeMS);
	d->KartStates.RevEngine.releaseCooldownTimerMS = (s16)cooldownTimer;
	cooldownTimer = CTR_MipsSll(cooldownTimer, 16);
	if (cooldownTimer < 0)
	{
		d->KartStates.RevEngine.releaseCooldownTimerMS = 0;
	}

	cooldownTimer = (u16)d->KartStates.RevEngine.emptyCooldownTimerMS;
	cooldownTimer = CTR_MipsSubLo(cooldownTimer, (u16)GAME_TRACKER->elapsedTimeMS);
	d->KartStates.RevEngine.emptyCooldownTimerMS = (s16)cooldownTimer;
	cooldownTimer = CTR_MipsSll(cooldownTimer, 16);
	if (cooldownTimer < 0)
	{
		d->KartStates.RevEngine.emptyCooldownTimerMS = 0;
	}

	VehPhysProc_Driving_PhysLinear(t, d);

	if (d->KartStates.RevEngine.boolMaskGrab == 0)
	{
		return;
	}

	GAME_TRACKER->cameraDC[d->driverID].flags |= CAMERA_FLAG_MASK_GRAB;
	GAME_TRACKER->cameraDC[d->driverID].maskGrabHeightOffset = VEH_STUCK_REV_CAMERA_HEIGHT_OFFSET;

	d->posCurr.y = CTR_MipsSubLo(d->posCurr.y, VEH_STUCK_REV_MASK_DESCENT_STEP);

	// if maskObj exists
	if (d->KartStates.RevEngine.maskObj != 0)
	{
		d->KartStates.RevEngine.maskObj->duration = MASK_HEAD_DURATION_NORMAL;
	}
}


void VehStuckProc_RevEngine_Animate(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int fillStep;
	register int revLevel CTR_PSX_REGISTER("$2");
	int boostMeter;
	u32 packedStatus;
	int accelClassStat;
	s16 meterRoomLeft;
	int squishScale;

	if ((d->fireSpeed > 0) && (d->KartStates.RevEngine.releaseCooldownTimerMS == 0) && ((d->KartStates.RevEngine.lockoutFlags & REV_ENGINE_LOCKOUT_ALL) == 0))
	{
		int revDelta = CTR_MipsSubLo(d->KartStates.RevEngine.fireLevel, d->KartStates.RevEngine.boostMeter);
		if (revDelta < 0)
		{
			revDelta = CTR_MipsNegLo(revDelta);
		}

		CTR_PSX_KEEP_VALUE(revDelta);
		revDelta = CTR_MipsSra(revDelta, 1);

		fillStep = revDelta;

		// Speed of filling the meter changes
		// depending on how full the meter is,
		// there are two speeds
		if (VEH_STUCK_REV_FILL_STEP_MAX < revDelta)
		{
			fillStep = VEH_STUCK_REV_FILL_STEP_MAX;
		}

		if (fillStep < VEH_STUCK_REV_STEP_MIN)
		{
			fillStep = VEH_STUCK_REV_STEP_MIN;
		}

		{
			register int boostMeterSnapshot CTR_PSX_REGISTER("$4");
			register int activeState CTR_PSX_REGISTER("$3");

			revLevel = VehCalc_InterpBySpeed(d->KartStates.RevEngine.fireLevel, fillStep, d->KartStates.RevEngine.boostMeter);
			boostMeterSnapshot = d->KartStates.RevEngine.boostMeter;
			activeState = REV_ENGINE_CHARGE_ACTIVE;

			d->KartStates.RevEngine.fireLevel = revLevel;
			d->KartStates.RevEngine.chargeState = activeState;

			if (boostMeterSnapshot <= revLevel)
			{
				struct GameTracker *gameTracker;
				s16 overRevTimerMS;

				VEH_LOAD_GAME_TRACKER(gameTracker);
				overRevTimerMS = (s16)CTR_MipsAddLo((u16)d->KartStates.RevEngine.overRevTimerMS, (u16)gameTracker->elapsedTimeMS);
				d->KartStates.RevEngine.overRevTimerMS = overRevTimerMS;

				if (VEH_STUCK_REV_OVERREV_TIMEOUT < overRevTimerMS)
				{
					d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;
					d->KartStates.RevEngine.lockoutFlags |= REV_ENGINE_LOCKOUT_ALL;

					OtherFX_Play_Echo(VEH_STUCK_REV_OVERREV_FX, 1, ((u32)d->actionsFlagSet >> 16) & 1);
				}
			}
			else
			{
				d->KartStates.RevEngine.overRevTimerMS = 0;
			}
		}
		goto LAB_80067dec;
	}
	d->KartStates.RevEngine.overRevTimerMS = 0;

	if (d->KartStates.RevEngine.chargeState == REV_ENGINE_CHARGE_ACTIVE)
	{
		d->KartStates.RevEngine.releaseCooldownTimerMS = VEH_STUCK_REV_RELEASE_COOLDOWN;
		d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;

		if (d->const_AccelSpeed_ClassStat < d->KartStates.RevEngine.fireLevel)
		{
			d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_RELEASED_ABOVE_ACCEL;
		}
	}

	if (d->KartStates.RevEngine.chargeState != REV_ENGINE_CHARGE_IDLE)
	{
		register s32 accelSpeed CTR_PSX_REGISTER("$3");

		accelSpeed = d->const_AccelSpeed_ClassStat;
		if (d->KartStates.RevEngine.fireLevel < accelSpeed)
		{
			register s32 interpSpeed CTR_PSX_REGISTER("$5");
			register s32 interpTarget CTR_PSX_REGISTER("$6");
#ifndef CTR_NATIVE
			register s32 divideMagicHigh CTR_PSX_REGISTER("$2") = 0x55550000;
#endif

			interpSpeed = accelSpeed;
#ifdef CTR_NATIVE
			CTR_PSX_KEEP_VALUE(interpSpeed);
#else
			__asm__("" : "+r"(interpSpeed) : "r"(divideMagicHigh));
#endif
			interpTarget = interpSpeed + d->const_SacredFireSpeed;
			interpSpeed = interpTarget - interpSpeed;
			d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;
			boostMeter = VehCalc_InterpBySpeed(d->KartStates.RevEngine.boostMeter, interpSpeed / 3 + 3, interpTarget);
			d->KartStates.RevEngine.boostMeter = boostMeter;
		}
	}

	if (0 < d->KartStates.RevEngine.fireLevel)
	{
		register s32 decayStep CTR_PSX_REGISTER("$5") = CTR_MipsSra(d->KartStates.RevEngine.fireLevel, 1);
		if ((d->KartStates.RevEngine.lockoutFlags & REV_ENGINE_LOCKOUT_REV_DECAY) != 0)
		{
			if (VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED < decayStep)
			{
				decayStep = VEH_STUCK_REV_DECAY_STEP_MAX_LOCKED;
			}
		}
		else
		{
			if (VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL < decayStep)
			{
				decayStep = VEH_STUCK_REV_DECAY_STEP_MAX_NORMAL;
			}
		}

		if (decayStep < VEH_STUCK_REV_STEP_MIN)
		{
			decayStep = VEH_STUCK_REV_STEP_MIN;
		}

		revLevel = CTR_MipsSubLo(d->KartStates.RevEngine.fireLevel, decayStep);
		d->KartStates.RevEngine.fireLevel = revLevel;

		if (revLevel < 1)
		{
			d->KartStates.RevEngine.emptyCooldownTimerMS = VEH_STUCK_REV_EMPTY_COOLDOWN;
			d->KartStates.RevEngine.fireLevel = 0;
		}
	}
	else
	{
		s32 sacredFireThird;

#ifdef CTR_NATIVE
		d->KartStates.RevEngine.lockoutFlags &= ~REV_ENGINE_LOCKOUT_REV_DECAY;
		sacredFireThird = d->const_SacredFireSpeed / 3;
		d->KartStates.RevEngine.boostMeter = d->const_AccelSpeed_ClassStat + sacredFireThird;
#else
		register s32 divideMagic CTR_PSX_REGISTER("$2") = 0x55550000;
		register s32 sacredFireSpeed CTR_PSX_REGISTER("$3");

		CTR_PSX_LOAD_SIGNED_HALF(sacredFireSpeed, d, offsetof(struct Driver, const_SacredFireSpeed), d->const_SacredFireSpeed);
		__asm__("ori %0,%1,0x5556" : "=r"(divideMagic) : "r"(divideMagic), "r"(sacredFireSpeed));
		__asm__ volatile("mult %0,%1" : : "r"(sacredFireSpeed), "r"(divideMagic));
		d->KartStates.RevEngine.lockoutFlags &= ~REV_ENGINE_LOCKOUT_REV_DECAY;
		sacredFireSpeed >>= 31;
		__asm__ volatile("lh $2,%2(%0)\n\tmfhi $8\n\tsubu %1,$8,%1\n\taddu $2,$2,%1\n\tsw $2,%3(%0)"
		                 : "+r"(d), "+r"(sacredFireSpeed)
		                 : "I"(offsetof(struct Driver, const_AccelSpeed_ClassStat)), "I"(offsetof(struct Driver, KartStates.RevEngine.boostMeter))
		                 : "$2", "memory");
#endif
	}

	if (d->fireSpeed < 1)
	{
		d->KartStates.RevEngine.lockoutFlags &= ~REV_ENGINE_LOCKOUT_PEDAL_HELD;
	}

LAB_80067dec:;

	packedStatus = *(VehStuckProcWord *)(void *)&d->KartStates.RevEngine.emptyCooldownTimerMS;
	if ((packedStatus & REV_ENGINE_PACKED_BUSY_MASK) == 0)
	{
		goto inspectRevEngineLevel;
	}

	d->revEngineState = 2;
	goto revEngineStateReady;

inspectRevEngineLevel:
	if (d->KartStates.RevEngine.fireLevel < d->const_AccelSpeed_ClassStat)
	{
		goto revEngineBelowAccel;
	}

	d->revEngineState = 1;
	goto revEngineStateReady;

revEngineBelowAccel:
	d->revEngineState = 0;
revEngineStateReady:
	accelClassStat = d->const_AccelSpeed_ClassStat;

	d->speedometerNeedleValue = d->KartStates.RevEngine.fireLevel;

	if (d->KartStates.RevEngine.fireLevel < accelClassStat)
	{
		int lowRoomEnd;

		// 476 and 447 can be absolutely any value,
		// by default they are 15 and 30, but as long as
		// they are proportional (1 and 2, 4 and 8), they
		// behave the same as 15 and 30

		// 477 changes when meter turns red
		lowRoomEnd = CTR_MipsSll((u8)d->const_turboLowRoomWarning, VEH_STUCK_REV_TURBO_ROOM_SHIFT) + 1;
		meterRoomLeft = VehCalc_MapToRange(d->KartStates.RevEngine.fireLevel, 0, accelClassStat,
		                                   CTR_MipsSll(d->const_turboMaxRoom, VEH_STUCK_REV_TURBO_ROOM_SHIFT), lowRoomEnd);
	}
	else
	{
		// 477 changes when meter turns red
		meterRoomLeft = VehCalc_MapToRange(d->KartStates.RevEngine.fireLevel, accelClassStat, CTR_MipsAddLo(accelClassStat, d->const_SacredFireSpeed),
		                                   CTR_MipsSll(d->const_turboLowRoomWarning, VEH_STUCK_REV_TURBO_ROOM_SHIFT), 1);
	}

	d->turbo_MeterRoomLeft = meterRoomLeft;
	CTR_PSX_MEMORY_BARRIER();

	d->distanceDrivenBackwards = 0;
	{
		register int squishValue CTR_PSX_REGISTER("$2");
		register int squishCompare CTR_PSX_REGISTER("$3");

		squishValue = CTR_MipsSra((s16)d->speedometerNeedleValue, VEH_STUCK_REV_SQUISH_SHIFT);
		CTR_PSX_KEEP_VALUE(squishValue);
		squishCompare = squishValue;
		d->jumpSquishStretch = (s16)squishValue;
		squishScale = squishCompare;
	}

	if (VEH_STUCK_REV_SQUISH_LIMIT <= squishScale)
	{
		d->jumpSquishStretch = VEH_STUCK_REV_SQUISH_MAX;
	}
	else
	{
		if (squishScale < 0)
		{
			d->jumpSquishStretch = 0;
		}
	}

	// Set the scale of the car while revving the engine,
	// this is a basic "squash and stretch" concept of animation, before motion

	// Reduce height a little
	inst->scale.y = (s16)CTR_MipsSubLo(VEH_STUCK_REV_MODEL_BASE_SCALE, (u16)d->jumpSquishStretch);
	inst->scale.x = (s16)(d->jumpSquishStretch * VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR / VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR +
	                      VEH_STUCK_REV_MODEL_BASE_SCALE);
	inst->scale.z = (s16)(d->jumpSquishStretch * VEH_STUCK_REV_MODEL_WIDTH_SQUISH_NUMERATOR / VEH_STUCK_REV_MODEL_WIDTH_SQUISH_DENOMINATOR +
	                      VEH_STUCK_REV_MODEL_BASE_SCALE);
}


void VehStuckProc_RevEngine_Init(struct Thread *t, struct Driver *d)
{
	(void)t;
	// spawn function that waits for traffic lights

	// kart state to rev
	d->kartState = KS_ENGINE_REVVING;
	d->revEngineState = 0;

	// assume reason for revving is: start of race
	d->KartStates.RevEngine.fireLevel = 0;
	d->KartStates.RevEngine.boolMaskGrab = false;
	d->KartStates.RevEngine.maskObj = NULL;

	// if this is a mask grab
	if (CTR_MipsAddLo(d->quadBlockHeight, VEH_STUCK_REV_MASK_GRAB_HEIGHT_TRIGGER) < d->posCurr.y)
	{
		// assume reason for revving is: mask grab
		d->KartStates.RevEngine.boolMaskGrab = true;
		d->KartStates.RevEngine.maskObj = VehPickupItem_MaskUseWeapon(d, false);

		d->actionsFlagSet &= ~ACTION_TOUCH_GROUND;

		// CameraDC flag
		GAME_TRACKER->cameraDC[d->driverID].flags |= CAMERA_FLAG_DIRECTION_CHANGED;
	}

	d->boolFirstFrameSinceRevEngine = true;
	d->funcPtrs[DRIVER_FUNC_UPDATE] = VehStuckProc_RevEngine_Update;
	d->funcPtrs[DRIVER_FUNC_PHYS_LINEAR] = VehStuckProc_RevEngine_PhysLinear;
	d->funcPtrs[DRIVER_FUNC_AUDIO] = VehPhysProc_Driving_Audio;
	d->funcPtrs[DRIVER_FUNC_TRANSLATE_MATRIX] = VehPhysForce_TranslateMatrix;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = VehStuckProc_RevEngine_Animate;
	d->funcPtrs[DRIVER_FUNC_PARTICLES] = VehEmitter_DriverMain;
	d->KartStates.RevEngine.overRevTimerMS = 0;
	d->KartStates.RevEngine.releaseCooldownTimerMS = 0;
	d->KartStates.RevEngine.chargeState = REV_ENGINE_CHARGE_IDLE;
	d->KartStates.RevEngine.lockoutFlags = 0;
	d->KartStates.RevEngine.emptyCooldownTimerMS = 0;
	d->funcPtrs[DRIVER_FUNC_INIT] = NULL;
	d->funcPtrs[DRIVER_FUNC_PHYS_ANGULAR] = NULL;
	d->funcPtrs[DRIVER_FUNC_APPLY_FORCES] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_MOVED] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLLIDE_DRIVERS] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_FIXED] = NULL;
	d->funcPtrs[DRIVER_FUNC_JUMP_FRICTION] = NULL;

	// NOTE(aalhendi): Keep the signed halfword promotion inside the expression;
	// GCC 2.8 otherwise reloads it unsigned and emits a separate sign extension.
	d->KartStates.RevEngine.boostMeter = d->const_AccelSpeed_ClassStat + CTR_MipsSra(CTR_MipsSll(d->const_SacredFireSpeed, 16), 16) / 3;
}


void VehStuckProc_Tumble_Update(struct Thread *thread, struct Driver *driver)
{
	if (driver->NoInputTimer != 0)
	{
		return;
	}

	driver->matrixArray = BAKED_GTE_MATRIX_NONE;
	driver->matrixIndex = 0;
	VehPhysProc_Driving_Init(thread, driver);
}


void VehStuckProc_Tumble_PhysLinear(struct Thread *thread, struct Driver *driver)
{
	driver->NoInputTimer = (s16)CTR_MipsSubLo((u16)driver->NoInputTimer, (u16)GAME_TRACKER->elapsedTimeMS);

	if (driver->NoInputTimer < 0)
	{
		driver->NoInputTimer = 0;
	}

	VehPhysProc_Driving_PhysLinear(thread, driver);

	driver->jump_ForcedMS = VEH_TUMBLE_FORCED_JUMP_MS;
	driver->baseSpeed = 0;
	driver->fireSpeed = 0;
	driver->actionsFlagSet |= ACTION_WARP | ACTION_FRONT_SKID | ACTION_BACK_SKID | ACTION_ACCEL_PREVENTION;
	driver->jump_InitialVelY = (s16)CTR_MipsAddLo(CTR_MipsSll((u16)driver->NoInputTimer, VEH_TUMBLE_JUMP_VELOCITY_TIMER_SHIFT), VEH_TUMBLE_JUMP_VELOCITY_BASE);
}


void VehStuckProc_Tumble_PhysAngular(struct Thread *thread, struct Driver *driver)
{
	register s32 rotationSpinRate CTR_PSX_REGISTER("$3");
	register s32 angularValue CTR_PSX_REGISTER("$4");
	register struct GameTracker *gGT CTR_PSX_REGISTER("$5");
	register s32 turnAngleCurr CTR_PSX_REGISTER("$2");

	(void)thread;
	rotationSpinRate = (u16)driver->rotationSpinRate;
	angularValue = (u16)driver->turnAngleLerpVel;
	VEH_LOAD_GAME_TRACKER(gGT);

	driver->numFramesSpentSteering = VEH_TUMBLE_STEERING_FRAME_SENTINEL;

	rotationSpinRate -= (s16)rotationSpinRate >> VEH_TUMBLE_ANGULAR_DAMP_SHIFT;
	driver->rotationSpinRate = (s16)rotationSpinRate;
	CTR_PSX_MEMORY_BARRIER();

	rotationSpinRate = (s16)angularValue >> VEH_TUMBLE_ANGULAR_DAMP_SHIFT;
	angularValue -= rotationSpinRate;

	turnAngleCurr = (u16)driver->turnAngleCurr;
	rotationSpinRate = (u16)driver->rotationSpinRate;
	driver->turnAngleLerpVel = (s16)angularValue;
	driver->ampTurnState = (s16)rotationSpinRate;

	turnAngleCurr += angularValue;
	turnAngleCurr += VEH_TUMBLE_TURN_WRAP_BIAS;
	turnAngleCurr &= VEH_TUMBLE_TURN_MASK;
	turnAngleCurr -= VEH_TUMBLE_TURN_WRAP_BIAS;
	driver->turnAngleCurr = (s16)turnAngleCurr;

	angularValue = (u16)driver->turnWobbleAngle;
	angularValue -= (s16)angularValue >> VEH_TUMBLE_ANGULAR_DAMP_SHIFT;
	driver->turnWobbleAngle = (s16)angularValue;

	rotationSpinRate = (s16)rotationSpinRate;
	driver->angle += (s16)((rotationSpinRate * gGT->elapsedTimeMS) >> VEH_TUMBLE_SPIN_RATE_SHIFT);
	driver->angle &= VEH_TUMBLE_TURN_MASK;

	(driver->rotCurr).y = driver->angle + driver->turnAngleCurr + driver->turnWobbleAngle;

	(driver->rotCurr).w =
	    VehCalc_InterpBySpeed((int)(driver->rotCurr).w, (gGT->elapsedTimeMS << VEH_TUMBLE_ROT_W_INTERP_SHIFT) >> VEH_TUMBLE_ROT_W_INTERP_SHIFT, 0);

	VehPhysForce_RotAxisAngle(&driver->matrixMovingDir, CTR_VECTOR_DATA(&(driver->AxisAngle1_normalVec)), driver->angle);
}


void VehStuckProc_Tumble_Animate(struct Thread *thread, struct Driver *driver)
{
	int matrixIndex;
	int arrLength;

	(void)thread;

	// divide by 32ms to get frame index
	matrixIndex = CTR_MipsSra(driver->NoInputTimer, VEH_TUMBLE_ANIM_FRAME_TIME_SHIFT);

	driver->matrixArray = BAKED_GTE_MATRIX_BLASTED;
	arrLength = data.bakedGteMath[BAKED_GTE_MATRIX_BLASTED].numEntries;

	// modulus to wrap repeat animation
	matrixIndex %= arrLength;

	if (driver->KartStates.Blasted.boolPlayBackwards != 0)
	{
		matrixIndex = CTR_MipsSubLo(arrLength, CTR_MipsAddLo(matrixIndex, VEH_TUMBLE_ANIM_REVERSE_OFFSET));
	}

	driver->matrixIndex = matrixIndex;
}


void VehStuckProc_Tumble_Init(struct Thread *thread, struct Driver *driver)
{
	int numAnimFrames;
	int animFrame;
	int rng;
	s8 simpTurnState;

	(void)thread;
	driver->kartState = KS_BLASTED;
	driver->turbo_MeterRoomLeft = 0;

	if (LOAD_IsOpen_RacingOrBattle() && ((GAME_TRACKER->gameMode1 & ADVENTURE_ARENA) == 0))
	{
		RB_Player_ModifyWumpa(driver, -VEH_TUMBLE_WUMPA_PENALTY);
	}

	driver->instSelf->animIndex = 0;

	numAnimFrames = VehFrameInst_GetNumAnimFrames(driver->instSelf, 0);
	animFrame = VehFrameInst_GetStartFrame(0, numAnimFrames);

	driver->instSelf->animFrame = (s16)animFrame;

	rng = MixRNG_Scramble();
	driver->KartStates.Blasted.boolPlayBackwards = rng & VEH_TUMBLE_BACKWARDS_RNG_MASK;

	driver->funcPtrs[DRIVER_FUNC_UPDATE] = VehStuckProc_Tumble_Update;
	driver->funcPtrs[DRIVER_FUNC_PHYS_LINEAR] = VehStuckProc_Tumble_PhysLinear;
	driver->funcPtrs[DRIVER_FUNC_AUDIO] = VehPhysProc_Driving_Audio;
	driver->funcPtrs[DRIVER_FUNC_PHYS_ANGULAR] = VehStuckProc_Tumble_PhysAngular;
	driver->funcPtrs[DRIVER_FUNC_APPLY_FORCES] = VehPhysForce_OnApplyForces;
	driver->funcPtrs[DRIVER_FUNC_COLL_MOVED] = COLL_MOVED_PlayerSearch;
	driver->funcPtrs[DRIVER_FUNC_COLLIDE_DRIVERS] = VehPhysForce_CollideDrivers;
	driver->funcPtrs[DRIVER_FUNC_COLL_FIXED] = COLL_FIXED_PlayerSearch;
	driver->funcPtrs[DRIVER_FUNC_JUMP_FRICTION] = VehPhysGeneral_JumpAndFriction;
	driver->funcPtrs[DRIVER_FUNC_TRANSLATE_MATRIX] = VehPhysForce_TranslateMatrix;
	driver->funcPtrs[DRIVER_FUNC_ANIMATE] = VehStuckProc_Tumble_Animate;
	driver->funcPtrs[DRIVER_FUNC_INIT] = NULL;
	driver->funcPtrs[DRIVER_FUNC_PARTICLES] = VehEmitter_DriverMain;

	simpTurnState = driver->simpTurnState;
	if (simpTurnState > 0)
	{
		GAMEPAD_JogCon1(driver, VEH_TUMBLE_RUMBLE_STRONG, VEH_TUMBLE_RUMBLE_DURATION);
	}
	else
	{
		GAMEPAD_JogCon1(driver, VEH_TUMBLE_RUMBLE_WEAK, VEH_TUMBLE_RUMBLE_DURATION);
	}
}

enum
{
	VEH_WARP_DUST_SEGMENTS = 16,
	VEH_WARP_DUST_POINT_STRIDE_SHORTS = 4,
	VEH_WARP_DUST_RANDOM_MASK = 0xfff,
	VEH_WARP_DUST_RANDOM_SHIFT = FRACTIONAL_BITS,
	VEH_WARP_DUST_RECURSION_MIN_SPAN = 2,
	VEH_WARP_DUST_RADIUS_DECAY = 0xc00,
	VEH_WARP_DUST_SPAWN_TIMER_BIT = 1,
	VEH_WARP_DUST_JITTER_SCALE_SHIFT = 5,
	VEH_WARP_DUST_CAMERA_OFFSET_SHIFT = 10,
	VEH_WARP_DUST_POSITION_SHIFT = FRACTIONAL_BITS_8,
	VEH_WARP_DUST_RING_COUNT = 6,
	VEH_WARP_DUST_RING_ANGLE_SHIFT = 12,
	VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT = 5,
	VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT = 6,
	VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT = 8,
	VEH_WARP_DUST_MOVE_RADIUS = 0x100,
	VEH_WARP_DUST_WAVE_ANGLE_SHIFT = 7,
	VEH_WARP_DUST_WAVE_HEIGHT_SHIFT = 7,
	VEH_WARP_DUST_OT_DEPTH_SHIFT = 6,
	VEH_WARP_BEAM_HEIGHT_OFFSET = 0x100,
	VEH_WARP_DUST_ANGLE_STEP = 100,
	VEH_WARP_TIMER_STEP = 26,
	VEH_WARP_TIMER_MAX = 800,
	VEH_WARP_EXPAND_SCALE_SPEED = 120,
	VEH_WARP_EXPAND_SCALE_TARGET_XZ = 4800,
	VEH_WARP_LIFT_HEIGHT_LIMIT = 0x8000,
	VEH_WARP_LIFT_STEP = 0x800,
	VEH_WARP_LAUNCH_REV_STATE = 2,
	VEH_WARP_SHRINK_SCALE_SPEED_XZ = 600,
	VEH_WARP_SHRINK_SCALE_SPEED_Y = 3200,
	VEH_WARP_SHRINK_SCALE_TARGET_Y = 24000,
	VEH_WARP_FLARE_HEIGHT_OFFSET = 0x40,
	VEH_WARP_HEIGHT_OFFSET_STEP = 0x1800,
	VEH_WARP_TURN_WRAP_BIAS = 0x800,
	VEH_WARP_TURN_MASK = 0xfff,
	VEH_WARP_INITIAL_TIMER = 0x3c,
	VEH_WARP_SOUND_ID = 0x97,
	VEH_WARP_ENGINE_AUDIO_STRIDE = 4,
	VEH_WARP_POSITION_SHIFT = FRACTIONAL_BITS_8,
};

CTR_STATIC_ASSERT(sizeof(SVECTOR) == VEH_WARP_DUST_POINT_STRIDE_SHORTS * sizeof(s16));

static const u32 VEH_WARP_DUST_DRAW_MODE = 0xe1000a20u;
static const u32 VEH_WARP_DUST_POLY_G4_CODE = 0x3a000000u;
static const u32 VEH_WARP_DUST_EDGE_COLOR = 0x007f1f3fu;
static const u32 VEH_WARP_DUST_PACKET_TAG = 0x11000000u;

void VehStuckProc_Warp_MoveDustPuff(s16 *points, int span, int radius, s16 *jitterScale)
{
	register int radiusHalf CTR_PSX_REGISTER("$16");
	int jitterX = CTR_MipsSra(CTR_MipsMulLo(MixRNG_Scramble() & VEH_WARP_DUST_RANDOM_MASK, radius), VEH_WARP_DUST_RANDOM_SHIFT);
	int jitterY;
	int jitterZ;
	s16 *end;
	int halfSpan;
	s16 *mid;
	radiusHalf = CTR_MipsSra(radius, 1);
	if (jitterX < radiusHalf)
	{
		jitterX = CTR_MipsSubLo(jitterX, radius);
	}

	jitterY = CTR_MipsSra(CTR_MipsMulLo(MixRNG_Scramble() & VEH_WARP_DUST_RANDOM_MASK, radius), VEH_WARP_DUST_RANDOM_SHIFT);
	if (jitterY < radiusHalf)
	{
		jitterY = CTR_MipsSubLo(jitterY, radius);
	}

	jitterZ = CTR_MipsSra(CTR_MipsMulLo(MixRNG_Scramble() & VEH_WARP_DUST_RANDOM_MASK, radius), VEH_WARP_DUST_RANDOM_SHIFT);
	if (jitterZ < radiusHalf)
	{
		jitterZ = CTR_MipsSubLo(jitterZ, radius);
	}
	CTR_PSX_KEEP_VALUE(radiusHalf);

	end = points + span * VEH_WARP_DUST_POINT_STRIDE_SHORTS;
	halfSpan = CTR_MipsSra(span, 1);
	mid = points + halfSpan * VEH_WARP_DUST_POINT_STRIDE_SHORTS;

	mid[0] =
	    (s16)CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(points[0], end[0]), 1), CTR_MipsSra(CTR_MipsMulLo(jitterScale[0], jitterX), VEH_WARP_DUST_RANDOM_SHIFT));
	mid[1] =
	    (s16)CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(points[1], end[1]), 1), CTR_MipsSra(CTR_MipsMulLo(jitterScale[1], jitterY), VEH_WARP_DUST_RANDOM_SHIFT));
	mid[2] =
	    (s16)CTR_MipsAddLo(CTR_MipsSra(CTR_MipsAddLo(points[2], end[2]), 1), CTR_MipsSra(CTR_MipsMulLo(jitterScale[2], jitterZ), VEH_WARP_DUST_RANDOM_SHIFT));

	if (span > VEH_WARP_DUST_RECURSION_MIN_SPAN)
	{
		int nextRadius = CTR_MipsSra(radius * VEH_WARP_DUST_RADIUS_DECAY, VEH_WARP_DUST_RANDOM_SHIFT);
		VehStuckProc_Warp_MoveDustPuff(points, halfSpan, nextRadius, jitterScale);
		VehStuckProc_Warp_MoveDustPuff(mid, halfSpan, nextRadius, jitterScale);
	}
}


void VehStuckProc_Warp_AddDustPuff1(struct ScratchpadStruct *sps)
{
	struct GameTracker *gGT = GAME_TRACKER;
	struct Particle *p;

	// if even frame don't spawn
	if (gGT->timer & VEH_WARP_DUST_SPAWN_TIMER_BIT)
	{
		return;
	}

	p = Particle_Init(0, gGT->iconGroup[1], &data.emSet_Warppad[0]);

	if (p == NULL)
	{
		return;
	}

	// position variables
	p->axis[0].startVal = CTR_MipsAddLo(p->axis[0].startVal, CTR_MipsSll(sps->Input1.pos.x, FRACTIONAL_BITS_8));
	p->axis[1].startVal = CTR_MipsAddLo(p->axis[1].startVal, CTR_MipsSll(sps->Input1.pos.y, FRACTIONAL_BITS_8));
	p->axis[2].startVal = CTR_MipsAddLo(p->axis[2].startVal, CTR_MipsSll(sps->Input1.pos.z, FRACTIONAL_BITS_8));
}


// NOTE(aalhendi): Native builds use the C forms below, while the PSX forms pin
// the scratchpad, GTE, and packet operations to retail's instruction order.
typedef u16 VehWarpDustHalfword CTR_MAY_ALIAS;
typedef s16 VehWarpDustSignedHalfword CTR_MAY_ALIAS;
typedef u32 VehWarpDustWord CTR_MAY_ALIAS;

#define VEH_WARP_DUST_HALF(base, offset)        (*(VehWarpDustHalfword *)((u8 *)(base) + (offset)))
#define VEH_WARP_DUST_SIGNED_HALF(base, offset) (*(VehWarpDustSignedHalfword *)((u8 *)(base) + (offset)))
#define VEH_WARP_DUST_WORD(base, offset)        (*(VehWarpDustWord *)((u8 *)(base) + (offset)))

#if defined(CTR_NATIVE)
#define VehWarpDust_LoadMatrices(matrix) \
	do                                   \
	{                                    \
		gte_SetRotMatrix(matrix);        \
		gte_SetTransMatrix(matrix);      \
	} while (0)
#define VehWarpDust_LoadV3(left, point, right)  CTR_GteLoadSV3((const SVECTOR *)(left), (const SVECTOR *)(point), (const SVECTOR *)(right))
#define VehWarpDust_LoadFirstV3(scratch, point) VehWarpDust_LoadV3((scratch) + 0x88, (point), (scratch) + 0x90)
#define VehWarpDust_StoreProjection(output)          \
	do                                               \
	{                                                \
		VEH_WARP_DUST_WORD((output), 0) = MFC2(12);  \
		VEH_WARP_DUST_WORD((output), 4) = MFC2(13);  \
		VEH_WARP_DUST_WORD((output), 8) = MFC2(14);  \
		VEH_WARP_DUST_WORD((output), 12) = MFC2(17); \
	} while (0)
#define VehWarpDust_StoreInitialProjection(output, scratch) VehWarpDust_StoreProjection(output)
#define VehWarpDust_LinkPacket(ot, prim)                    CtrGpu_LinkPacket24((ot), (u32 *)(prim), (prim), VEH_WARP_DUST_PACKET_TAG)
#define VehWarpDust_SetupCameraOffsets(pb, scratch, cameraZ, offsetX, offsetY)                                                       \
	do                                                                                                                               \
	{                                                                                                                                \
		u32 cameraX = (u16)(pb)->matrix_CameraTranspose.m[0][0];                                                                     \
		u32 cameraY = (u16)(pb)->matrix_CameraTranspose.m[1][0];                                                                     \
		(cameraZ) = (u16)(pb)->matrix_CameraTranspose.m[2][0];                                                                       \
		VEH_WARP_DUST_HALF((scratch), 0xb8) =                                                                                        \
		    (u16)CTR_MipsSra(CTR_MipsAddLo((s16)cameraX, (pb)->matrix_CameraTranspose.m[0][1]), VEH_WARP_DUST_JITTER_SCALE_SHIFT);   \
		(offsetX) = (s32)(cameraX << 16) >> (16 + VEH_WARP_DUST_CAMERA_OFFSET_SHIFT);                                                \
		VEH_WARP_DUST_HALF((scratch), 0xba) =                                                                                        \
		    (u16)CTR_MipsSra(CTR_MipsAddLo((s16)cameraY, (pb)->matrix_CameraTranspose.m[1][1]), VEH_WARP_DUST_JITTER_SCALE_SHIFT);   \
		(offsetY) = (s32)(cameraY << 16) >> (16 + VEH_WARP_DUST_CAMERA_OFFSET_SHIFT);                                                \
		VEH_WARP_DUST_HALF((scratch), 0xbc) =                                                                                        \
		    (u16)CTR_MipsSra(CTR_MipsAddLo((s16)(cameraZ), (pb)->matrix_CameraTranspose.m[2][1]), VEH_WARP_DUST_JITTER_SCALE_SHIFT); \
	} while (0)
#define VehWarpDust_InitEndpoint(gGT, d, warp, scratch, cameraZ, prim, offsetZ)                                       \
	do                                                                                                                \
	{                                                                                                                 \
		(prim) = (u8 *)(gGT)->backBuffer->primMem.cursor;                                                             \
		(cameraZ) <<= 16;                                                                                             \
		(offsetZ) = (s32)(cameraZ) >> (16 + VEH_WARP_DUST_CAMERA_OFFSET_SHIFT);                                       \
		if (((d)->instSelf->flags & HIDE_MODEL) != 0)                                                                 \
		{                                                                                                             \
			VEH_WARP_DUST_HALF((scratch), 0x80) = (u16)CTR_MipsSra((d)->posCurr.x, VEH_WARP_DUST_POSITION_SHIFT);     \
			VEH_WARP_DUST_HALF((scratch), 0x82) = (u16)CTR_MipsSra((warp)->beamHeight, VEH_WARP_DUST_POSITION_SHIFT); \
			VEH_WARP_DUST_HALF((scratch), 0x84) = (u16)CTR_MipsSra((d)->posCurr.z, VEH_WARP_DUST_POSITION_SHIFT);     \
			VehStuckProc_Warp_AddDustPuff1((struct ScratchpadStruct *)((scratch) + 0x80));                            \
		}                                                                                                             \
	} while (0)
#define VehWarpDust_InitRings(ring, edgeColor)  \
	do                                          \
	{                                           \
		(ring) = 0;                             \
		(edgeColor) = VEH_WARP_DUST_EDGE_COLOR; \
	} while (0)
#define VehWarpDust_StartRing(ring, warp, baseAngle, sine)                                            \
	do                                                                                                \
	{                                                                                                 \
		(baseAngle) = CTR_MipsSll((ring), VEH_WARP_DUST_RING_ANGLE_SHIFT) / VEH_WARP_DUST_RING_COUNT; \
		(sine) = MATH_Sin(CTR_MipsAddLo((baseAngle), (warp)->dustAngle));                             \
	} while (0)
#define VehWarpDust_NextRing(ring, nextRing, repeat)      \
	do                                                    \
	{                                                     \
		(nextRing) = (ring) + 1;                          \
		(repeat) = (nextRing) < VEH_WARP_DUST_RING_COUNT; \
	} while (0)
#define VehWarpDust_SetupFirstProjection(scratch, point, previous, current, offsetX, offsetY, offsetZ)       \
	do                                                                                                       \
	{                                                                                                        \
		(point) = (scratch);                                                                                 \
		(previous) = (scratch) + 0x98;                                                                       \
		(current) = (scratch) + 0xa8;                                                                        \
		VEH_WARP_DUST_HALF((scratch), 0x88) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF((point), 0), (offsetX)); \
		VEH_WARP_DUST_HALF((scratch), 0x8a) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF((point), 2), (offsetY)); \
		VEH_WARP_DUST_HALF((scratch), 0x8c) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF((point), 4), (offsetZ)); \
		VEH_WARP_DUST_HALF((scratch), 0x90) = (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF((point), 0), (offsetX)); \
		VEH_WARP_DUST_HALF((scratch), 0x92) = (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF((point), 2), (offsetY)); \
		VEH_WARP_DUST_HALF((scratch), 0x94) = (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF((point), 4), (offsetZ)); \
	} while (0)
#define VehWarpDust_InitSegments(segment, packetEnd, prim, scratch) \
	do                                                              \
	{                                                               \
		(segment) = 0;                                              \
		(packetEnd) = (prim) + 68;                                  \
	} while (0)
#define VehWarpDust_ProjectNext(scratch, point, previous, current, offsetX, offsetY, offsetZ)                \
	do                                                                                                       \
	{                                                                                                        \
		u8 *swapProjection;                                                                                  \
		(point) += 8;                                                                                        \
		VEH_WARP_DUST_HALF((scratch), 0x88) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF((point), 0), (offsetX)); \
		VEH_WARP_DUST_HALF((scratch), 0x8a) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF((point), 2), (offsetY)); \
		VEH_WARP_DUST_HALF((scratch), 0x8c) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF((point), 4), (offsetZ)); \
		VEH_WARP_DUST_HALF((scratch), 0x90) = (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF((point), 0), (offsetX)); \
		VEH_WARP_DUST_HALF((scratch), 0x92) = (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF((point), 2), (offsetY)); \
		VEH_WARP_DUST_HALF((scratch), 0x94) = (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF((point), 4), (offsetZ)); \
		swapProjection = (current);                                                                          \
		(current) = (previous);                                                                              \
		(previous) = swapProjection;                                                                         \
	} while (0)
#define VehWarpDust_LoadNextV3(scratch, point) VehWarpDust_LoadV3((scratch) + 0x88, (point), (scratch) + 0x90)
#define VehWarpDust_EmitNextSegment(packetEnd, previous, current, edgeColor, pb, prim, segment)                \
	do                                                                                                         \
	{                                                                                                          \
		u32 *ot;                                                                                               \
		VEH_WARP_DUST_WORD((packetEnd), -64) = VEH_WARP_DUST_DRAW_MODE;                                        \
		VEH_WARP_DUST_WORD((packetEnd), -60) = VEH_WARP_DUST_POLY_G4_CODE;                                     \
		VEH_WARP_DUST_WORD((packetEnd), -56) = VEH_WARP_DUST_WORD((previous), 0);                              \
		VEH_WARP_DUST_WORD((packetEnd), -52) = (edgeColor);                                                    \
		VEH_WARP_DUST_WORD((packetEnd), -48) = VEH_WARP_DUST_WORD((previous), 4);                              \
		VEH_WARP_DUST_WORD((packetEnd), -44) = 0;                                                              \
		VEH_WARP_DUST_WORD((packetEnd), -40) = VEH_WARP_DUST_WORD((current), 0);                               \
		VEH_WARP_DUST_WORD((packetEnd), -36) = (edgeColor);                                                    \
		VEH_WARP_DUST_WORD((packetEnd), -32) = VEH_WARP_DUST_WORD((current), 4);                               \
		VEH_WARP_DUST_WORD((packetEnd), -28) = VEH_WARP_DUST_POLY_G4_CODE;                                     \
		VEH_WARP_DUST_WORD((packetEnd), -24) = VEH_WARP_DUST_WORD((previous), 8);                              \
		VEH_WARP_DUST_WORD((packetEnd), -20) = (edgeColor);                                                    \
		VEH_WARP_DUST_WORD((packetEnd), -16) = VEH_WARP_DUST_WORD((previous), 4);                              \
		VEH_WARP_DUST_WORD((packetEnd), -12) = 0;                                                              \
		VEH_WARP_DUST_WORD((packetEnd), -8) = VEH_WARP_DUST_WORD((current), 8);                                \
		VEH_WARP_DUST_WORD((packetEnd), -4) = (edgeColor);                                                     \
		VEH_WARP_DUST_WORD((packetEnd), 0) = VEH_WARP_DUST_WORD((current), 4);                                 \
		ot = (pb)->ptrOT + CTR_MipsSra((s32)VEH_WARP_DUST_WORD((previous), 12), VEH_WARP_DUST_OT_DEPTH_SHIFT); \
		VehWarpDust_LinkPacket(ot, prim);                                                                      \
		(segment)++;                                                                                           \
		(packetEnd) += 72;                                                                                     \
	} while (0)
#else
// NOTE(aalhendi): The two encoded JALs preserve retail's occupied delay slots;
// their targets are fixed by the retail EXE address map used by this build.
#define VehWarpDust_LoadMatrices(matrix)                                                                                                                    \
	__asm__ volatile("lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\tlw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\tctc2 $12,$2\n\tctc2 " \
	                 "$13,$3\n\tctc2 $14,$4\n\tlw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\tctc2 $13,$6\n\tctc2 $14,$7"                \
	                 :                                                                                                                                      \
		                 : "r"(matrix), "m"(*(const MATRIX *)(matrix))                                                                                          \
	                 : "$12", "$13", "$14")
#define VehWarpDust_LoadV3(left, point, right)                                                                            \
	__asm__ volatile("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)\n\tlwc2 $2,0(%1)\n\tlwc2 $3,4(%1)\n\tlwc2 $4,0(%2)\n\tlwc2 $5,4(%2)" \
	                 :                                                                                                    \
	                 : "r"(left), "r"(point), "r"(right)                                                                  \
	                 : "memory")
#define VehWarpDust_LoadFirstV3(scratch, point) \
	__asm__ volatile("addiu $2,%0,136\n\t"      \
	                 "sh $3,148(%0)\n\t"        \
	                 "lwc2 $0,0($2)\n\t"        \
	                 "lwc2 $1,4($2)\n\t"        \
	                 "lwc2 $2,0(%1)\n\t"        \
	                 "lwc2 $3,4(%1)\n\t"        \
	                 "addiu $2,%0,144\n\t"      \
	                 "lwc2 $4,0($2)\n\t"        \
	                 "lwc2 $5,4($2)"            \
	                 :                          \
	                 : "r"(scratch), "r"(point) \
	                 : "$2", "memory")
#define VehWarpDust_StoreProjection(output) \
	__asm__ volatile("swc2 $12,0(%0)\n\tswc2 $13,4(%0)\n\tswc2 $14,8(%0)\n\taddiu $2,%0,12\n\tswc2 $17,0($2)" : : "r"(output) : "$2", "memory")
#define VehWarpDust_StoreInitialProjection(output, scratch) \
	__asm__ volatile("swc2 $12,0(%0)\n\tswc2 $13,4(%0)\n\tswc2 $14,8(%0)\n\taddiu $2,%1,164\n\tswc2 $17,0($2)" : : "r"(output), "r"(scratch) : "$2", "memory")
#define VehWarpDust_LinkPacket(ot, prim)                                  \
	do                                                                    \
	{                                                                     \
		VEH_WARP_DUST_WORD((prim), 0) = *(ot) | VEH_WARP_DUST_PACKET_TAG; \
		*(ot) = ((u32)(prim) << 8) >> 8;                                  \
	} while (0)
#define VehWarpDust_SetupCameraOffsets(pb, scratch, cameraZ, offsetX, offsetY) \
	__asm__ volatile("lhu $4,72(%3)\n\t"                                       \
	                 "lh $3,74(%3)\n\t"                                        \
	                 "lhu $5,78(%3)\n\t"                                       \
	                 "lhu %0,84(%3)\n\t"                                       \
	                 "sll $4,$4,16\n\t"                                        \
	                 "sra $2,$4,16\n\t"                                        \
	                 "addu $2,$2,$3\n\t"                                       \
	                 "sra $2,$2,5\n\t"                                         \
	                 "sra %1,$4,26\n\t"                                        \
	                 "sh $2,184(%4)\n\t"                                       \
	                 "lh $2,78(%3)\n\t"                                        \
	                 "lh $3,80(%3)\n\t"                                        \
	                 "sll $5,$5,16\n\t"                                        \
	                 "addu $2,$2,$3\n\t"                                       \
	                 "sra $2,$2,5\n\t"                                         \
	                 "sh $2,186(%4)\n\t"                                       \
	                 "lh $2,84(%3)\n\t"                                        \
	                 "lh $3,86(%3)\n\t"                                        \
	                 "sra %2,$5,26\n\t"                                        \
	                 "addu $2,$2,$3\n\t"                                       \
	                 "sra $2,$2,5\n\t"                                         \
	                 "sh $2,188(%4)"                                           \
	                 : "=r"(cameraZ), "=r"(offsetX), "=r"(offsetY)             \
	                 : "r"(pb), "r"(scratch)                                   \
	                 : "$2", "$3", "$4", "$5", "memory")
#define VehWarpDust_InitEndpoint(gGT, d, warp, scratch, cameraZ, prim, offsetZ) \
	__asm__ volatile(".set noreorder\n\t"                                       \
	                 "lw $2,16(%2)\n\t"                                         \
	                 "lw $10,64($sp)\n\t"                                       \
	                 "lw %0,128($2)\n\t"                                        \
	                 "lw $2,28($10)\n\t"                                        \
	                 "sll %3,%3,16\n\t"                                         \
	                 "lw $2,40($2)\n\t"                                         \
	                 "nop\n\t"                                                  \
	                 "andi $2,$2,0x80\n\t"                                      \
	                 "beqz $2,1f\n\t"                                           \
	                 "sra %1,%3,26\n\t"                                         \
	                 "lw $2,724($10)\n\t"                                       \
	                 "nop\n\t"                                                  \
	                 "sra $2,$2,8\n\t"                                          \
	                 "sh $2,128(%4)\n\t"                                        \
	                 "lw $10,68($sp)\n\t"                                       \
	                 "nop\n\t"                                                  \
	                 "lw $2,16($10)\n\t"                                        \
	                 "nop\n\t"                                                  \
	                 "sra $2,$2,8\n\t"                                          \
	                 "sh $2,130(%4)\n\t"                                        \
	                 "lw $10,64($sp)\n\t"                                       \
	                 "lui $4,0x1f80\n\t"                                        \
	                 "lw $2,732($10)\n\t"                                       \
	                 "ori $4,$4,0x188\n\t"                                      \
	                 "sra $2,$2,8\n\t"                                          \
	                 ".word 0x0c01a16c\n\t"                                     \
	                 "sh $2,132(%4)\n\t"                                        \
	                 "1:\n\t"                                                   \
	                 ".set noreorder"                                           \
	                 : "=r"(prim), "=r"(offsetZ)                                \
	                 : "r"(gGT), "r"(cameraZ), "r"(scratch)                     \
	                 : "$2", "$4", "$31", "memory")
#define VehWarpDust_InitRings(ring, edgeColor)     \
	__asm__ volatile("sw $0,%1\n\t"                \
	                 "lui %0,0x7f\n\t"             \
	                 "ori %0,%0,0x1f3f"            \
	                 : "=r"(edgeColor), "=m"(ring) \
	                 :                             \
	                 : "memory")
#define VehWarpDust_StartRing(ring, warp, baseAngle, sine) \
	__asm__ volatile("lui $3,0x2aaa\n\t"                   \
	                 "lw $10,16($sp)\n\t"                  \
	                 "ori $3,$3,0xaaab\n\t"                \
	                 "sll $2,$10,12\n\t"                   \
	                 "mult $2,$3\n\t"                      \
	                 "lw $10,68($sp)\n\t"                  \
	                 "sra $2,$2,31\n\t"                    \
	                 "lw $4,12($10)\n\t"                   \
	                 "mfhi $10\n\t"                        \
	                 "subu %0,$10,$2\n\t"                  \
	                 ".word 0x0c00f461\n\t"                \
	                 "addu $4,%0,$4"                       \
	                 : "=r"(baseAngle), "=r"(sine)         \
	                 :                                     \
	                 : "memory")
#define VehWarpDust_NextRing(ring, nextRing, repeat) \
	__asm__ volatile("lw %0,%2\n\t"                  \
	                 "nop\n\t"                       \
	                 "addiu %0,%0,1\n\t"             \
	                 "slti %1,%0,6"                  \
	                 : "=r"(nextRing), "=r"(repeat)  \
	                 : "m"(ring))
#define VehWarpDust_SetupFirstProjection(scratch, point, previous, current, offsetX, offsetY, offsetZ) \
	__asm__ volatile("move %0,%3\n\t"                                                                  \
	                 "addiu %1,%3,152\n\t"                                                             \
	                 "addiu %2,%3,168\n\t"                                                             \
	                 "lhu $2,0(%3)\n\t"                                                                \
	                 "lhu $3,2(%3)\n\t"                                                                \
	                 "addu $2,$2,%4\n\t"                                                               \
	                 "sh $2,136(%3)\n\t"                                                               \
	                 "lhu $2,4(%3)\n\t"                                                                \
	                 "addu $3,$3,%5\n\t"                                                               \
	                 "sh $3,138(%3)\n\t"                                                               \
	                 "lhu $3,0(%3)\n\t"                                                                \
	                 "addu $2,$2,%6\n\t"                                                               \
	                 "sh $2,140(%3)\n\t"                                                               \
	                 "lhu $2,2(%3)\n\t"                                                                \
	                 "subu $3,$3,%4\n\t"                                                               \
	                 "sh $3,144(%3)\n\t"                                                               \
	                 "lhu $3,4(%3)\n\t"                                                                \
	                 "subu $2,$2,%5\n\t"                                                               \
	                 "subu $3,$3,%6\n\t"                                                               \
	                 "sh $2,146(%3)"                                                                   \
	                 : "=r"(point), "=r"(previous), "=r"(current)                                      \
	                 : "r"(scratch), "r"(offsetX), "r"(offsetY), "r"(offsetZ)                          \
	                 : "$2", "$3", "memory")
#define VehWarpDust_InitSegments(segment, packetEnd, prim, scratch) \
	__asm__ volatile("move %0,$0\n\t"                               \
	                 "addiu %1,%2,68\n\t"                           \
	                 "addiu $7,%3,4"                                \
	                 : "=r"(segment), "=r"(packetEnd)               \
	                 : "r"(prim), "r"(scratch))
#define VehWarpDust_ProjectNext(scratch, point, previous, current, offsetX, offsetY, offsetZ) \
	__asm__ volatile("addiu %0,%0,8\n\t"                                                      \
	                 "lhu $2,0(%0)\n\t"                                                       \
	                 "addiu $7,$7,8\n\t"                                                      \
	                 "addu $2,$2,%4\n\t"                                                      \
	                 "sh $2,136(%3)\n\t"                                                      \
	                 "lhu $2,-2($7)\n\t"                                                      \
	                 "nop\n\t"                                                                \
	                 "addu $2,$2,%5\n\t"                                                      \
	                 "sh $2,138(%3)\n\t"                                                      \
	                 "lhu $2,0($7)\n\t"                                                       \
	                 "nop\n\t"                                                                \
	                 "addu $2,$2,%6\n\t"                                                      \
	                 "sh $2,140(%3)\n\t"                                                      \
	                 "lhu $2,0(%0)\n\t"                                                       \
	                 "move $3,%2\n\t"                                                         \
	                 "subu $2,$2,%4\n\t"                                                      \
	                 "sh $2,144(%3)\n\t"                                                      \
	                 "lhu $2,-2($7)\n\t"                                                      \
	                 "move %2,%1\n\t"                                                         \
	                 "subu $2,$2,%5\n\t"                                                      \
	                 "sh $2,146(%3)\n\t"                                                      \
	                 "lhu $2,0($7)\n\t"                                                       \
	                 "move %1,$3\n\t"                                                         \
	                 "subu $2,$2,%6\n\t"                                                      \
	                 "sh $2,148(%3)"                                                          \
	                 : "+r"(point), "+r"(previous), "+r"(current)                             \
	                 : "r"(scratch), "r"(offsetX), "r"(offsetY), "r"(offsetZ)                 \
	                 : "$2", "$3", "memory")
#define VehWarpDust_LoadNextV3(scratch, point)  \
	__asm__ volatile("addiu $2,%0,136\n\t"      \
	                 "lwc2 $0,0($2)\n\t"        \
	                 "lwc2 $1,4($2)\n\t"        \
	                 "lwc2 $2,0(%1)\n\t"        \
	                 "lwc2 $3,4(%1)\n\t"        \
	                 "addiu $2,%0,144\n\t"      \
	                 "lwc2 $4,0($2)\n\t"        \
	                 "lwc2 $5,4($2)"            \
	                 :                          \
	                 : "r"(scratch), "r"(point) \
	                 : "$2", "memory")
#define VehWarpDust_EmitNextSegment(packetEnd, previous, current, edgeColor, pb, prim, segment) \
	__asm__ volatile("lui $2,0xe100\n\t"                                                        \
	                 "ori $2,$2,0x0a20\n\t"                                                     \
	                 "lui $10,0x3a00\n\t"                                                       \
	                 "sw $2,-64(%0)\n\t"                                                        \
	                 "sw $10,-60(%0)\n\t"                                                       \
	                 "sw %4,-52(%0)\n\t"                                                        \
	                 "sw $0,-44(%0)\n\t"                                                        \
	                 "sw %4,-36(%0)\n\t"                                                        \
	                 "lw $2,0(%2)\n\t"                                                          \
	                 "nop\n\t"                                                                  \
	                 "sw $2,-56(%0)\n\t"                                                        \
	                 "lw $2,4(%2)\n\t"                                                          \
	                 "nop\n\t"                                                                  \
	                 "sw $2,-48(%0)\n\t"                                                        \
	                 "lw $2,0(%3)\n\t"                                                          \
	                 "nop\n\t"                                                                  \
	                 "sw $2,-40(%0)\n\t"                                                        \
	                 "lw $2,4(%3)\n\t"                                                          \
	                 "sw $10,-28(%0)\n\t"                                                       \
	                 "sw %4,-20(%0)\n\t"                                                        \
	                 "sw $0,-12(%0)\n\t"                                                        \
	                 "sw %4,-4(%0)\n\t"                                                         \
	                 "sw $2,-32(%0)\n\t"                                                        \
	                 "lw $2,8(%2)\n\t"                                                          \
	                 "nop\n\t"                                                                  \
	                 "sw $2,-24(%0)\n\t"                                                        \
	                 "lw $2,4(%2)\n\t"                                                          \
	                 "nop\n\t"                                                                  \
	                 "sw $2,-16(%0)\n\t"                                                        \
	                 "lw $2,8(%3)\n\t"                                                          \
	                 "addiu %1,%1,1\n\t"                                                        \
	                 "sw $2,-8(%0)\n\t"                                                         \
	                 "lw $2,4(%3)\n\t"                                                          \
	                 "lui $3,0x1100\n\t"                                                        \
	                 "sw $2,0(%0)\n\t"                                                          \
	                 "lw $2,12(%2)\n\t"                                                         \
	                 "lw $4,244(%5)\n\t"                                                        \
	                 "sra $2,$2,6\n\t"                                                          \
	                 "sll $2,$2,2\n\t"                                                          \
	                 "addu $4,$4,$2\n\t"                                                        \
	                 "lw $2,0($4)\n\t"                                                          \
	                 "addiu %0,%0,72\n\t"                                                       \
	                 "or $2,$2,$3\n\t"                                                          \
	                 "sw $2,0(%6)\n\t"                                                          \
	                 "sll $2,%6,8\n\t"                                                          \
	                 "srl $2,$2,8\n\t"                                                          \
	                 "sw $2,0($4)"                                                              \
	                 : "+r"(packetEnd), "+r"(segment)                                           \
	                 : "r"(previous), "r"(current), "r"(edgeColor), "r"(pb), "r"(prim)          \
	                 : "$2", "$3", "$4", "memory")
#endif

void VehStuckProc_Warp_AddDustPuff2(struct Driver *d, struct DriverWarpState *warp)
{
	register u8 *scratch CTR_PSX_REGISTER("$17") = CTR_SCRATCHPAD_PTR(u8, 0x108);
	register struct PushBuffer *pb CTR_PSX_REGISTER("$21");
	register u8 *prim CTR_PSX_REGISTER("$19");
	s32 offsetX;
	register s32 offsetY CTR_PSX_REGISTER("$23");
	register s32 offsetZ CTR_PSX_REGISTER("$22");
	register u32 edgeColor CTR_PSX_REGISTER("$20");
	register s32 baseAngle CTR_PSX_REGISTER("$16");
	register struct GameTracker *gGT CTR_PSX_REGISTER("$7");
	register u32 cameraZ CTR_PSX_REGISTER("$6");
	int ring;
	int repeatRing;

	gGT = GAME_TRACKER;
	{
		register u32 driverID CTR_PSX_REGISTER("$3") = d->driverID;
		register size_t pushBufferOffset CTR_PSX_REGISTER("$2");

		pushBufferOffset = CTR_MipsSll(driverID, 4);
		pushBufferOffset = CTR_MipsAddLo(pushBufferOffset, driverID);
		pushBufferOffset = CTR_MipsSll(pushBufferOffset, 4);
		pushBufferOffset = CTR_MipsAddLo(pushBufferOffset, offsetof(struct GameTracker, pushBuffer));
		CTR_PSX_OBSERVE_VALUE(driverID);
		CTR_PSX_OBSERVE_VALUE(pushBufferOffset);
		pb = (struct PushBuffer *)((u8 *)gGT + pushBufferOffset);
	}
	VehWarpDust_LoadMatrices(&pb->matrix_ViewProj);

	VehWarpDust_SetupCameraOffsets(pb, scratch, cameraZ, offsetX, offsetY);
	CTR_PSX_KEEP_VALUE(offsetX);
	CTR_PSX_KEEP_VALUE(offsetY);

	VehWarpDust_InitEndpoint(gGT, d, warp, scratch, cameraZ, prim, offsetZ);

	VehWarpDust_InitRings(ring, edgeColor);
	do
	{
		register int waveIndex CTR_PSX_REGISTER("$18");
		register u8 *pointCursor CTR_PSX_REGISTER("$16");
		register u8 *point CTR_PSX_REGISTER("$9");
		register u8 *previousProjection CTR_PSX_REGISTER("$6");
		register u8 *currentProjection CTR_PSX_REGISTER("$8");
		register u8 *packetEnd CTR_PSX_REGISTER("$5");
		register int segment CTR_PSX_REGISTER("$18");

		{
			register int sine CTR_PSX_REGISTER("$2");

			VehWarpDust_StartRing(ring, warp, baseAngle, sine);

			VEH_WARP_DUST_SIGNED_HALF(scratch, 0x00) =
			    (s16)CTR_MipsSubLo(CTR_MipsSra(d->posCurr.x, VEH_WARP_DUST_POSITION_SHIFT), CTR_MipsSra(sine, VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT));
		}
		VEH_WARP_DUST_HALF(scratch, 0x02) = (u16)CTR_MipsSra(warp->quadHeight, VEH_WARP_DUST_POSITION_SHIFT);
		{
			int cosine = MATH_Cos(CTR_MipsAddLo(baseAngle, warp->dustAngle));

			VEH_WARP_DUST_SIGNED_HALF(scratch, 0x04) =
			    (s16)CTR_MipsSubLo(CTR_MipsSra(d->posCurr.z, VEH_WARP_DUST_POSITION_SHIFT), CTR_MipsSra(cosine, VEH_WARP_DUST_HIDDEN_BASE_OFFSET_SHIFT));
		}

		VEH_WARP_DUST_HALF(scratch, 0x80) = (u16)CTR_MipsSra(d->posCurr.x, VEH_WARP_DUST_POSITION_SHIFT);
		VEH_WARP_DUST_HALF(scratch, 0x82) = (u16)CTR_MipsSra(warp->beamHeight, VEH_WARP_DUST_POSITION_SHIFT);
		VEH_WARP_DUST_HALF(scratch, 0x84) = (u16)CTR_MipsSra(d->posCurr.z, VEH_WARP_DUST_POSITION_SHIFT);

		if ((d->instSelf->flags & HIDE_MODEL) == 0)
		{
			int trigValue = MATH_Sin(CTR_MipsAddLo(baseAngle, warp->dustAngle));

			VEH_WARP_DUST_HALF(scratch, 0x00) =
			    (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF(scratch, 0x00), CTR_MipsSra(trigValue, VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT));
			trigValue = MATH_Cos(CTR_MipsAddLo(baseAngle, warp->dustAngle));
			VEH_WARP_DUST_HALF(scratch, 0x04) =
			    (u16)CTR_MipsSubLo(VEH_WARP_DUST_HALF(scratch, 0x04), CTR_MipsSra(trigValue, VEH_WARP_DUST_VISIBLE_BASE_OFFSET_SHIFT));
			trigValue = MATH_Sin(CTR_MipsAddLo(baseAngle, warp->dustAngle));
			VEH_WARP_DUST_HALF(scratch, 0x80) =
			    (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF(scratch, 0x80), CTR_MipsSra(trigValue, VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT));
			trigValue = MATH_Cos(CTR_MipsAddLo(baseAngle, warp->dustAngle));
			VEH_WARP_DUST_HALF(scratch, 0x84) =
			    (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF(scratch, 0x84), CTR_MipsSra(trigValue, VEH_WARP_DUST_VISIBLE_ENDPOINT_OFFSET_SHIFT));
		}
		else
		{
			VehStuckProc_Warp_AddDustPuff1((struct ScratchpadStruct *)scratch);
		}

		VehStuckProc_Warp_MoveDustPuff((s16 *)scratch, VEH_WARP_DUST_SEGMENTS, VEH_WARP_DUST_MOVE_RADIUS, (s16 *)(scratch + 0xb8));

		waveIndex = 1;
		pointCursor = scratch + 8;
		do
		{
			int waveHeight = CTR_MipsSra(MATH_Sin(CTR_MipsSll(waveIndex, VEH_WARP_DUST_WAVE_ANGLE_SHIFT)), VEH_WARP_DUST_WAVE_HEIGHT_SHIFT);

			VEH_WARP_DUST_HALF(pointCursor, 2) = (u16)CTR_MipsAddLo(VEH_WARP_DUST_HALF(pointCursor, 2), waveHeight);
			waveIndex++;
			pointCursor += 8;
		} while (waveIndex < VEH_WARP_DUST_SEGMENTS);

		VehWarpDust_SetupFirstProjection(scratch, point, previousProjection, currentProjection, offsetX, offsetY, offsetZ);
		VehWarpDust_LoadFirstV3(scratch, point);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rtpt_b();
		VehWarpDust_StoreInitialProjection(previousProjection, scratch);

		VehWarpDust_InitSegments(segment, packetEnd, prim, scratch);
		do
		{
			VehWarpDust_ProjectNext(scratch, point, previousProjection, currentProjection, offsetX, offsetY, offsetZ);
			VehWarpDust_LoadNextV3(scratch, point);
			CTR_PSX_GTE_PIPELINE_DELAY();
			gte_rtpt_b();
			VehWarpDust_StoreProjection(previousProjection);

			VehWarpDust_EmitNextSegment(packetEnd, previousProjection, currentProjection, edgeColor, pb, prim, segment);
			prim += 72;
		} while (segment < VEH_WARP_DUST_SEGMENTS);

		{
			int nextRing;

			VehWarpDust_NextRing(ring, nextRing, repeatRing);
			ring = nextRing;
		}
	} while (repeatRing != 0);

	GAME_TRACKER->backBuffer->primMem.cursor = (u32 *)prim;
}


void VehStuckProc_Warp_PhysAngular(struct Thread *th, struct Driver *d)
{
	SVec4 flarePos;
	struct Instance *inst;
	struct DriverWarpState *warp;
	s32 wrappedTurnAngle;

	(void)th;
	inst = d->instSelf;
	warp = &d->KartStates.Warp;

	if ((inst->flags & HIDE_MODEL) == 0)
	{
		register s32 quadHeight CTR_PSX_REGISTER("$3") = warp->quadHeight;
		int beamHeight = CTR_MipsAddLo(d->posCurr.y, VEH_WARP_BEAM_HEIGHT_OFFSET);

		warp->beamHeight = beamHeight;
		if (beamHeight < quadHeight)
		{
			warp->beamHeight = quadHeight;
		}

		if ((inst->flags & HIDE_MODEL) == 0)
		{
			warp->dustAngle = CTR_MipsSubLo(warp->dustAngle, VEH_WARP_DUST_ANGLE_STEP);
		}

		VehStuckProc_Warp_AddDustPuff2(d, warp);
	}

	warp->timer = CTR_MipsAddLo(warp->timer, VEH_WARP_TIMER_STEP);

	if (VEH_WARP_TIMER_MAX < warp->timer)
	{
		warp->timer = VEH_WARP_TIMER_MAX;
		d->revEngineState = VEH_WARP_LAUNCH_REV_STATE;

		inst->scale.x = VehCalc_InterpBySpeed(inst->scale.x, VEH_WARP_SHRINK_SCALE_SPEED_XZ, 0);
		inst->scale.y = VehCalc_InterpBySpeed(inst->scale.y, VEH_WARP_SHRINK_SCALE_SPEED_Y, VEH_WARP_SHRINK_SCALE_TARGET_Y);
		inst->scale.z = VehCalc_InterpBySpeed(inst->scale.z, VEH_WARP_SHRINK_SCALE_SPEED_XZ, 0);

		if (inst->scale.x == 0)
		{
			if ((inst->flags & HIDE_MODEL) == 0)
			{
				flarePos.x = (s16)CTR_MipsSra(d->posCurr.x, VEH_WARP_POSITION_SHIFT);
				flarePos.y = (s16)CTR_MipsAddLo(CTR_MipsSra(warp->quadHeight, VEH_WARP_POSITION_SHIFT), VEH_WARP_FLARE_HEIGHT_OFFSET);
				flarePos.z = (s16)CTR_MipsSra(d->posCurr.z, VEH_WARP_POSITION_SHIFT);

				FLARE_Init(CTR_VECTOR_DATA(&flarePos));
			}

			inst->flags |= HIDE_MODEL;
		}
		else
		{
			warp->heightOffset = CTR_MipsSubLo(warp->heightOffset, VEH_WARP_HEIGHT_OFFSET_STEP);
			d->posCurr.y = CTR_MipsAddLo(d->posCurr.y, warp->heightOffset);
		}
	}
	else
	{
		inst->scale.x = VehCalc_InterpBySpeed(inst->scale.x, VEH_WARP_EXPAND_SCALE_SPEED, VEH_WARP_EXPAND_SCALE_TARGET_XZ);
		inst->scale.y = VehCalc_InterpBySpeed(inst->scale.y, VEH_WARP_EXPAND_SCALE_SPEED, VEH_WARP_EXPAND_SCALE_TARGET_XZ >> 1);
		inst->scale.z = VehCalc_InterpBySpeed(inst->scale.z, VEH_WARP_EXPAND_SCALE_SPEED, VEH_WARP_EXPAND_SCALE_TARGET_XZ);

		if (d->posCurr.y < CTR_MipsAddLo(d->quadBlockHeight, VEH_WARP_LIFT_HEIGHT_LIMIT))
		{
			d->posCurr.y = CTR_MipsAddLo(d->posCurr.y, VEH_WARP_LIFT_STEP);
		}
	}

	{
		register s32 cameraAngle CTR_PSX_REGISTER("$4") = (u16)d->angle;

		wrappedTurnAngle = CTR_MipsSubLo(CTR_MipsAddLo(CTR_MipsAddLo((u16)d->turnAngleCurr, (u16)warp->timer), VEH_WARP_TURN_WRAP_BIAS) & VEH_WARP_TURN_MASK,
		                                 VEH_WARP_TURN_WRAP_BIAS);
		cameraAngle = CTR_MipsAddLo(cameraAngle, wrappedTurnAngle);
		d->turnAngleCurr = (s16)wrappedTurnAngle;
		d->rotCurr.y = (s16)CTR_MipsAddLo((u16)d->turnWobbleAngle, cameraAngle);
	}
	d->actionsFlagSet |= ACTION_WARP;
}


void VehStuckProc_Warp_Init(struct Thread *th, struct Driver *d)
{
	int engine;
	struct Instance *inst;
	struct DriverWarpState *warp;

	(void)th;
	if (d->kartState == KS_WARP_PAD)
	{
		return;
	}

	// If you are not in a warp pad

	warp = &d->KartStates.Warp;
	warp->timer = VEH_WARP_INITIAL_TIMER;
	warp->heightOffset = 0;
	warp->quadHeight = d->quadBlockHeight;

	// Warp sound?
	OtherFX_Play(VEH_WARP_SOUND_ID, 1);

	OtherFX_Stop1((int)d->driverAudioPtrs[1]);
	d->driverAudioPtrs[1] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[2]);
	d->driverAudioPtrs[2] = 0;
	OtherFX_Stop1((int)d->driverAudioPtrs[0]);
	d->driverAudioPtrs[0] = 0;

	engine = GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[d->driverID]].engineID;

	EngineAudio_Stop((u16)((engine * VEH_WARP_ENGINE_AUDIO_STRIDE) + d->driverID));

	// driver -> instSelf
	inst = d->instSelf;

	// instance flags, now reflective
	inst->flags |= REFLECTIVE;

	// vertical line for split or reflection
	inst->vertSplit = (s16)CTR_MipsSra(d->quadBlockHeight, VEH_WARP_POSITION_SHIFT);

	// CameraDC, freecam mode
	GAME_TRACKER->cameraDC[d->driverID].cameraMode = CAMERA_MODE_FREECAM;

	d->funcPtrs[DRIVER_FUNC_AUDIO] = VehPhysProc_Driving_Audio;
	d->funcPtrs[DRIVER_FUNC_PHYS_ANGULAR] = VehStuckProc_Warp_PhysAngular;
	d->funcPtrs[DRIVER_FUNC_TRANSLATE_MATRIX] = VehPhysForce_TranslateMatrix;
	d->funcPtrs[DRIVER_FUNC_ANIMATE] = VehFrameProc_Driving;

	// you are now in a warp pad
	d->kartState = KS_WARP_PAD;

	d->speed = 0;
	d->speedApprox = 0;

	d->funcPtrs[DRIVER_FUNC_INIT] = NULL;
	d->funcPtrs[DRIVER_FUNC_UPDATE] = NULL;
	d->funcPtrs[DRIVER_FUNC_PHYS_LINEAR] = NULL;
	d->funcPtrs[DRIVER_FUNC_APPLY_FORCES] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_MOVED] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLLIDE_DRIVERS] = NULL;
	d->funcPtrs[DRIVER_FUNC_COLL_FIXED] = NULL;
	d->funcPtrs[DRIVER_FUNC_JUMP_FRICTION] = NULL;
	d->funcPtrs[DRIVER_FUNC_PARTICLES] = VehEmitter_DriverMain;

	// driver is warping
	d->actionsFlagSet |= ACTION_WARP;
}

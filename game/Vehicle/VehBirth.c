#include "VehCommon.h"
#include <custom-characters/character_resources.h>

enum
{
	VEH_BIRTH_ADV_RETURN_LEVEL_COUNT = 0x14,
	VEH_BIRTH_DOOR_FORWARD_OFFSET = 800,
	VEH_BIRTH_DOOR_SIDE_OFFSET = 0x200,
	VEH_BIRTH_DOOR_Y_OFFSET = 0x17a,
	VEH_BIRTH_DOOR_EXIT_SPEED = 0xa00,
	VEH_BIRTH_DRIVER_INSTANCE_SCALE = 0xccc,
	VEH_BIRTH_CHEAT_WUMPA_COUNT = 99,
	VEH_BIRTH_CHEAT_ITEM_TURBO = HELD_ITEM_TURBO,
	VEH_BIRTH_CHEAT_ITEM_BOMB = HELD_ITEM_BOMB_1X,
	VEH_BIRTH_CHEAT_ITEM_MASK = HELD_ITEM_MASK,
	VEH_BIRTH_CHEAT_ITEM_COUNT = 9,
	VEH_BIRTH_CHEAT_DURATION = 0x2d00,
	VEH_BIRTH_PLAYER_THREAD_FLAGS = SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Driver), NONE, LARGE, PLAYER),
};

CTR_STATIC_ASSERT(sizeof(struct Driver) < THREAD_LARGE_STACK_SIZE - sizeof(struct Item));

typedef u32 VehBirthInstDefNameWord CTR_MAY_ALIAS;

void VehBirth_TeleportSelf(struct Driver *d, u32 spawnFlag, int spawnPosY)
{
	SVec3 posBottom;
	SVec3 warppadPos;
	SVec3 gridRot = {0};
	bool extraGridSlot = false;
	struct Driver *driver = d;
	s16 *warppadRot = NULL;
	int spawnOutsideBoss = 0;
	struct InstDef *doorInst;
	struct ScratchpadStruct *sps;
	register struct GameTracker *initialTracker CTR_PSX_REGISTER("$4");

	initialTracker = GAME_TRACKER;
	doorInst = NULL;
	if (initialTracker->level1 == NULL)
	{
		return;
	}

	if (initialTracker->level1->ptr_mesh_info == NULL)
	{
		return;
	}

	sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
	VehSpawn_InitGroundProbe(initialTracker, sps);

	initialTracker->gameMode2 &= ~VEH_FREEZE_DOOR;

	if ((spawnFlag & VEH_BIRTH_SPAWN_USE_LEVEL_POSITION) != 0)
	{
		if ((GAME_TRACKER->podiumRewardID == STATIC_KEY) && (GAME_TRACKER->currAdvProfile.numKeys == 1))
		{
			register int numInstances CTR_PSX_REGISTER("$3");
			register int instIndex CTR_PSX_REGISTER("$6");
			register int loopLimit CTR_PSX_REGISTER("$7");
			register u8 *nameCursor CTR_PSX_REGISTER("$4");
			register u8 *doorName CTR_PSX_REGISTER("$5");
			register u32 firstNameWord CTR_PSX_REGISTER("$8");
			register int doorModelID CTR_PSX_REGISTER("$9");

			// NOTE(aalhendi): Retail compares the fixed-width door name as four
			// words; the may-alias type keeps those loads valid in native C.
			numInstances = (int)GAME_TRACKER->level1->numInstances;
			doorInst = GAME_TRACKER->level1->ptrInstDefs;
			if (numInstances > 0)
			{
				instIndex = 0;
				doorModelID = STATIC_DOOR;
				doorName = (u8 *)rdata.s_door5;
				firstNameWord = ((VehBirthInstDefNameWord *)rdata.s_door5)[0];
				loopLimit = numInstances;
				nameCursor = (u8 *)doorInst + 0xc;
			VehBirth_DoorLoop:
				if ((*(s16 *)(nameCursor + 0x30) == doorModelID) && (((VehBirthInstDefNameWord *)doorInst->name)[0] == firstNameWord) &&
				    (*(VehBirthInstDefNameWord *)(nameCursor - 8) == ((VehBirthInstDefNameWord *)doorName)[1]) &&
				    (*(VehBirthInstDefNameWord *)(nameCursor - 4) == ((VehBirthInstDefNameWord *)doorName)[2]) &&
				    (*(VehBirthInstDefNameWord *)nameCursor == ((VehBirthInstDefNameWord *)doorName)[3]))
				{
					goto VehBirth_DoorLoopDone;
				}

				nameCursor += sizeof(*doorInst);
				instIndex++;
				doorInst++;
				if (instIndex < loopLimit)
				{
					goto VehBirth_DoorLoop;
				}

			VehBirth_DoorLoopDone:
			{
				register int foundDoor CTR_PSX_REGISTER("$2") = instIndex < (int)GAME_TRACKER->level1->numInstances;

				if (foundDoor)
				{
					goto VehBirth_DoorFound;
				}
			}

				doorInst = NULL;
			}
			else
			{
				doorInst = NULL;
			}
		VehBirth_DoorFound:;
		}
		else if (GAME_TRACKER->podiumRewardID == STATIC_TROPHY)
		{
			register int rewardIndex CTR_PSX_REGISTER("$6");
			u32 *rewards;
			s16 *trackIDs;
			int trackOffset;

			spawnOutsideBoss = 1;
			rewardIndex = 0;
			rewards = GAME_ADV_PROGRESS.rewards;
			trackIDs = data.advHubTrackIDs;
			trackOffset = (GAME_TRACKER->levelID - N_SANITY_BEACH) * 8;
			do
			{
				int reward = *(s16 *)((size_t)trackOffset + (size_t)trackIDs) + ADV_REWARD_FIRST_TROPHY;

				if (!CHECK_ADV_BIT(rewards, reward))
				{
					spawnOutsideBoss = 0;
					break;
				}

				rewardIndex++;
				trackOffset += sizeof(s16);
			} while (rewardIndex < 4);

			{
				register int hasAllTrophies CTR_PSX_REGISTER("$2") = spawnOutsideBoss;

				if (hasAllTrophies && CHECK_ADV_BIT(GAME_ADV_PROGRESS.rewards, GAME_TRACKER->levelID - N_SANITY_BEACH + ADV_REWARD_FIRST_BOSS_KEY))
				{
					spawnOutsideBoss = 0;
				}
			}
		}

		if ((GAME_TRACKER->gameMode2 & SPAWN_AT_BOSS) != 0)
		{
			spawnOutsideBoss = 1;
		}

		if (doorInst != NULL)
		{
			int trig;

			GAME_TRACKER->gameMode2 |= VEH_FREEZE_DOOR;
			trig = MATH_Cos(doorInst->rot.y);
			posBottom.x = doorInst->pos.x + (trig * VEH_BIRTH_DOOR_FORWARD_OFFSET >> FRACTIONAL_BITS) +
			              (MATH_Cos(doorInst->rot.y + ANG_HALF_PI) * VEH_BIRTH_DOOR_SIDE_OFFSET >> FRACTIONAL_BITS);
			posBottom.y = doorInst->pos.y + VEH_BIRTH_DOOR_Y_OFFSET;
			trig = MATH_Sin(doorInst->rot.y);
			posBottom.z = doorInst->pos.z + (trig * VEH_BIRTH_DOOR_FORWARD_OFFSET >> FRACTIONAL_BITS) +
			              (MATH_Sin(doorInst->rot.y + ANG_HALF_PI) * VEH_BIRTH_DOOR_SIDE_OFFSET >> FRACTIONAL_BITS);
		}
		else
		{
			register int shouldSpawnOutside CTR_PSX_REGISTER("$2") = spawnOutsideBoss;

			if (shouldSpawnOutside)
			{
				posBottom.x = GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[1].pos.x;
				posBottom.y = GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[1].pos.y + VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET;
				posBottom.z = GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[1].pos.z;
				goto VehBirth_PositionReady;
			}

			if ((GAME_TRACKER->gameMode1 & ADVENTURE_ARENA) != 0)
			{
				if (GAME_TRACKER->podiumRewardID != NOFUNC)
				{
					posBottom.x = GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[0].pos.x;
					posBottom.y = GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[0].pos.y + VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET;
					posBottom.z = GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[0].pos.z;
					goto VehBirth_PositionReady;
				}

				{
					int prevLEV = GAME_TRACKER->prevLEV;

					// NOTE(aalhendi): Keep these as separate tests. GCC 2.8 emits the
					// same branch ladder and delay-slot constants as retail.
					if (prevLEV == MAIN_MENU_LEVEL)
					{
						goto VehBirth_StartlinePosition;
					}
					if (prevLEV == ADVENTURE_GARAGE)
					{
						goto VehBirth_StartlinePosition;
					}
					if (prevLEV == -1)
					{
						goto VehBirth_StartlinePosition;
					}
					if (prevLEV == SCRAPBOOK)
					{
						goto VehBirth_StartlinePosition;
					}
					if ((u32)(prevLEV - CREDITS_CRASH) < VEH_BIRTH_ADV_RETURN_LEVEL_COUNT)
					{
						goto VehBirth_StartlinePosition;
					}

					warppadRot = AH_WarpPad_GetSpawnPosRot(CTR_VECTOR_DATA(&(warppadPos)));
					posBottom.x = warppadPos.x;
					posBottom.y = warppadPos.y + VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET;
					posBottom.z = warppadPos.z;
					goto VehBirth_PositionReady;
				}
			}

		VehBirth_StartlinePosition:
		{
			register u8 *spawnOrder CTR_PSX_REGISTER("$3");
			register u32 driverID CTR_PSX_REGISTER("$2");
			register struct GameTracker *tracker CTR_PSX_REGISTER("$4");
			register u32 spawnIndex CTR_PSX_REGISTER("$5");

			spawnOrder = (u8 *)VEH_KART_SPAWN_ORDER;
			driverID = driver->driverID;
			tracker = GAME_TRACKER;
			spawnIndex = spawnOrder[driverID];
			CTR_PSX_KEEP_VALUE(spawnIndex);

			VehStartGrid_GetTransform(tracker->level1, spawnIndex, &posBottom, &gridRot);
			extraGridSlot = spawnIndex >= len(tracker->level1->DriverSpawn);
			if (!extraGridSlot)
			{
				posBottom.y += VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET;
			}

			driver->actionsFlagSet |= ACTION_BEHIND_START_LINE;
			if (tracker->level1->ptr_restart_points == NULL)
			{
				// NOTE(aalhendi): Retail does an unguarded low-address read here;
				// native cannot dereference PS1 null-space for menu/hub-style LEVs.
				driver->distanceToFinish_checkpoint = 0;
			}
			else
			{
				driver->distanceToFinish_checkpoint = tracker->level1->ptr_restart_points[0].distToFinish << 3;
			}
		}
		}
	}
	else
	{
		posBottom.x = (s16)CTR_MipsSra(driver->posCurr.x, FRACTIONAL_BITS_8);
		posBottom.y = (s16)CTR_MipsAddLo(CTR_MipsSra(driver->posCurr.y, FRACTIONAL_BITS_8), VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET);
		posBottom.z = (s16)CTR_MipsSra(driver->posCurr.z, FRACTIONAL_BITS_8);
	}

VehBirth_PositionReady:
	if (extraGridSlot)
	{
		driver->lastValid = NULL;
		VehStartGrid_ResolveHeight(&posBottom, sps);
	}
	else
	{
		VehSpawn_ProbeGround(&posBottom, sps);
	}

	if (sps->boolDidTouchQuadblock != 0)
	{
		driver->AxisAngle3_normalVec.x = sps->hit.plane.normal.x;
		driver->AxisAngle3_normalVec.y = sps->hit.plane.normal.y;
		driver->AxisAngle3_normalVec.z = sps->hit.plane.normal.z;
		driver->lastValid = sps->hit.ptrQuadblock;
	}
	else
	{
		driver->AxisAngle3_normalVec.x = 0;
		driver->AxisAngle3_normalVec.y = FP_ONE;
		driver->AxisAngle3_normalVec.z = 0;
	}

	{
		register u32 normalX CTR_PSX_REGISTER("$2") = (u16)driver->AxisAngle3_normalVec.x;
		register u32 normalY CTR_PSX_REGISTER("$3") = (u16)driver->AxisAngle3_normalVec.y;
		register u32 normalZ CTR_PSX_REGISTER("$4") = (u16)driver->AxisAngle3_normalVec.z;

		driver->AxisAngle1_normalVec.x = normalX;
		driver->AxisAngle2_normalVec.x = normalX;
		driver->AxisAngle1_normalVec.y = normalY;
		driver->AxisAngle2_normalVec.y = normalY;
		driver->AxisAngle1_normalVec.z = normalZ;
		driver->AxisAngle2_normalVec.z = normalZ;
	}

	{
		register int normalIndex CTR_PSX_REGISTER("$6") = 0;
		register struct Driver *normalDriver CTR_PSX_REGISTER("$5") = driver;

		do
		{
			normalDriver->AxisAngle4_normalVec.x = driver->AxisAngle2_normalVec.x;
			normalDriver->AxisAngle4_normalVec.y = driver->AxisAngle2_normalVec.y;
			normalDriver->AxisAngle4_normalVec.z = driver->AxisAngle2_normalVec.z;
			normalIndex++;
			normalDriver = (struct Driver *)((u8 *)normalDriver + sizeof(SVec3Slot));
		} while (normalIndex < 1);
	}

	driver->posCurr.x = CTR_MipsSll(sps->Union.QuadBlockColl.hitPos.x, FRACTIONAL_BITS_8);
	driver->posCurr.y = CTR_MipsSll(CTR_MipsAddLo(sps->Union.QuadBlockColl.hitPos.y, spawnPosY), FRACTIONAL_BITS_8);
	driver->posCurr.z = CTR_MipsSll(sps->Union.QuadBlockColl.hitPos.z, FRACTIONAL_BITS_8);
	driver->posPrev.y = driver->posCurr.y;
	driver->posPrev.x = driver->posCurr.x;
	driver->posPrev.z = driver->posCurr.z;
	driver->quadBlockHeight = CTR_MipsSll(sps->Union.QuadBlockColl.hitPos.y, FRACTIONAL_BITS_8);

	if ((spawnFlag & VEH_BIRTH_SPAWN_USE_LEVEL_POSITION) != 0)
	{
		if (doorInst != NULL)
		{
			driver->rotCurr.y = ANG_MODULO_TWO_PI(doorInst->rot.y + ANG_PI);
			GAME_TRACKER->gameMode2 &= ~GAME_MODE2_SPAWN_CLEAR_MASK;
		}
		else
		{
			register int shouldSpawnOutside CTR_PSX_REGISTER("$2") = spawnOutsideBoss;

			if (shouldSpawnOutside)
			{
				register struct GameTracker *bossTracker CTR_PSX_REGISTER("$4");
				register u32 rotY CTR_PSX_REGISTER("$5");
				register u32 updatedRotY CTR_PSX_REGISTER("$2");

				bossTracker = GAME_TRACKER;
				updatedRotY = (u16)bossTracker->level1->ptrSpawnType2_PosRot[1].coords.posRot[1].rot.y;
				updatedRotY += ANG_HALF_PI;
				rotY = ANG_MODULO_TWO_PI(updatedRotY);

				driver->rotCurr.y = rotY;
				if ((bossTracker->gameMode2 & SPAWN_AT_BOSS) != 0)
				{
					if (bossTracker->levelID == CITADEL_CITY)
					{
						updatedRotY = rotY + ANG_HALF_PI;
						goto VehBirth_UpdateBossRotation;
					}

					if ((bossTracker->levelID == GEM_STONE_VALLEY) && (bossTracker->prevLEV == HOT_AIR_SKYWAY))
					{
						// NOTE(aalhendi): Make the redundant retail store observable so
						// GCC can schedule it into the following jump's delay slot.
						CTR_PSX_FORGET_VALUE(rotY);
						driver->rotCurr.y = rotY;
						goto VehBirth_BossRotationReady;
					}

					CTR_PSX_RELOAD(driver->rotCurr.y);
					updatedRotY = (u16)driver->rotCurr.y + ANG_PI;

				VehBirth_UpdateBossRotation:
					driver->rotCurr.y = ANG_MODULO_TWO_PI(updatedRotY);
				}

			VehBirth_BossRotationReady:
				GAME_TRACKER->gameMode2 &= ~GAME_MODE2_SPAWN_CLEAR_MASK;
				goto VehBirth_RotationReady;
			}

			if ((GAME_TRACKER->gameMode1 & ADVENTURE_ARENA) != 0)
			{
				if (GAME_TRACKER->podiumRewardID != NOFUNC)
				{
					driver->rotCurr.y = ANG_MODULO_TWO_PI(GAME_TRACKER->level1->ptrSpawnType2_PosRot[1].coords.posRot[0].rot.y);
					goto VehBirth_RotationReady;
				}

				if ((GAME_TRACKER->prevLEV != MAIN_MENU_LEVEL) && (GAME_TRACKER->prevLEV != -1) && (warppadRot != NULL))
				{
					driver->rotCurr.x = warppadRot[0];
					driver->rotCurr.y = warppadRot[1];
					driver->rotCurr.z = warppadRot[2];
					driver->rotCurr.y = ANG_MODULO_TWO_PI(driver->rotCurr.y + ANG_HALF_PI);
					goto VehBirth_RotationReady;
				}
			}

			{
				driver->rotCurr.x = gridRot.x;
				driver->rotCurr.y = ANG_MODULO_TWO_PI(gridRot.y + ANG_HALF_PI);
				driver->rotCurr.z = gridRot.z;
			}
		}
	}

VehBirth_RotationReady:
	driver->speed = 0;
	driver->speedApprox = 0;
	driver->jumpHeightCurr = 0;
	driver->jumpHeightPrev = 0;
	driver->forwardAccelImpulse = 0;
	driver->angle = driver->rotCurr.y;
	driver->rotPrev.x = driver->rotCurr.x;
	driver->rotPrev.y = driver->rotCurr.y;
	driver->rotPrev.z = driver->rotCurr.z;

	if ((doorInst != NULL) && ((spawnFlag & VEH_BIRTH_SPAWN_USE_LEVEL_POSITION) != 0))
	{
		driver->speed = VEH_BIRTH_DOOR_EXIT_SPEED;
	}

	// set animation to zero
	driver->instSelf->animIndex = 0;

	driver->instSelf->animFrame = VehFrameInst_GetStartFrame(0, VehFrameInst_GetNumAnimFrames(driver->instSelf, 0));

	// Set Scale (x, y, z)
	driver->instSelf->scale.x = VEH_BIRTH_DRIVER_INSTANCE_SCALE;
	driver->instSelf->scale.y = VEH_BIRTH_DRIVER_INSTANCE_SCALE;
	driver->instSelf->scale.z = VEH_BIRTH_DRIVER_INSTANCE_SCALE;

	driver->matrixArray = BAKED_GTE_MATRIX_NONE;
	driver->matrixIndex = 0;
	driver->jump_LandingBoost = 0;
	driver->jumpMeter = 0;
	driver->jumpMeterTimer = 0;
	driver->turnAngleCurr = 0;
	driver->turnAngleLerpVel = 0;
	driver->turnAnglePrev = 0;
	driver->rotCurr.w = 0;
	driver->rotPrev.w = 0;
	driver->pendingDamageType = 0;
	driver->jumpSquishStretch = 0;
	driver->underDriver = 0;
	driver->distanceDrivenBackwards = 0;
	driver->clockReceive = 0;
	driver->revEngineState = 0;
	driver->actionsFlagSet &= ~(ACTION_AIRBORNE | ACTION_HIGH_JUMP);

	if ((spawnFlag & VEH_BIRTH_SPAWN_INIT_RACE_STATE) == 0)
	{
		return;
	}
	CTR_PSX_KEEP_VALUE(spawnFlag);

	if (driver->instSelf->thread->modelIndex == DYNAMIC_PLAYER)
	{
		register int funcIndex CTR_PSX_REGISTER("$6");
		register u8 *funcCursor CTR_PSX_REGISTER("$2");

		// NOTE(aalhendi): Retail carries a base at Driver+0x30 and clears
		// the function table through a +0x54 displacement.
		funcIndex = DRIVER_FUNC_COUNT - 1;
		funcCursor = (u8 *)driver + 0x30;
		do
		{
			*(DriverFunc *)(funcCursor + 0x54) = NULL;
			funcIndex--;
			funcCursor -= sizeof(DriverFunc);
		} while (funcIndex >= 0);

		CAM_StartOfRace(&GAME_TRACKER->cameraDC[driver->driverID]);

		driver->instSelf->thread->funcThTick = ((GAME_TRACKER->gameMode1 & (GAME_CUTSCENE | MAIN_MENU)) == 0) ? NULL : VehBirth_NullThread;

		// set OnInit function
		if ((GAME_TRACKER->gameMode1 & ADVENTURE_ARENA) != 0)
		{
			driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
		}
		else
		{
			driver->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_RevEngine_Init;
		}
	}

	driver->lapIndex = 0;
	driver->numWumpas = 0;
	driver->lapTime = 0;
	driver->distanceToFinish_curr = 0;

	driver->actionsFlagSet &= ~(ACTION_RACE_FINISHED | ACTION_BOT);

	if ((GAME_TRACKER->gameMode2 & CHEAT_WUMPA) != 0)
	{
		driver->numWumpas = VEH_BIRTH_CHEAT_WUMPA_COUNT;
	}

	driver->heldItemID = HELD_ITEM_NONE;
	driver->numHeldItems = 0;
	driver->PickupLetterHUD.numCollected = 0;

	if ((GAME_TRACKER->gameMode2 & CHEAT_MASK) != 0)
	{
		driver->heldItemID = VEH_BIRTH_CHEAT_ITEM_MASK;
		driver->numHeldItems = VEH_BIRTH_CHEAT_ITEM_COUNT;
	}
	else if ((GAME_TRACKER->gameMode2 & CHEAT_TURBO) != 0)
	{
		driver->heldItemID = VEH_BIRTH_CHEAT_ITEM_TURBO;
		driver->numHeldItems = VEH_BIRTH_CHEAT_ITEM_COUNT;
	}
	else if ((GAME_TRACKER->gameMode2 & CHEAT_BOMBS) != 0)
	{
		driver->heldItemID = VEH_BIRTH_CHEAT_ITEM_BOMB;
		driver->numHeldItems = VEH_BIRTH_CHEAT_ITEM_COUNT;
	}

	driver->BattleHUD.numLives = GAME_TRACKER->battleSetup.lifeLimit;

	if (
	    // If Permanent Invisibility Cheat is Enabled
	    ((GAME_TRACKER->gameMode2 & CHEAT_INVISIBLE) != 0) &&

	    // only make players invisible, not AIs
	    (driver->driverID < GAME_TRACKER->numPlyrCurrGame))
	{
		driver->instSelf->flags &= ~(DRAW_TRANSPARENT | GHOST_DRAW_TRANSPARENT);

		driver->instSelf->flags |= GHOST_DRAW_TRANSPARENT;

		driver->invisibleTimer = VEH_BIRTH_CHEAT_DURATION;
	}

	if ((GAME_TRACKER->gameMode2 & CHEAT_ENGINE) != 0)
	{
		driver->superEngineTimer = VEH_BIRTH_CHEAT_DURATION;
	}
}


void VehBirth_TeleportAll(struct GameTracker *gGT, u32 spawnFlags)
{
	struct Driver *d;
	s16 i;

	// NOTE(aalhendi): Retail ignores this parameter and reloads gGT from globals.
	(void)gGT;

	for (i = 0; i < (int)len(GAME_TRACKER->drivers); i++)
	{
		d = GAME_TRACKER->drivers[i];

		if (d == NULL)
		{
			continue;
		}

		if (d->instSelf->thread->modelIndex == DYNAMIC_ROBOT_CAR)
		{
			BOTS_GotoStartingLine(d);
		}

		else
		{
			VehBirth_TeleportSelf(d, spawnFlags | VEH_BIRTH_SPAWN_USE_LEVEL_POSITION, 0);
		}
	}
}

struct Model *VehBirth_GetModelByName(char *searchName)
{
	return LOAD_GetRetailCharacterModel(searchName);
}

#include "VehBirth_Consts.c"

void VehBirth_EngineAudio_AllPlayers(void)
{
	struct GameTracker *gGT = GAME_TRACKER;
	struct Thread *th;
	struct Driver *d;
	register u32 driverID CTR_PSX_REGISTER("$3");
	register int engine CTR_PSX_REGISTER("$4");

	for (th = gGT->threadBuckets[PLAYER].thread; th != 0; th = th->siblingThread)
	{
		d = th->object;
		driverID = d->driverID;
		engine = GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[driverID]].engineID;
		engine = Chaos_EngineClassForDriver(d, engine);
		engine = EngineClass_AudioClass(engine);
		CTR_PSX_MEMORY_BARRIER();
		engine = (engine * 4) + driverID;

		EngineAudio_InitOnce((u16)engine, HOWL_SFX_CENTER_NO_DISTORTION);
	}
}

void VehBirth_NullThread(struct Thread *t)
{
	(void)t;
}

#include "VehBirth_Appearance.c"

void VehBirth_NonGhost(struct Thread *t, int index)
{
	register int playerIndex CTR_PSX_REGISTER("$19") = index;
	struct Driver *d;
	int collisionValue;
	int id;
	struct Model *m;
	struct Instance *inst;
	struct Instance *wakeInst;

	// model index = DYNAMIC_PLAYER,
	// AI will override this right after
	// the end of the function
	collisionValue = DYNAMIC_PLAYER;
	CTR_PSX_KEEP_VALUE(playerIndex);
	CTR_PSX_KEEP_VALUE(collisionValue);
	t->modelIndex = (s16)collisionValue;

	collisionValue = THREAD_DRIVER_HIT_RADIUS;
	t->driverHitRadius = (s16)collisionValue;
	collisionValue = THREAD_DRIVER_HIT_RADIUS_SQUARED;
	t->driverHitRadiusSquared = collisionValue;
	collisionValue = THREAD_DRIVER_HIT_RADIUS;
	t->driverCollisionReserved_0x3e = (s16)collisionValue;
	t->driverCollisionReserved_0x3c = 0;
	t->driverCollisionReserved_0x40 = 0;

	d = t->object;
	// The recycled pool payload includes native fields beyond the retail prefix.
	memset(d, 0, sizeof(*d));
	d->maskIsAku = -1;

	if ((GAME_TRACKER->gameMode1 & MAIN_MENU) != 0)
	{
		id = GAME_CHARACTER_IDS[0];
	}
	else
	{
		register s16 *characterIDs CTR_PSX_REGISTER("$2");
		register s16 *characterID CTR_PSX_REGISTER("$3");

		characterIDs = GAME_CHARACTER_IDS;
		characterID = &characterIDs[playerIndex];
		id = *characterID;
	}

	m = LOAD_GetDriverModel(playerIndex, id);

	inst = INSTANCE_Birth3D(m, m->name, t);

	t->inst = inst;

	// Wake
	m = GAME_TRACKER->modelPtr[STATIC_WAKE];
	if (m != 0)
	{
		wakeInst = INSTANCE_Birth3D(m, m->name, 0);
		d->wakeInst = wakeInst;

		if (wakeInst != 0)
		{
			wakeInst->flags |= HIDE_MODEL | ANIM_LOOP;
		}
	}

	inst = t->inst;
	if (playerIndex < GAME_TRACKER->numPlyrCurrGame)
	{
		inst->flags |= OWNER_PUSHBUFFER_GATE;
	}

	d->driverID = playerIndex;
	d->instSelf = inst;

	VehBirth_TireSprites(t);
	// NOTE(aalhendi): Retail leaves terrainMeta2 unset until COLL_FIXED;
	// native cannot dereference the PS1 low-memory null-space before then.
	d->terrainMeta2 = d->terrainMeta1;
	if ((GAME_TRACKER->gameMode1 & GAME_MODE_MENU_OR_CUTSCENE_MASK) == 0 && playerIndex < GAME_TRACKER->numPlyrCurrGame)
	{
		const int defaultClass = GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[playerIndex]].engineID;
		VehBirth_SetConstsForClass(d, VehBirth_EngineClassForPlayer(playerIndex, defaultClass));
	}
	else
	{
		VehBirth_SetConsts(d);
	}

	// if you are in cutscene or in main menu
	if ((GAME_TRACKER->gameMode1 & GAME_MODE_MENU_OR_CUTSCENE_MASK) != 0)
	{
		// dont update, make invisible
		t->funcThTick = VehBirth_NullThread;
		inst->flags |= HIDE_MODEL;
	}
}

struct Driver *VehBirth_Player(int index)
{
	register struct Thread *t CTR_PSX_REGISTER("$16");
	register struct Driver *d CTR_PSX_REGISTER("$17");
	int teamID;

	t = PROC_BirthWithObject(VEH_BIRTH_PLAYER_THREAD_FLAGS, 0, VEH_PLAYER_THREAD_NAME, 0);
	d = t->object;

	VehBirth_NonGhost(t, index);

	teamID = (s8)GAME_TRACKER->battleSetup.teamOfEachPlayer[index];
	d->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
	d->BattleHUD.teamID = teamID;

	return d;
}

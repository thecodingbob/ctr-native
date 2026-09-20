#include <common.h>

enum AHDoorConstants
{
	AH_DOOR_ACCESS_FLAG = 2,
	AH_DOOR_INTERACTION_FORWARD_OFFSET = 0x300,
	AH_DOOR_NEAR_DIST_SQ = 0x90000,
	AH_DOOR_NEAR_DIST_MAX_SQ = AH_DOOR_NEAR_DIST_SQ - 1,
	AH_DOOR_CAMERA_HOLD_FRAMES = CTR_SECONDS_TO_FRAMES(2),
	AH_DOOR_KEY_SPIN_FRAMES = CTR_SECONDS_TO_FRAMES(4),
	AH_DOOR_KEY_LIGHT_X = -0xc98,
	AH_DOOR_KEY_LIGHT_Y = 0x99f,
	AH_DOOR_KEY_LIGHT_Z = 0x232,
	AH_DOOR_KEY_TARGET_SCALE = 0xa00,
	AH_DOOR_KEY_SCALE_STEP = 0x40,
	AH_DOOR_KEY_ROT_STEP = 0x40,
	AH_DOOR_KEY_RAISE_HEIGHT = 0xa0,
	AH_DOOR_KEY_RAISE_STEP = 4,
	AH_DOOR_KEY_ORBIT_STEP = 0x10,
	AH_DOOR_KEY_SHRINK_FRAME_COUNT = 0xb,
	AH_DOOR_KEY_FLOAT_RADIUS_SHIFT = 5,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_0 = 0x0a,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_1 = 0x0f,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_2 = 0x14,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_3 = 0x19,
	AH_DOOR_KEY_FLOAT_SFX_ID = 0x67,
	AH_DOOR_UNLOCK_SFX_FRAME = 0x50,
	AH_DOOR_UNLOCK_SFX_ID = 0x93,
	AH_DOOR_CREAK_SFX_ID = 0x94,
	AH_DOOR_OPEN_ROTATION = 0x400,
	AH_DOOR_OPEN_ROTATION_STEP = 0x10,
	AH_DOOR_PAIR_OFFSET = 0x600,
	AH_DOOR_CAMERA_FORWARD_OFFSET = 0x312,
	AH_DOOR_CAMERA_SIDE_OFFSET = 0x600,
	AH_DOOR_CAMERA_HEIGHT_OFFSET = 0x17a,
	AH_DOOR_CAMERA_PITCH_OFFSET = 0x800,
};

void AH_Door_ThDestroy(struct Thread *t)
{
	int i;
	struct WoodDoor *woodDoor = t->object;

	if (woodDoor->otherDoor != NULL)
	{
		INSTANCE_Death(woodDoor->otherDoor);
		woodDoor->otherDoor = NULL;
	}

	for (i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
	{
		INSTANCE_Death(woodDoor->keyInst[i]);
		woodDoor->keyInst[i] = NULL;
	}
	return;
}

void AH_Door_ThTick(struct Thread *t)
{
	b32 doorIsOpen;
	s16 requiredCount;
	s16 numKeys;
	u16 hintId;
	s32 i;
	s32 ratio;
	s32 distX;
	s32 distY;
	s32 distZ;
	s32 dist;
	s32 levelID;
	s32 beachLevel;
	SVec3 desiredPos;
	SVec3 desiredRot;

	struct GameTracker *gGT;
	struct GameTracker *initialTracker = GAME_TRACKER;
	struct CameraDC *cDC = initialTracker->cameraDC;
	register struct Instance *doorInst CTR_PSX_REGISTER("$21") = t->inst;
	struct Driver *driver = initialTracker->drivers[0];
	struct WoodDoor *door = t->object;
	// Each route has its own reward bit, including the two beach doors.
	doorIsOpen = false;

	if (GAME_TRACKER->levelID == N_SANITY_BEACH && door->doorID == AH_DOOR_BEACH_TO_GLACIER_PARK &&
	    (AH_STORY_REWARDS & ADV_REWARD_DOOR_BEACH_TO_GLACIER_PARK_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == N_SANITY_BEACH && door->doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY &&
	         (AH_STORY_REWARDS & ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == GEM_STONE_VALLEY && (AH_STORY_REWARDS & ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == THE_LOST_RUINS && (AH_STORY_REWARDS & ADV_REWARD_DOOR_LOST_RUINS_TO_GLACIER_PARK_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == GLACIER_PARK && (AH_STORY_REWARDS & ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK))
		doorIsOpen = true;
    if (g_config.unlockAllGates)
    {
      doorIsOpen = true;
    }
	// Test the point just in front of the doorway, using wrapping MIPS distance arithmetic.
	ratio = MATH_Cos((int)doorInst->instDef->rot.y);
	distX = CTR_MipsSubLo(CTR_MipsAddLo(doorInst->matrix.t[0], ratio * AH_DOOR_INTERACTION_FORWARD_OFFSET >> 0xc), driver->instSelf->matrix.t[0]);
	distY = CTR_MipsSubLo(doorInst->matrix.t[1], driver->instSelf->matrix.t[1]);
	ratio = MATH_Sin((int)doorInst->instDef->rot.y);
	distZ = CTR_MipsSubLo(CTR_MipsAddLo(doorInst->matrix.t[2], ratio * AH_DOOR_INTERACTION_FORWARD_OFFSET >> 0xc), driver->instSelf->matrix.t[2]);
	dist = (s32)((u32)CTR_MipsMulLo(distX, distX) + (u32)CTR_MipsMulLo(distY, distY) + (u32)CTR_MipsMulLo(distZ, distZ));
	// Only the nearby door controls access through the collision boundary.
	if (dist < AH_DOOR_NEAR_DIST_SQ)
	{
		if ((s16)doorIsOpen)
		{
			GAME_DOOR_ACCESS_FLAGS |= AH_DOOR_ACCESS_FLAG;
		}
		else
		{
			GAME_DOOR_ACCESS_FLAGS &= ~AH_DOOR_ACCESS_FLAG;
		}
	}

	if (GAME_TRACKER->levelID != N_SANITY_BEACH)
		requiredCount = AH_HUB_REQUIRED_KEYS[GAME_TRACKER->levelID - GEM_STONE_VALLEY];
	else
	{
		requiredCount = 1;
		if (door->doorID == AH_DOOR_BEACH_TO_GLACIER_PARK)
			requiredCount = 2;
	}
	// A boss-key cutscene elsewhere owns the player until it finishes.
	gGT = GAME_TRACKER;
	if ((gGT->gameMode2 & 4) != 0)
	{
		return;
	}

	if ((s16)doorIsOpen)
		goto doorOpened;

	if ((AH_DOOR_NEAR_DIST_MAX_SQ < dist) && ((door->camFlags & WdCam_CutscenePlaying) == 0))
	{
		return;
	}

	// Queue each missing-key hint once; an open door has a separate greeting.
	if (gGT->currAdvProfile.numKeys < requiredCount)
	{
		if (requiredCount == 1)
		{
			if (AH_STORY_REWARDS & ADV_REWARD_HINT_MUST_HAVE_ONE_BOSS_KEY_MASK)
				return;
			hintId = ADV_MASK_HINT_ID_MUST_HAVE_ONE_BOSS_KEY;
		}
		else
		{
			if (requiredCount != 2)
				return;
			if (GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_HINT] & ADV_REWARD_HINT_MUST_HAVE_TWO_BOSS_KEYS_MASK)
				return;
			hintId = ADV_MASK_HINT_ID_MUST_HAVE_TWO_BOSS_KEYS;
		}
		goto requestHint;
	}

	// The camera transition and the door animation advance together.
	if ((door->camFlags & WdCam_FlyingOut) != 0)
	{
		door->camTimer_unused = AH_DOOR_CAMERA_HOLD_FRAMES;

		if (((cDC->flags & CAMERA_FLAG_TRANSITION_AWAY) == 0) && ((door->camFlags & WdCam_FlyingIn) == 0))
		{
			driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
			door->camFlags |= WdCam_FlyingIn;
		}
		else if (((cDC->flags & CAMERA_FLAG_TRANSITION_HOLD) != 0) && ((door->camFlags & WdCam_FullyOut) == 0))
		{
			door->camFlags |= WdCam_FullyOut;
		}
	}
	else
	{
		if (door->camTimer_unused != 0)
		{
			door->camTimer_unused--;
			goto checkOpening;
		}
		if ((gGT->gameMode1 & PAUSE_ALL) != 0)
		{
			goto checkOpening;
		}

		driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_FreezeEndEvent_Init;

		door->camFlags |= WdCam_CutscenePlaying;
		numKeys = requiredCount;
		if (door->frameCount_doorOpenAnim < AH_DOOR_KEY_SPIN_FRAMES)
		{
			if (driver->speedApprox < 0x80)
			{
				// This local vector is lighting input during the key animation, then a camera target.
				desiredPos.x = AH_DOOR_KEY_LIGHT_X;
				desiredPos.y = AH_DOOR_KEY_LIGHT_Y;
				desiredPos.z = AH_DOOR_KEY_LIGHT_Z;
				// Publish each key immediately; subsequent state comes from the current tracker.
				if (door->keyInst[0] == NULL)
				{
					{
						for (i = 0; i < numKeys; i++)
						{
							door->keyInst[i] = INSTANCE_Birth3D(GAME_TRACKER->modelPtr[STATIC_KEY], AH_KEY_NAME, t);
							door->keyInst[i]->colorRGBA = INST_COLOR_KEY;

							door->keyInst[i]->flags |= USE_SPECULAR_LIGHT;
							door->frameCount_unused++;
							door->keyInst[i]->matrix = GAME_TRACKER->drivers[0]->instSelf->matrix;
							door->keyInst[i]->scale.x = 0;
							door->keyInst[i]->scale.y = 0;
							door->keyInst[i]->scale.z = 0;

							// Retail refreshes translation after the full matrix copy and scale reset.
							door->keyInst[i]->matrix.t[0] = GAME_TRACKER->drivers[0]->instSelf->matrix.t[0];
							door->keyInst[i]->matrix.t[1] = GAME_TRACKER->drivers[0]->instSelf->matrix.t[1];
							door->keyInst[i]->matrix.t[2] = GAME_TRACKER->drivers[0]->instSelf->matrix.t[2];
						}
					}
					door->hudFlags = (u8)GAME_TRACKER->hudFlags;
					GAME_TRACKER->hudFlags = 0;
				}
				i = 0;
				if (0 < numKeys)
				{
					s32 keyCount;
					SVec3 *keyRotation;
					do
					{
						// NOTE(aalhendi): Retain the slot address, not a tracker snapshot, across trig/light calls.
						struct GameTracker **trackerSlot = &GAME_TRACKER;
						keyCount = numKeys;
						keyRotation = &door->keyRot;
						if (door->keyInst[i] != NULL)
						{
							if (door->keyInst[i]->scale.x < AH_DOOR_KEY_TARGET_SCALE)
							{
								door->keyInst[i]->scale.x += AH_DOOR_KEY_SCALE_STEP;
								door->keyInst[i]->scale.y += AH_DOOR_KEY_SCALE_STEP;
								door->keyInst[i]->scale.z += AH_DOOR_KEY_SCALE_STEP;
							}
							if (door->keyInst[i]->matrix.t[1] < (CTR_MipsAddLo((*trackerSlot)->drivers[0]->instSelf->matrix.t[1], AH_DOOR_KEY_RAISE_HEIGHT)))
							{
								door->keyInst[i]->matrix.t[1] = CTR_MipsAddLo(door->keyInst[i]->matrix.t[1], AH_DOOR_KEY_RAISE_STEP);
							}

							{
								SVec3 *lightingRot;
								lightingRot = keyRotation;
								if (1 < keyCount)
								{
									// NOTE(aalhendi): Keep the trig result live while loading scale, before reusing the argument registers.
									register s32 orbitTrig CTR_PSX_REGISTER("$2");
									register s32 orbitScale CTR_PSX_REGISTER("$4");
									struct Instance *orbitKey;
									struct Instance *orbitPlayer;
									s32 savedTrig;
									s32 keyOrbitOffset = i * (FP_ONE / keyCount);

									orbitTrig = MATH_Sin(door->keyOrbit + keyOrbitOffset);
									orbitPlayer = (*trackerSlot)->drivers[0]->instSelf;
									orbitKey = door->keyInst[i];
									orbitScale = orbitKey->scale.x;
									CTR_PSX_ORDER_VALUES(orbitTrig, orbitScale);
									savedTrig = orbitTrig;
									orbitKey->matrix.t[0] = CTR_MipsAddLo(orbitPlayer->matrix.t[0], (orbitScale / 32) * savedTrig >> 12);
									orbitTrig = MATH_Cos(door->keyOrbit + keyOrbitOffset);
									orbitPlayer = (*trackerSlot)->drivers[0]->instSelf;
									orbitKey = door->keyInst[i];
									orbitScale = orbitKey->scale.x;
									CTR_PSX_ORDER_VALUES(orbitTrig, orbitScale);
									savedTrig = orbitTrig;
									orbitKey->matrix.t[2] = CTR_MipsAddLo(orbitPlayer->matrix.t[2], (orbitScale / 32) * savedTrig >> 12);
									// The rotation argument becomes live only after the final orbit coordinate.
									CTR_PSX_OBSERVE_VALUE(savedTrig);

									lightingRot = keyRotation;
								}

								Vector_SpecLightSpin3D(door->keyInst[i], lightingRot, &desiredPos);
							}
							ConvertRotToMatrix(&door->keyInst[i]->matrix, keyRotation);
						}
						i++;
					} while (i < numKeys);
				}

				door->keyRot.x = 0;
				door->keyRot.y += AH_DOOR_KEY_ROT_STEP;
				door->keyRot.z = 0;

				door->keyOrbit += AH_DOOR_KEY_ORBIT_STEP;

				door->frameCount_doorOpenAnim++;

				// Four rising-key cues, followed by the unlock and door-creak cues.
				if (door->frameCount_doorOpenAnim == AH_DOOR_KEY_FLOAT_SFX_FRAME_0)
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xff7680);
				else if (door->frameCount_doorOpenAnim == AH_DOOR_KEY_FLOAT_SFX_FRAME_1)
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xeb8080);
				else if (door->frameCount_doorOpenAnim == AH_DOOR_KEY_FLOAT_SFX_FRAME_2)
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xd78a80);
				else if (door->frameCount_doorOpenAnim == AH_DOOR_KEY_FLOAT_SFX_FRAME_3)
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xc39480);
				else if (door->frameCount_doorOpenAnim == AH_DOOR_UNLOCK_SFX_FRAME)
					OtherFX_Play(AH_DOOR_UNLOCK_SFX_ID, 1);
				else if (door->frameCount_doorOpenAnim == AH_DOOR_KEY_SPIN_FRAMES)
					OtherFX_Play(AH_DOOR_CREAK_SFX_ID, 1);
			}
			goto checkOpening;
		}

		ratio = MATH_Cos((int)doorInst->instDef->rot.y);

		i = MATH_Cos((int)doorInst->instDef->rot.y + AH_DOOR_OPEN_ROTATION);
		desiredPos.x = doorInst->matrix.t[0] + (s16)(ratio * AH_DOOR_CAMERA_FORWARD_OFFSET >> 0xc) + (s16)(i * AH_DOOR_CAMERA_SIDE_OFFSET >> 0xc);
		desiredPos.y = doorInst->matrix.t[1] + AH_DOOR_CAMERA_HEIGHT_OFFSET;

		ratio = MATH_Sin((int)doorInst->instDef->rot.y);

		i = MATH_Sin((int)doorInst->instDef->rot.y + AH_DOOR_OPEN_ROTATION);
		desiredPos.z = doorInst->matrix.t[2] + (s16)(ratio * AH_DOOR_CAMERA_FORWARD_OFFSET >> 0xc) + (s16)(i * AH_DOOR_CAMERA_SIDE_OFFSET >> 0xc);
		desiredRot.x = doorInst->instDef->rot.x + AH_DOOR_CAMERA_PITCH_OFFSET;
		desiredRot.y = doorInst->instDef->rot.y;
		desiredRot.z = doorInst->instDef->rot.z;
		// Begin opening on this same tick after setting the camera target.
		CAM_SetDesiredPosRot(&GAME_TRACKER->cameraDC[0], &desiredPos, &desiredRot);

		GAMEPAD_JogCon2(driver, 0, 0);
		door->camFlags |= WdCam_FlyingOut;

		goto checkOpening;
	}

checkOpening:
	if (!(door->camFlags & WdCam_FlyingOut))
		return;

	// Both halves turn together; keys shrink for eleven frames before removal.
	if (door->doorRot.y < AH_DOOR_OPEN_ROTATION)
	{
		SVec3 leftRot;
		s16 shrinkScale[11] = {AH_DOOR_KEY_SHRINK_SCALES};
		door->doorRot.y += AH_DOOR_OPEN_ROTATION_STEP;
		desiredPos.x = door->doorRot.x;
		desiredPos.y = doorInst->instDef->rot.y - door->doorRot.y;
		desiredPos.z = door->doorRot.z;
		leftRot.x = door->doorRot.x;
		leftRot.y = doorInst->instDef->rot.y + door->doorRot.y;
		leftRot.z = door->doorRot.z;
		ConvertRotToMatrix(&doorInst->matrix, &leftRot);
		ConvertRotToMatrix(&door->otherDoor->matrix, &desiredPos);
		if (door->keyShrinkFrame < AH_DOOR_KEY_SHRINK_FRAME_COUNT)
		{
			for (i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
			{
				if (door->keyInst[i] != NULL)
				{
					door->keyInst[i]->scale.x = shrinkScale[door->keyShrinkFrame];
					door->keyInst[i]->scale.y = shrinkScale[door->keyShrinkFrame];
					door->keyInst[i]->scale.z = shrinkScale[door->keyShrinkFrame];
				}
			}

			door->keyShrinkFrame++;

			return;
		}
		for (i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
		{
			INSTANCE_Death(door->keyInst[i]);
			door->keyInst[i] = NULL;
		}

		return;
	}
	// Persist the newly opened route, then return camera, physics, and HUD control.
	levelID = GAME_TRACKER->levelID;
	beachLevel = N_SANITY_BEACH;

	if (((levelID == beachLevel) && (door->doorID == AH_DOOR_BEACH_TO_GLACIER_PARK)) || (levelID == THE_LOST_RUINS))
	{
		GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_STORY] |= ADV_REWARD_DOORS_TO_GLACIER_PARK_MASK;
	}

	else if ((levelID == beachLevel) && (door->doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY))
	{
		GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_STORY] |= ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK;
	}
	else if (GAME_TRACKER->levelID == GEM_STONE_VALLEY)
	{
		GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_STORY] |= ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK;
	}
	else
	{
		GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_STORY] |= ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK;
	}

	cDC->flags |= CAMERA_FLAG_TRANSITION_BACK;

	driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
	door->camFlags = (door->camFlags & ~WdCam_CutscenePlaying) | WdCam_FlyingIn;
	GAME_TRACKER->hudFlags = (u8)door->hudFlags;
	return;
doorOpened:
	if (cDC->flags & CAMERA_FLAG_TRANSITION_AWAY)
		return;
	if (AH_STORY_REWARDS & ADV_REWARD_HINT_NEW_WORLD_GREETING_MASK)
		return;
	hintId = ADV_MASK_HINT_ID_NEW_WORLD_GREETING;
requestHint:
	MainFrame_RequestMaskHint(hintId, 0);
}

void AH_Door_LInB(struct Instance *inst)
{
	s32 levelID;
	s32 ratio;
	s32 key;
	s16 digit;
	b16 doorIsOpen = false;
	SVec3 rightRot;
	SVec3 leftRot;
	struct Thread *t = inst->thread;
	struct Model *model;
	struct WoodDoor *door;

	if (t != NULL)
		return;

	t = inst->thread = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct WoodDoor), NONE, SMALL, STATIC), AH_Door_ThTick, AH_DOOR_NAME, 0);
	if (t == NULL)
		return;

	door = t->object;
	t->inst = inst;
	t->funcThDestroy = AH_Door_ThDestroy;

	// The left-hand door comes from the level; its partner is created here.
	inst->flags |= SPLIT_SPECIAL;
	for (key = AH_WOOD_DOOR_KEY_COUNT - 1; key >= 0; key--)
		door->keyInst[key] = NULL;

	door->frameCount_unused = 0;
	door->camFlags = WdCam_None;
	door->camTimer_unused = 0;
	door->frameCount_doorOpenAnim = 0;
	door->keyShrinkFrame = 0;
	door->doorRot.x = 0;
	door->doorRot.y = 0;
	door->doorRot.z = 0;
	door->doorID = 0;

	for (digit = 5; inst->name[digit] != '\0'; digit++)
	{
		s32 value;
		door->doorID *= 10;
		value = (u8)inst->name[digit] - '0';
		door->doorID = door->doorID + value;
	}

	levelID = GAME_TRACKER->levelID;
	if (levelID == GLACIER_PARK)
	{
		// Two key holes.
		door->otherDoor = INSTANCE_Birth3D(GAME_TRACKER->modelPtr[STATIC_DOOR3], AH_DOOR_NAME, t);
	}
	else if (levelID == N_SANITY_BEACH && door->doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY)
	{
		// No key holes.
		model = GAME_TRACKER->modelPtr[STATIC_DOOR2];
		if (model != NULL)
			door->otherDoor = INSTANCE_Birth3D(model, AH_DOOR_NAME, t);
		else
			door->otherDoor = NULL;
	}
	else
	{
		// One key hole.
		door->otherDoor = INSTANCE_Birth3D(GAME_TRACKER->modelPtr[STATIC_DOOR], AH_DOOR_NAME, t);
	}

	door->otherDoor->flags |= SPLIT_SPECIAL | REVERSE_CULL_DIRECTION;
	door->otherDoor->matrix = inst->matrix;
	door->otherDoor->scale.x = -FP_ONE;
	door->otherDoor->flags |= REVERSE_CULL_DIRECTION;

	ratio = MATH_Cos(inst->instDef->rot.y) * AH_DOOR_PAIR_OFFSET;
	door->otherDoor->matrix.t[0] = inst->matrix.t[0] + (ratio >> 12);
	door->otherDoor->matrix.t[1] = inst->matrix.t[1];
	ratio = MATH_Sin(inst->instDef->rot.y) * AH_DOOR_PAIR_OFFSET;
	door->otherDoor->matrix.t[2] = inst->matrix.t[2] + (ratio >> 12);

	inst->model->headers->flags |= 2;
	door->otherDoor->model->headers->flags |= 2;

	if (GAME_TRACKER->levelID == N_SANITY_BEACH && door->doorID == AH_DOOR_BEACH_TO_GLACIER_PARK &&
	    (AH_STORY_REWARDS & ADV_REWARD_DOOR_BEACH_TO_GLACIER_PARK_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == N_SANITY_BEACH && door->doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY &&
	         (AH_STORY_REWARDS & ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == GEM_STONE_VALLEY && (AH_STORY_REWARDS & ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == THE_LOST_RUINS && (AH_STORY_REWARDS & ADV_REWARD_DOOR_LOST_RUINS_TO_GLACIER_PARK_MASK))
		doorIsOpen = true;
	else if (GAME_TRACKER->levelID == GLACIER_PARK && (AH_STORY_REWARDS & ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK))
		doorIsOpen = true;
    if (g_config.unlockAllGates)
    {
        doorIsOpen = true;
    }
	if ((s16)doorIsOpen)
	{
		door->doorRot.y = AH_DOOR_OPEN_ROTATION;

		rightRot.x = door->doorRot.x;
		rightRot.y = inst->instDef->rot.y - door->doorRot.y;
		rightRot.z = door->doorRot.z;
		leftRot.x = door->doorRot.x;
		leftRot.y = inst->instDef->rot.y + door->doorRot.y;
		leftRot.z = door->doorRot.z;

		ConvertRotToMatrix(&inst->matrix, &leftRot);
		ConvertRotToMatrix(&door->otherDoor->matrix, &rightRot);
	}
}

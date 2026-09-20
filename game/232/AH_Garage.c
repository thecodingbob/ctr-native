#include <common.h>

enum AHGarageConstants
{
	AH_GARAGE_ACCESS_FLAG = 1,
	AH_GARAGE_NORMAL_OPEN_SFX = 0x95,
	AH_GARAGE_OXIDE_OPEN_SFX = 0x96,
	AH_GARAGE_DOOR_MOVE_STEP = 0x20,
	AH_GARAGE_DOOR_HEIGHT = 0x300,
	AH_GARAGE_DOOR_COOLDOWN_MS = 2 * SECOND,
	AH_GARAGE_TOP_ROT_STEP = 0x40,
	AH_GARAGE_CHALLENGE_NEAR_DIST_MAX_SQ = 0x143fff,
	AH_GARAGE_BOSS_COLLIDE_RADIUS = 0x300,
	AH_GARAGE_BOSS_COLLIDE_RADIUS_SQ = 0x90000,
	AH_GARAGE_INTERIOR_FORWARD_OFFSET = -0x280,
	AH_GARAGE_INTERIOR_DIST_SQ = 0x40000,
	AH_GARAGE_FADE_STEP = -0x2aa,
	AH_GARAGE_OXIDE_FINAL_BOSS_ID = 5,
	AH_GARAGE_TOP_FORWARD_OFFSET = 0x4c,
	AH_GARAGE_TOP_DEPTH_BIAS = 0xfe,
};

void AH_Garage_ThDestroy(struct Thread *t)
{
	struct BossGarageDoor *garage = t->object;

	if (garage->garageTopInst != NULL)
	{
		INSTANCE_Death(garage->garageTopInst);
		garage->garageTopInst = NULL;
	}

	return;
}

void AH_Garage_Open(struct ScratchpadStruct *sps, void *hitObject)
{
	struct Thread *garageThread;
	struct BossGarageDoor *garage;
	struct Instance *garageInst;
	struct Thread *otherTh = hitObject;

	if (otherTh->modelIndex != DYNAMIC_PLAYER)
	{
		return;
	}

	garageThread = sps->Union.ThBuckColl.thread;
	garage = garageThread->object;
	garageInst = garageThread->inst;

	if (
	    // if door is not opening
	    (garage->direction != BOSS_GARAGE_DOOR_OPENING) &&

	    // if door is closed,
	    // if posY is the same as instDef posY
	    (garageInst->matrix.t[1] == garageInst->instDef->pos.y))
	{
		u32 soundID = AH_GARAGE_NORMAL_OPEN_SFX;

		// if you are not in gemstone valley
		// play sound of normal boss door opening

		// Level ID
		// if you are in Gemstone Valley
		if (GAME_TRACKER->levelID == GEM_STONE_VALLEY)
		{
			// play sound of oxide door opening
			soundID = AH_GARAGE_OXIDE_OPEN_SFX;
		}

		// Play sound
		OtherFX_Play(soundID, 1);
	}

	// door is now opening
	garage->direction = BOSS_GARAGE_DOOR_OPENING;

	// enable access through a door (disable collision)
	GAME_DOOR_ACCESS_FLAGS |= AH_GARAGE_ACCESS_FLAG;
}

void AH_Garage_ThTick(struct Thread *t)
{
	s16 raceLevels[5] = {AH_BOSS_RACE_LEVELS};
	struct GameTracker *gGT = GAME_TRACKER;
	s16 bosses[5] = {AH_BOSS_CHARACTER_IDS};
	struct PushBuffer *view;
	b32 bossIsOpen = true;
	s32 i;
	s32 top;
	s32 move;
	s32 ratio;
	s32 bottom;
	u32 soundID;
	s32 closeLevel;
	s32 distX, distY, distZ;

	struct BossGarageDoor *garage;
	struct Instance *inst;
	struct Instance *drv_inst;
	struct ScratchpadStruct *sps;

	garage = t->object;
	inst = t->inst;
	drv_inst = gGT->drivers[0]->instSelf;
	view = &gGT->pushBuffer[0];

	// Animate the moving door.
	if (garage->direction != BOSS_GARAGE_DOOR_STOPPED)
	{
		// Increment animation by 0x20 in either direction
		move = inst->matrix.t[1] + garage->direction * AH_GARAGE_DOOR_MOVE_STEP;
		inst->matrix.t[1] = move;

		top = inst->instDef->pos.y + AH_GARAGE_DOOR_HEIGHT;
		bottom = inst->instDef->pos.y;

		// If the door has gone past the top (height=0x300)
		if (move > top)
		{
			// Set position to the top
			inst->matrix.t[1] = top;

			// Door is now open (not moving)
			garage->direction = BOSS_GARAGE_DOOR_STOPPED;

			// Cooldown for 2 seconds
			garage->cooldown = AH_GARAGE_DOOR_COOLDOWN_MS;

			// Make invisible
			inst->flags |= HIDE_MODEL;
		}
		// If the door has gone past the bottom
		else if (move < bottom)
		{
			// Set position to the bottom
			inst->matrix.t[1] = bottom;

			garage->direction = BOSS_GARAGE_DOOR_STOPPED;

			garage->cooldown = 0;

			// Enable door collision
			GAME_DOOR_ACCESS_FLAGS &= ~AH_GARAGE_ACCESS_FLAG;
		}
		// If the door is between the top and bottom positions
		else if (garage->garageTopInst != 0)
		{
			// Update rotation of garagetop
			garage->rot.x += garage->direction * AH_GARAGE_TOP_ROT_STEP;

			ConvertRotToMatrix(&garage->garageTopInst->matrix, &garage->rot);
		}

		inst->flags = (inst->flags | SPLIT_LINE | REFLECTION_FUNC23 | WATER_SPLIT_WHITE) & ~SPLIT_SPECIAL;
	}
	else
	{
		// Count down before closing the open door.
		if (garage->cooldown != 0)
		{
			// subtract frame timer
			garage->cooldown -= gGT->elapsedTimeMS;

			// if countdown is not done, dont close door
			if (garage->cooldown > 0)
			{
				goto checkRewards;
			}

			// play sound of normal boss door opening, except for Oxide
			// NOTE(aalhendi): Finish reading the tracker before selecting the sound
			// argument; this preserves retail register lifetimes without fixed registers.
			closeLevel = gGT->levelID;
			CTR_PSX_OBSERVE_VALUE(closeLevel);
			soundID = closeLevel == GEM_STONE_VALLEY ? AH_GARAGE_OXIDE_OPEN_SFX : AH_GARAGE_NORMAL_OPEN_SFX;

			// Play sound
			OtherFX_Play(soundID, 1);

			// erase cooldown
			garage->cooldown = 0;

			// door is closing
			garage->direction = BOSS_GARAGE_DOOR_CLOSING;

			inst->flags &= ~HIDE_MODEL;
		}
		else
		{
			inst->flags |= SPLIT_SPECIAL;
			inst->flags &= ~(SPLIT_LINE | REFLECTION_FUNC23 | WATER_SPLIT_WHITE);
		}
	}
checkRewards:

if (!g_config.unlockAllPortals) {
  {
    s32 hubLevel = GAME_TRACKER->levelID;
    i = 0;
    if (hubLevel == GEM_STONE_VALLEY)
    {
      for (; i < AH_BOSS_KEY_COUNT; i++)
      {
        if (!CHECK_ADV_BIT(GAME_ADV_PROGRESS.rewards, i + ADV_REWARD_FIRST_BOSS_KEY))
        {
          bossIsOpen = false;
          break;
        }
      }
    }
    else
    {
      u32 *rewards = GAME_ADV_PROGRESS.rewards;
      const s16 *tracks = AH_HUB_TRACK_IDS;
      s32 byteOffset = (hubLevel - N_SANITY_BEACH) * AH_HUB_TRACK_COUNT * sizeof(s16);
      for (; i < AH_HUB_TRACK_COUNT; i++)
      {
        if (!CHECK_ADV_BIT(rewards, *(s16 *)(byteOffset + (u32)tracks) + ADV_REWARD_FIRST_TROPHY))
        {
          bossIsOpen = false;
          break;
        }
        byteOffset += sizeof(s16);
      }
    }
  }
}


	distX = drv_inst->matrix.t[0] - inst->instDef->pos.x;
	distY = drv_inst->matrix.t[1] - inst->instDef->pos.y;
	distZ = drv_inst->matrix.t[2] - inst->instDef->pos.z;

	i = distX * distX + distY * distY + distZ * distZ;
	// if in a state where you're seeing the boss key open an adv door,
	// or some other kind of cutscene where you can't move
	if ((GAME_TRACKER->gameMode2 & 4) != 0)
	{
		return;
	}

	// check distance
	if (AH_GARAGE_CHALLENGE_NEAR_DIST_MAX_SQ < i)
	{
		goto checkUnlocked;
	}

	{
		s16 challengeBosses[5] = {AH_BOSS_CHARACTER_IDS};

		// if aku is not giving a hint
		if (AH_HINT_STATE == 0)
		{
			// draw string, lng_challenge
			DecalFont_DrawLine(

			    GAME_LANGUAGE_STRINGS[AH_BOSS_CHALLENGE_TEXT[challengeBosses[GAME_TRACKER->levelID - GEM_STONE_VALLEY]]],

			    view->rect.x + (view->rect.w >> 1), ((view->rect.y + view->rect.h) - 0x1e), FONT_BIG, JUSTIFY_CENTER | ORANGE);
		}
	}
	if ((s16)bossIsOpen)
	{
		goto openGarage;
	}

	// if this is gemstone valley
	if (GAME_TRACKER->levelID == GEM_STONE_VALLEY)
	{
		// if hint is not unlocked "need 4 keys for oxide"
		if (!(AH_STORY_REWARDS & (1u << (ADV_REWARD_HINT_NEED_FOUR_KEYS_FOR_OXIDE & 31))))
		{
			// HintID: need four keys to race oxide
			MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_NEED_FOUR_KEYS_FOR_OXIDE, 0);
		}
	}
	// not gemstone valley
	else
	{
		//  if hint is not unlocked "to access this boss garage..."
		if (!(AH_STORY_REWARDS & (1u << (ADV_REWARD_HINT_NEED_FOUR_TROPHIES_FOR_BOSS & 31))))
		{
			// HintID: need four trophies to enter boss
			MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_NEED_FOUR_TROPHIES_FOR_BOSS, 0);
		}
	}

checkUnlocked:

	if (!(s16)bossIsOpen)
	{
		return;
	}

openGarage:
{
	Vec3 pos;

	sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
	sps->Input1.pos.x = inst->instDef->pos.x;
	sps->Input1.pos.y = inst->instDef->pos.y;
	sps->Input1.pos.z = inst->instDef->pos.z;
	sps->Input1.hitRadius = AH_GARAGE_BOSS_COLLIDE_RADIUS;
	sps->Input1.hitRadiusSquared = AH_GARAGE_BOSS_COLLIDE_RADIUS_SQ;

	sps->Union.ThBuckColl.thread = t;
	sps->Union.ThBuckColl.funcCallback = AH_Garage_Open;
	sps->Input1.modelID = STATIC_PINGARAGE;

	// Open garage door when player gets within radius of door
	PROC_CollideHitboxWithBucket(GAME_TRACKER->threadBuckets[PLAYER].thread, sps, 0);

	ratio = MATH_Sin((int)inst->instDef->rot.y);

	pos.x = (int)inst->instDef->pos.x + (ratio * AH_GARAGE_INTERIOR_FORWARD_OFFSET >> 0xc);
	pos.y = (int)inst->instDef->pos.y;

	ratio = MATH_Cos((int)inst->instDef->rot.y);

	pos.z = (int)inst->instDef->pos.z + (ratio * AH_GARAGE_INTERIOR_FORWARD_OFFSET >> 0xc);

	// DriverPos - DoorPos
	distX = drv_inst->matrix.t[0] - pos.x;
	distY = drv_inst->matrix.t[1] - pos.y;
	distZ = drv_inst->matrix.t[2] - pos.z;

	// If small distance (inside garage)
	i = distX * distX + distY * distY + distZ * distZ;
	if (i < AH_GARAGE_INTERIOR_DIST_SQ)
	{
		// Fade To Black
		GAME_TRACKER->pushBuffer_UI.fadeFromBlack_desiredResult = 0;
		GAME_TRACKER->pushBuffer_UI.fade_step = AH_GARAGE_FADE_STEP;
	}

	// If fade complete, start loading level
	if (GAME_TRACKER->pushBuffer_UI.fadeFromBlack_currentValue == 0)
	{
		GAME_REMOVE_CONFIG_0 |= ADVENTURE_ARENA;
		GAME_ADD_CONFIG_0 |= ADVENTURE_BOSS;

		if ((GAME_TRACKER->levelID == GEM_STONE_VALLEY) && (GAME_TRACKER->currAdvProfile.numRelics == ADV_OXIDE_FINAL_RELIC_COUNT))
		{
			// set string index (0-5) to "N Oxide's Final Challenge"
			GAME_TRACKER->bossID = AH_GARAGE_OXIDE_FINAL_BOSS_ID;
		}

		else
		{
			GAME_TRACKER->bossID = bosses[GAME_TRACKER->levelID - GEM_STONE_VALLEY];
		}

		// Set the boss character (P2)
		GAME_CHARACTER_IDS[1] = AH_LEVEL_METADATA[raceLevels[GAME_TRACKER->levelID - GEM_STONE_VALLEY]].characterID_Boss;

		// NOTE(aalhendi): Retail reloads the destination after the callback.
		RaceFlag_SetDrawOrder(1);
		MainRaceTrack_RequestLoad(raceLevels[GAME_TRACKER->levelID - GEM_STONE_VALLEY]);
	}
}
	return;
}

void AH_Garage_LInB(struct Instance *inst)
{
	b32 bossIsOpen = true;
	s32 i;
	s16 bitIndex;
	s32 ratio;
	struct Thread *t = inst->thread;
	struct Instance *garageTop;
	struct BossGarageDoor *garage;
	s32 levelID;

	if (t != NULL)
	{
		return;
	}

	t = inst->thread = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct BossGarageDoor), NONE, SMALL, STATIC),

	                                        AH_Garage_ThTick, // behavior
	                                        AH_GARAGE_NAME,   // debug name
	                                        0                 // thread relative
	);

	if (t == NULL)
	{
		return;
	}

	t->inst = inst;
	t->funcThDestroy = AH_Garage_ThDestroy;

	garage = t->object;
	garage->direction = BOSS_GARAGE_DOOR_STOPPED;
	garage->cooldown = 0;

	// if this is not oxide's door
	if (inst->model->id != STATIC_OXIDEGARAGE)
	{
		// make a "garagetop" to make door appear to roll up

		garageTop = INSTANCE_Birth3D(GAME_TRACKER->modelPtr[STATIC_GARAGETOP], AH_GARAGE_TOP_NAME, t);

		garage->garageTopInst = garageTop;
		garageTop->matrix = inst->matrix;

		ratio = MATH_Sin((int)inst->instDef->rot.y) * AH_GARAGE_TOP_FORWARD_OFFSET;

		// continue setting GarageTop position
		garage->garageTopInst->matrix.t[0] = inst->matrix.t[0] + (ratio >> 12);
		garage->garageTopInst->matrix.t[1] = inst->matrix.t[1] + AH_GARAGE_DOOR_HEIGHT;

		ratio = MATH_Cos((int)inst->instDef->rot.y) * AH_GARAGE_TOP_FORWARD_OFFSET;

		// continue setting GarageTop position
		garage->garageTopInst->matrix.t[2] = inst->matrix.t[2] + (ratio >> 12);

		garage->garageTopInst->depthBiasNormal = AH_GARAGE_TOP_DEPTH_BIAS;
	}
	else
	{
		garage->garageTopInst = NULL;
	}

        if (!g_config.unlockAllPortals) {
          levelID = GAME_TRACKER->levelID;
          i = 0;
          if (levelID == GEM_STONE_VALLEY)
          {
            // ripper roo boss key
            // check all boss keys
            for (; i < AH_BOSS_KEY_COUNT; i++)
            {
              if (!CHECK_ADV_BIT(GAME_ADV_PROGRESS.rewards, i + ADV_REWARD_FIRST_BOSS_KEY))
              {
                bossIsOpen = false;
                break;
              }
            }
          }

          // if not gemstone valley
          else
          {
            u32 *rewards = GAME_ADV_PROGRESS.rewards;
            const s16 *tracks = AH_HUB_TRACK_IDS;
            s32 trackOffset = (levelID - N_SANITY_BEACH) * AH_HUB_TRACK_COUNT * sizeof(s16);
            // NOTE(aalhendi): Both targets have 32-bit addresses. Keep the byte
            // offset separate from the table base, as in retail's trophy traversal.
            // check all tracks on hub
            for (; i < AH_HUB_TRACK_COUNT; i++)
            {
              // if any trophy on this hub is not unlocked
              if (!CHECK_ADV_BIT(rewards, *(s16 *)(trackOffset + (u32)tracks) + ADV_REWARD_FIRST_TROPHY))
              {
                // boss is not open
                bossIsOpen = false;
                break;
              }
              trackOffset += sizeof(s16);
            }
          }
        }

	// if boss is open
	if ((s16)bossIsOpen)
	{
		u32 *rewards = GAME_ADV_PROGRESS.rewards;
		// check if boss was NOT beaten on this hub (levelID - 0x19)
		bitIndex = AH_BOSS_REWARDS[GAME_TRACKER->levelID - GEM_STONE_VALLEY];
		if (CHECK_ADV_BIT(rewards, (s16)bitIndex))
		{
			t->modelIndex = 2;
		}
		else
		{
			t->modelIndex = 1;
		}
	}

	// if boss is not open
	else
	{
		// locked
		t->modelIndex = 0;
	}

	garage->rot.x = inst->instDef->rot.x;
	garage->rot.y = inst->instDef->rot.y;
	garage->rot.z = inst->instDef->rot.z;

	inst->depthBiasNormal = 1;
	inst->vertSplit = inst->instDef->pos.y + AH_GARAGE_DOOR_HEIGHT;
	inst->specLightX = 0;
	inst->depthBiasSecondary = inst->depthBiasNormal;
}

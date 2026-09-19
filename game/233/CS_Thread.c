#include <common.h>
#include <ctr_gte_transfer.h>

struct CSThreadParentFrameScratch
{
	SVec3Slot parentPos;
	u8 pad_110[0x08];
	SVec3Slot parentRot;
};

enum CsPathModelKind
{
	CS_PATH_MODEL_PPOINT_THING_INTRO = STATIC_PPOINTTHINGINTRO - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_PR_THING_INTRO = STATIC_PRTHINGINTRO - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_OXIDE_LIL_SHIP = STATIC_OXIDELILSHIP - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_COCO_SELECT = STATIC_COCOSELECT - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_END_OXIDE_BIG_SHIP = STATIC_ENDOXIDEBIGSHIP - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_END_OXIDE_LIL_SHIP = STATIC_ENDOXIDELILSHIP - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_OXIDE_SPEAKER = STATIC_OXIDESPEAKER - STATIC_PPOINTTHINGINTRO,
	CS_PATH_MODEL_KIND_COUNT = STATIC_OXIDESPEAKER - STATIC_PPOINTTHINGINTRO + 1,
};

enum CsThreadConstants
{
	CS_PODIUM_SECOND_HIDE_START_FRAME = 0x65,
	CS_PODIUM_SECOND_HIDE_FRAME_COUNT = 0x87,
	CS_PODIUM_SECOND_DEPTH_BIAS_RESTORE = 2,
	CS_PODIUM_FIRST_HIDE_START_FRAME = 0x83,
	CS_PODIUM_FIRST_HIDE_FRAME_COUNT = 0x69,
	CS_PODIUM_FIRST_DEPTH_BIAS_RESTORE = 6,
	CS_FADE_FROM_BLACK_CURRENT = 0x1fff,
	CS_FADE_FROM_BLACK_TARGET = FP_ONE,
	CS_FADE_FROM_BLACK_STEP = -0x2aa,
	CS_RANDOM_DURATION_SHIFT = 2,
	CS_RANDOM_DURATION_MASK = ANG_TWO_PI - 1,
	CS_FRAME32_SHIFT = 5,
	CS_FRAME32_UNIT = 1 << CS_FRAME32_SHIFT,
	CS_FRAME32_MASK = CS_FRAME32_UNIT - 1,
	CS_CAMERA_PATH_TIME_NUMERATOR_SHIFT = 11,
	CS_CAMERA_PATH_TIME_DENOMINATOR_SHIFT = 16,
	CS_CAMERA_DISTANCE_DEFAULT = 0x100,
	CS_CAMERA_DISTANCE_CLOSE = 0x50,
	CS_CAMERA_DISTANCE_OXIDE_SHIP = 0x278,
	CS_CAMERA_DISTANCE_INTRO_WIDE = 0x1eb,
	CS_CAMERA_DISTANCE_INTRO_MEDIUM = 0x14d,
	CS_ND_CRATE_CAMERA_DISTANCE = 0x140,
	CS_ND_CRATE_SKIP_MIN_FRAME32 = 0xb5,
	CS_CREDITS_LEVEL_COUNT = SCRAPBOOK - CREDITS_CRASH,
	CS_OXIDE_ENDING_LEVEL_COUNT = OXIDE_TRUE_ENDING - OXIDE_ENDING + 1,
	CS_ADV_HUB_LEVEL_COUNT = INTRO_RACE_TODAY - GEM_STONE_VALLEY,
	CS_INTRO_MODEL_SCRIPT_COUNT = 0x10,
	CS_BOX_MODEL_SCRIPT_COUNT = 0x2b,
	CS_DANCE_MODEL_SCRIPT_COUNT = 0x10,
	CS_TAWNA_MODEL_SCRIPT_COUNT = 4,
	CS_PINHEAD_DEFAULT_SCRIPT_COUNT = 5,
	CS_KART_FRAME_OVERRIDE_COUNT = 4,
	CS_MASK_KART_MODEL_COUNT = 2,
	CS_CREDITS_ND_CRATE_SCRIPT_OFFSET = 0x18,
	CS_TRUE_ENDING_SCRIPT_OFFSET = 0x30,
	CS_DEFAULT_SCALE = FP_ONE,
	CS_PODIUM_CHARACTER_SCALE = 0x2800,
	CS_DEFAULT_PARTICLE_ID = -1,
	CS_DEFAULT_COLOR_WORD = 0x2e808080,
	CS_RANDOM_BYTE_MASK = 0xff,
	CS_ANIM_INDEX_LOW_BYTE_MASK = 0xff,
	CS_CONDITION_RESULT_MASK = 0xffff,
	CS_RANDOM_ALPHA_MASK = 0x7ff,
	CS_RANDOM_ALPHA_BASE = 0x400,
	CS_MODEL_LOD_DISTANCE_VISIBLE = 20000,
	CS_RANDOM_CLEAR_BOX_MASK = 0xf,
	CS_INTERPOLATE_LINE_COLOR_CODE = 0x42000000,
	CS_INTERPOLATE_LINE_OT_TAG = 0x05000000,
	CS_INTERPOLATE_LINE_DEPTH_MIN = 1,
	CS_INTERPOLATE_LINE_DEPTH_RANGE = 0x11ff,
	CS_INTERPOLATE_LINE_FADE_START_DEPTH = 0xa00,
	CS_INTERPOLATE_LINE_FADE_END_DEPTH = 0x1200,
	CS_INTERPOLATE_LINE_MAX_COLOR = 0x3f,
	CS_INTERPOLATE_LINE_FADE_SHIFT = 11,
	CS_INTERPOLATE_LINE_OT_SHIFT = 6,
	CS_INTERPOLATE_LINE_MAX_OT_INDEX = 0x3ff,
	CS_SUBTITLE_TEXT_MAX_WIDTH = 460,
	CS_SUBTITLE_BOX_HALF_WIDTH = 236,
	CS_SUBTITLE_BOX_Y_OFFSET = 4,
	CS_SUBTITLE_BOX_WIDTH = 472,
	CS_SUBTITLE_BOX_HEIGHT_PADDING = 8,
	CS_SUBTITLE_BOX_DRAW_STYLE = 4,
	CS_XA_ANIM_SYNC_NUMERATOR = 0x1e00,
	CS_XA_ANIM_SYNC_DENOMINATOR = 0xac44,
	CS_AUDIO_VOLUME_FX_BYTE_OFFSET = 2,
	CS_AUDIO_VOLUME_MUSIC_BYTE_OFFSET = 4,
	CS_AUDIO_VOLUME_VOICE_BYTE_OFFSET = 6,
	CS_BOSS_END_OVERLAY_TRANSITION_FRAMES = 3,
};

CTR_STATIC_ASSERT(offsetof(struct CSThreadParentFrameScratch, parentPos) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct CSThreadParentFrameScratch, parentRot) == 0x10);
CTR_STATIC_ASSERT((u16)CS_FADE_FROM_BLACK_STEP == 0xfd56);
CTR_STATIC_ASSERT(CS_INTRO_MODEL_SCRIPT_COUNT == sizeof(D233.introModelScripts) / sizeof(D233.introModelScripts[0]));
CTR_STATIC_ASSERT(CS_BOX_MODEL_SCRIPT_COUNT == sizeof(D233.boxModelScripts) / sizeof(D233.boxModelScripts[0]));
CTR_STATIC_ASSERT(CS_DANCE_MODEL_SCRIPT_COUNT == sizeof(D233.danceFirstScripts) / sizeof(D233.danceFirstScripts[0]));
CTR_STATIC_ASSERT(CS_DANCE_MODEL_SCRIPT_COUNT == sizeof(D233.danceOtherScripts) / sizeof(D233.danceOtherScripts[0]));
CTR_STATIC_ASSERT(CS_KART_FRAME_OVERRIDE_COUNT ==
                  sizeof(((struct OverlayDATA_233 *)0)->cs_initMatrixTable) / sizeof(((struct OverlayDATA_233 *)0)->cs_initMatrixTable[0]));
CTR_STATIC_ASSERT(CS_INTERPOLATE_LINE_FADE_END_DEPTH - CS_INTERPOLATE_LINE_FADE_START_DEPTH == 0x800);
CTR_STATIC_ASSERT(OFFSETOF(struct CsOpcodeMeta, animIndex) == CS_AUDIO_VOLUME_FX_BYTE_OFFSET);
CTR_STATIC_ASSERT(OFFSETOF(struct CsOpcodeMeta, frameStart) == CS_AUDIO_VOLUME_MUSIC_BYTE_OFFSET);
CTR_STATIC_ASSERT(OFFSETOF(struct CsOpcodeMeta, frameEnd) == CS_AUDIO_VOLUME_VOICE_BYTE_OFFSET);

#define CS_INTERPOLATE_LINE_DRAW_MODE 0xe1000a20u

int CS_Thread_UseOpcode(struct Instance *instance, struct CutsceneObj *cs)
{
	u8 numPlayers;
	b32 frameBoundaryHit;
	s16 numCamPathPoints;
	u16 clockEffectFlags;
	u16 cutsceneFlags;
	s32 lodIndexState;
	char *const *cutsceneOpcodes;
	s16 levelToLoad;
	s32 distanceToScreen;
	struct Thread *dancerThread;
	register s32 animFrame32 CTR_PSX_REGISTER("$22");
	struct CsOpcodeMeta *opcodeMeta;

	struct CsInitMatrixEntry *frameData;
	s32 nextFrameTime;
	struct ModelHeader *modelHeader;
	struct CsOpcodeMeta metadataBackup;
	SVec3 camRot;
	SVec3 camPos;
	s16 camPathFlags;
	s32 animIndex;
	s32 opcodeDuration;
	b32 opcodeChanged;
	s32 elapsedTimeRemaining;

	struct GameTracker *gGT = GAME_TRACKER;
	// NOTE(aalhendi): The interpreter updates decodedOpcode in place, but restores this entry snapshot when yielding.
	metadataBackup = cs->decodedOpcode;

	if (instance != 0)
	{
		if ((instance->flags & SPLIT_LINE) != 0)
		{
			instance->vertSplit = CS_VERT_SPLIT;
		}

		if ((s32)instance->model->id == (s32)(u8)gGT->podium_modelIndex_Second)
		{
			if ((u32)(CS_PODIUM_CAMERA_FRAME - CS_PODIUM_SECOND_HIDE_START_FRAME) < CS_PODIUM_SECOND_HIDE_FRAME_COUNT)
			{
				instance->flags |= HIDE_MODEL;
			}
			else
			{
				if ((instance->flags & HIDE_MODEL) == 0)
				{
					goto afterPodiumSecondModelCheck;
				}
				instance->depthBiasNormal -= CS_PODIUM_SECOND_DEPTH_BIAS_RESTORE;
				instance->depthBiasSecondary -= CS_PODIUM_SECOND_DEPTH_BIAS_RESTORE;
				instance->flags &= ~HIDE_MODEL;
			}
		}
	afterPodiumSecondModelCheck:

		if ((s32)instance->model->id == (s32)(u8)gGT->podium_modelIndex_First)
		{
			if ((u32)(CS_PODIUM_CAMERA_FRAME - CS_PODIUM_FIRST_HIDE_START_FRAME) < CS_PODIUM_FIRST_HIDE_FRAME_COUNT)
			{
				instance->flags |= HIDE_MODEL;
			}
			else
			{
				if ((instance->flags & HIDE_MODEL) == 0)
				{
					goto afterPodiumFirstModelCheck;
				}
				instance->depthBiasNormal -= CS_PODIUM_FIRST_DEPTH_BIAS_RESTORE;
				instance->depthBiasSecondary -= CS_PODIUM_FIRST_DEPTH_BIAS_RESTORE;
				instance->flags &= ~HIDE_MODEL;
			}
		}
	afterPodiumFirstModelCheck:

		if ((cs->flags & CS_FLAG_ADV_CHAR_SELECT_LOGIC) != 0)
		{
			if (((s32)instance->model->id - STATIC_CRASHSELECT == (s32)CS_GARAGE_CHARACTERS[CS_GARAGE_CURRENT]) && (CS_GARAGE_SELECTED == 1))
			{
				if ((cs->flags & CS_FLAG_ADV_CHAR_SELECT_SELECTED) == 0)
				{
					gGT->pushBuffer[0].fadeFromBlack_currentValue = CS_FADE_FROM_BLACK_CURRENT;
					gGT->pushBuffer[0].fadeFromBlack_desiredResult = CS_FADE_FROM_BLACK_TARGET;
					gGT->pushBuffer[0].fade_step = CS_FADE_FROM_BLACK_STEP;
					cs->flags |= CS_FLAG_ADV_CHAR_SELECT_SELECTED;
					CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(advCharSelectSelectOpcodes)[(s32)instance->model->id - STATIC_CRASHSELECT]);
					metadataBackup = cs->decodedOpcode;
					{
						s32 rng;
						struct CsOpcodeMeta *garageMeta;

						cs->animFrame32 = cs->metadataMeta->arg0.i;
						rng = MixRNG_Scramble();
						garageMeta = cs->metadataMeta;

						cs->opcodeDuration = garageMeta->frameStart + (s16)((s32)(((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) *
						                                                          (((s32)garageMeta->frameEnd - (s32)garageMeta->frameStart) + 1)) >>
						                                                    FRACTIONAL_BITS);
					}
				}
			}
			else
			{
				if ((cs->flags & CS_FLAG_ADV_CHAR_SELECT_SELECTED) != 0)
				{
					cs->flags &= ~CS_FLAG_ADV_CHAR_SELECT_SELECTED;
					CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(advCharSelectDeselectOpcodes)[(s32)instance->model->id - STATIC_CRASHSELECT]);
					metadataBackup = cs->decodedOpcode;
					{
						s32 rng;
						struct CsOpcodeMeta *garageMeta;

						cs->animFrame32 = cs->metadataMeta->arg0.i;
						rng = MixRNG_Scramble();
						garageMeta = cs->metadataMeta;

						cs->opcodeDuration = garageMeta->frameStart + (s16)((s32)(((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) *
						                                                          (((s32)garageMeta->frameEnd - (s32)garageMeta->frameStart) + 1)) >>
						                                                    FRACTIONAL_BITS);
					}
				}
			}
		}
	}

	animFrame32 = cs->animFrame32;
	lodIndexState = (s32)cs->lodIndex;
	opcodeDuration = (s32)cs->opcodeDuration;
	elapsedTimeRemaining = gGT->elapsedTimeMS;
	opcodeMeta = cs->metadataMeta;
	animIndex = (s32)opcodeMeta->animIndex;

	if (instance == 0)
	{
		struct PushBuffer *pushBuffer = &gGT->pushBuffer[0];
		numCamPathPoints = CAM_Path_GetNumPoints();
		if ((s32)numCamPathPoints != 0)
		{
			s32 cameraPathFrame = ((s32)((u32)gGT->msInThisLEV << CS_CAMERA_PATH_TIME_NUMERATOR_SHIFT)) >> CS_CAMERA_PATH_TIME_DENOMINATOR_SHIFT;
			if (cameraPathFrame >= (s32)numCamPathPoints + -1)
			{
				if (opcodeMeta->opcode == CS_OPCODE_YIELD)
				{
					CS_ScriptCmd_OpcodeNext(cs);
				}
				CAM_Path_Move((s32)(s16)(numCamPathPoints + -1), CTR_VECTOR_DATA(&(pushBuffer->pos)), CTR_VECTOR_DATA(&(pushBuffer->rot)), &camPathFlags);
			}
			else
			{
				CAM_Path_Move(cameraPathFrame, CTR_VECTOR_DATA(&(camPos)), CTR_VECTOR_DATA(&(camRot)), &camPathFlags);
				pushBuffer->pos.x = camPos.x;
				pushBuffer->pos.y = camPos.y;
				pushBuffer->pos.z = camPos.z;
				pushBuffer->rot.x = camRot.x;
				pushBuffer->rot.y = camRot.y;
				pushBuffer->rot.z = camRot.z;
			}

			clockEffectFlags = gGT->clockEffectEnabled & ~CAM_PATH_FLAG_CLOCK_EFFECT;
			gGT->clockEffectEnabled = clockEffectFlags;
			if ((camPathFlags & CAM_PATH_FLAG_CLOCK_EFFECT) != 0)
			{
				gGT->clockEffectEnabled = clockEffectFlags | CAM_PATH_FLAG_CLOCK_EFFECT;
			}

			if ((cs->flags & CS_FLAG_CAMERA_DISTANCE_OVERRIDE) == 0)
			{
				pushBuffer->distanceToScreen_PREV = CS_CAMERA_DISTANCE_DEFAULT;
				if ((camPathFlags & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x50) != 0)
				{
					pushBuffer->distanceToScreen_PREV = CS_CAMERA_DISTANCE_CLOSE;
				}
				if ((camPathFlags & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x278) != 0)
				{
					pushBuffer->distanceToScreen_PREV = CS_CAMERA_DISTANCE_OXIDE_SHIP;
				}
				if ((camPathFlags & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x1EB) != 0)
				{
					pushBuffer->distanceToScreen_PREV = CS_CAMERA_DISTANCE_INTRO_WIDE;
				}
				if ((camPathFlags & CAM_PATH_FLAG_DISTANCE_TO_SCREEN_0x14D) != 0)
				{
					pushBuffer->distanceToScreen_PREV = CS_CAMERA_DISTANCE_INTRO_MEDIUM;
				}
			}

			if (((camPathFlags & CAM_PATH_FLAG_RANDOM_CLEAR_BOX) != 0) && ((MixRNG_Scramble() & CS_RANDOM_CLEAR_BOX_MASK) == 0))
			{
				CTR_Box_DrawClearBox(&CS_SCRIPT(introClearBoxRect), &CS_SCRIPT(introClearBoxColor), 1, gGT->backBuffer->otMem.uiOT, &gGT->backBuffer->primMem);
			}

			if (GAME_TRACKER->levelID == NAUGHTY_DOG_CRATE)
			{
				pushBuffer->distanceToScreen_PREV = CS_ND_CRATE_CAMERA_DISTANCE;
			}

			pushBuffer->distanceToScreen_CURR = pushBuffer->distanceToScreen_PREV;
		}

		if ((GAMEPADS->gamepad[0].buttonsTapped & BTN_START) != 0)
		{
			gGT->clockEffectEnabled &= ~CAM_PATH_FLAG_CLOCK_EFFECT;
			if ((u32)(GAME_TRACKER->levelID - CREDITS_CRASH) < CS_CREDITS_LEVEL_COUNT)
			{
				CS_Credits_End();
			}
			else
			{
				if (GAME_TRACKER->levelID == NAUGHTY_DOG_CRATE)
				{
					if ((u32)gGT->msInThisLEV >> CS_FRAME32_SHIFT < CS_ND_CRATE_SKIP_MIN_FRAME32)
					{
						goto afterCameraAndSkipChecks;
					}
					RaceFlag_SetCanDraw(1);
					if (!RaceFlag_IsTransitioning() && !RaceFlag_IsFullyOnScreen())
					{
						RaceFlag_SetFullyOffScreen();
					}
				}
				else
				{
					RaceFlag_SetCanDraw(1);
					if (!RaceFlag_IsTransitioning() && !RaceFlag_IsFullyOnScreen())
					{
						RaceFlag_SetFullyOffScreen();
					}
					levelToLoad = CREDITS_CRASH;
					if ((u32)(GAME_TRACKER->levelID - OXIDE_ENDING) < CS_OXIDE_ENDING_LEVEL_COUNT)
					{
						goto requestSkipLevelLoad;
					}
				}
				CseqMusic_StopAll();
				CDSYS_XAPauseRequest();
				RaceFlag_SetDrawOrder(0);
				levelToLoad = MAIN_MENU_LEVEL;
			requestSkipLevelLoad:
				MainRaceTrack_RequestLoad(levelToLoad);
				{
					struct GameTracker *currentTracker = GAME_TRACKER;
					CS_FINISHED = 1;
					currentTracker->gameMode2 &= ~VEH_FREEZE_PODIUM;
				}
				cs->decodedOpcode = metadataBackup;
				return 1;
			}
		}
	}

	goto afterCameraAndSkipChecks;
// NOTE(aalhendi): Keep this exit outside the interpreter loop; both script paths share their final call.
reselectCreditsScript:
{
	s32 scriptIndex;
	cutsceneOpcodes = CS_SCRIPT(creditsCutsceneOpcodes);
	scriptIndex = gGT->levelID - CREDITS_CRASH;
	CS_ScriptCmd_OpcodeAt(cs, cutsceneOpcodes[scriptIndex]);
}
	goto updateInstanceAndReturn;

afterCameraAndSkipChecks:
	opcodeChanged = 0;
	if (elapsedTimeRemaining == 0)
		goto updateInstanceAndReturn;

	do
	{
		// NOTE(aalhendi): Keep the selected LOD live across every opcode, including commands that leave it unchanged.
		CTR_PSX_OBSERVE_VALUE(lodIndexState);
		switch (opcodeMeta->opcode)
		{
		case CS_OPCODE_UI_FADE_TO_BLACK:
			GAME_TRACKER->pushBuffer_UI.fadeFromBlack_desiredResult = 0;
			GAME_TRACKER->pushBuffer_UI.fade_step = CS_FADE_FROM_BLACK_STEP;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;

		case CS_OPCODE_WAIT_UI_FADE:
			if (0 < GAME_TRACKER->pushBuffer_UI.fadeFromBlack_currentValue)
			{
				goto updateInstanceAndReturn;
			}
			goto nextOpcode;
		case CS_OPCODE_SET_CAMERA_DISTANCE:
		{
			struct PushBuffer *pushBuffer = &gGT->pushBuffer[0];
			distanceToScreen = opcodeMeta->arg1.i;
			pushBuffer->distanceToScreen_PREV = distanceToScreen;
			pushBuffer->distanceToScreen_CURR = distanceToScreen;
			cs->flags |= CS_FLAG_CAMERA_DISTANCE_OVERRIDE;
			CS_ScriptCmd_OpcodeNext(cs);

			goto finishOpcodeStep;
		}

		case CS_OPCODE_SET_BOSS_CUTSCENE_INDEX:
			CS_BOSS_INDEX = opcodeMeta->arg1.i;
			if ((CS_BOSS_INDEX == 0) && (GAME_TRACKER->currAdvProfile.numRelics >= ADV_OXIDE_FINAL_RELIC_COUNT))
			{
				CS_BOSS_INDEX = 9;
			}
			CS_PHASE = CS_WAIT_INPUT;
			goto nextOpcode;
		case CS_OPCODE_END_BOSS_CUTSCENE:
		{
			struct GameTracker *tracker;
			CS_FINISHED = 1;
			CS_DestroyPodium_StartDriving();
			CS_BOSS_INDEX = -1;
			tracker = GAME_TRACKER;
			tracker->overlayTransition = CS_BOSS_END_OVERLAY_TRANSITION_FRAMES;
			tracker->gameMode2 &= ~VEH_FREEZE_PODIUM;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;
		}

		case CS_OPCODE_SET_UNK4_1333:
			cs->unk4 = 0x1333;
			goto nextOpcode;

		case CS_OPCODE_CLEAR_INSTANCE_FLAGS:
			if (instance != 0)
			{
				instance->flags &= ~opcodeMeta->arg1.u;
			}
			goto nextOpcode;

		case CS_OPCODE_SET_INSTANCE_FLAGS:
			if (instance != 0)
			{
				instance->flags |= opcodeMeta->arg1.u;
			}
			goto nextOpcode;

		case CS_OPCODE_ADD_INSTANCE_DEPTH_BIAS:
			if (instance != 0)
			{
				instance->depthBiasNormal += (char)opcodeMeta->arg1.i;
				instance->depthBiasSecondary += (char)opcodeMeta->arg1.i;
			}
			goto nextOpcode;

		case CS_OPCODE_SHOW_INSTANCE:
			if (instance != 0)
			{
				instance->flags &= ~HIDE_MODEL;
			}
			goto nextOpcode;

		case CS_OPCODE_HIDE_INSTANCE:
			if (instance != 0)
			{
				instance->flags |= HIDE_MODEL;
			}
			goto nextOpcode;

		case CS_OPCODE_SET_PARTICLE_ID:
			cs->particleID = (s16)opcodeMeta->arg1.i;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;

		case CS_OPCODE_RACEFLAG_TRANSITION1_IF_OFFSCREEN:
		{
			b32 raceFlagState = RaceFlag_IsFullyOffScreen();
			if (raceFlagState == 1)
			{
				RaceFlag_SetCanDraw(1);
				RaceFlag_BeginTransition(1);
			}
			goto nextOpcode;
		}

		case CS_OPCODE_ADVANCE_IF_RACEFLAG_ONSCREEN:
			if (!RaceFlag_IsFullyOnScreen())
				goto updateInstanceAndReturn;
			goto nextOpcode;
		case CS_OPCODE_ADVANCE_IF_CREDITS_TEXT_VALID:
		{
			u32 valid = CS_Credits_IsTextValid();
			// Only the low halfword is the script condition.
			valid <<= 16;
			CTR_PSX_OBSERVE_VALUE(valid);
			if (valid == 0)
				goto updateInstanceAndReturn;
			goto nextOpcode;
		}
		case CS_OPCODE_ADVANCE_IF_CREDITS_GHOSTS_READY:
			if ((s16)CS_Credits_NewCreditGhosts() == 0)
				goto updateInstanceAndReturn;
			goto nextOpcode;
		case CS_OPCODE_ADVANCE_IF_LEVEL_TIME_REACHED:
			if ((u32)gGT->msInThisLEV >> CS_FRAME32_SHIFT < opcodeMeta->arg1.u)
			{
				goto updateInstanceAndReturn;
			}
			goto nextOpcode;

		case CS_OPCODE_SYNC_ANIM_FRAME:
			animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, animFrame32 >> CS_FRAME32_SHIFT);
			animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
			goto updateInstanceAndReturn;

		case CS_OPCODE_RACEFLAG_TRANSITION2_IF_ONSCREEN:
		{
			b32 raceFlagState = RaceFlag_IsFullyOnScreen();
			if (raceFlagState == 1)
			{
				RaceFlag_BeginTransition(2);
			}
			goto nextOpcode;
		}

		case CS_OPCODE_LOAD_LEVEL_STARS:
			numPlayers = gGT->numPlyrCurrGame;
			gGT->stars.numStars = (s16)((s32)gGT->level1->stars.numStars / (s32)(u32)numPlayers);
			gGT->stars.spread = gGT->level1->stars.spread;
			gGT->stars.seed = gGT->level1->stars.seed;
			gGT->stars.distance = gGT->level1->stars.distance;
			CS_LOAD_NEXT_SWAP = 0;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;
		case CS_OPCODE_RESELECT_LEVEL_SCRIPT:
			gGT->bool_AdvHub_NeedToSwapLEV = 1;
			if (GAME_TRACKER->gameMode2 & CREDITS)
				goto reselectCreditsScript;
			{
				s32 scriptIndex;
				cutsceneOpcodes = CS_SCRIPT(introCutsceneOpcodes);
				scriptIndex = gGT->levelID - INTRO_RACE_TODAY;
				CS_ScriptCmd_OpcodeAt(cs, cutsceneOpcodes[scriptIndex]);
			}
			goto updateInstanceAndReturn;

		case CS_OPCODE_REQUEST_LEVEL:
		{
			s32 requested = opcodeMeta->arg1.i;
			CTR_PSX_OBSERVE_VALUE(requested);
			nextFrameTime = requested;
			gGT->levelID = nextFrameTime;
			switch (nextFrameTime)
			{
			case GEM_STONE_VALLEY:
				GAME_TRACKER->gameMode2 &= ~VEH_FREEZE_PODIUM;
				MainRaceTrack_RequestLoad(GEM_STONE_VALLEY);
				break;
			case INTRO_RACE_TODAY:
				RaceFlag_SetCanDraw(0);
				// fall through
			case CREDITS_CRASH:
				GAME_TRACKER->gameMode2 &= ~VEH_FREEZE_PODIUM;
				MainRaceTrack_RequestLoad((s16)nextFrameTime);
				break;
			case MAIN_MENU_LEVEL:
				RaceFlag_SetDrawOrder(0);
				GAME_TRACKER->gameMode2 &= ~VEH_FREEZE_PODIUM;
				MainRaceTrack_RequestLoad(MAIN_MENU_LEVEL);
				break;
			default:
				CS_LOAD_NEXT_SWAP = 1;
				LOAD_Hub_ReadFile(CS_BIGFILE_HEADER, nextFrameTime, 3 - (s32)gGT->activeMempackIndex);
				break;
			}
			goto nextOpcode;
		}

		case CS_OPCODE_WAIT_SWAP_QUEUE:
			if ((CS_LOAD_NEXT_SWAP == 0) || (CS_QUEUE_READY == 0) || (CS_QUEUE_LENGTH != 0))
			{
				goto updateInstanceAndReturn;
			}
			goto nextOpcode;

		case CS_OPCODE_PLAY_XA:
			CDSYS_XAPlay(opcodeMeta->arg0.i, opcodeMeta->arg1.i);
			if (CS_XA_STATE != XA_IDLE)
			{
				cs->flags = (cs->flags | CS_FLAG_XA_SYNC_ANIMATION) & ~CS_FLAG_XA_PLAYBACK_STARTED;
			}
			goto nextOpcode;
		case CS_OPCODE_WAIT_XA_DONE:
			if (CS_XA_STATE == XA_IDLE)
			{
				cs->flags &= ~CS_FLAG_XA_SYNC_ANIMATION;
				CS_ScriptCmd_OpcodeNext(cs);

				goto finishOpcodeStep;
			}

		case CS_OPCODE_YIELD:
			goto updateInstanceAndReturn;

		case CS_OPCODE_CLEAR_CUTSCENE_FLAGS:
			cs->flags &= ~(s16)opcodeMeta->arg1.i;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;
		case CS_OPCODE_SET_CUTSCENE_FLAGS:
			cs->flags |= (s16)opcodeMeta->arg1.i;
			CS_ScriptCmd_OpcodeNext(cs);

			goto finishOpcodeStep;

		case CS_OPCODE_FADE_FROM_BLACK:
			gGT->pushBuffer[0].fadeFromBlack_currentValue = CS_FADE_FROM_BLACK_CURRENT;
			gGT->pushBuffer[0].fadeFromBlack_desiredResult = CS_FADE_FROM_BLACK_TARGET;
			gGT->pushBuffer[0].fade_step = CS_FADE_FROM_BLACK_STEP;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;

		case CS_OPCODE_SCALE_TO:
			cs->desiredScale = (s16)opcodeMeta->arg0.i;
			cs->scaleSpeed = (s16)opcodeMeta->arg1.i;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;
		case CS_OPCODE_SET_PATH_MOTION:
		{
			u16 pathFlags;
			nextFrameTime = opcodeMeta->arg1.i;
			if (nextFrameTime == -1)
				pathFlags = cs->flags | CS_FLAG_PATH_MOTION_DISABLED;
			else
			{
				cs->pathProgress32 = 0;
				pathFlags = cs->flags & ~CS_FLAG_PATH_MOTION_DISABLED;
			}
			CTR_PSX_OBSERVE_VALUE(pathFlags);
			cs->flags = pathFlags;
			goto nextOpcode;
		}

		case CS_OPCODE_SET_VISIBLE_LOD:
			if (instance != 0)
			{
				s32 numHeaders = instance->model->numHeaders;
				if (numHeaders != 0 && (modelHeader = instance->model->headers) != 0)
				{
					lodIndexState = opcodeMeta->arg1.i;
					if (lodIndexState >= numHeaders)
						lodIndexState = numHeaders - 1;
					if (lodIndexState != 0)
					{
						numHeaders = lodIndexState;
						do
						{
							modelHeader->maxDistanceLOD = 0;
							modelHeader++;
						} while (--numHeaders);
					}
					modelHeader->maxDistanceLOD = CS_MODEL_LOD_DISTANCE_VISIBLE;
				}
			}
			goto nextOpcode;

		case CS_OPCODE_SPAWN_CHILD:
			if (instance != 0)
			{
				// Retail builds this opcode 3 init data at scratchpad 0x1f800108.
				struct CsThreadInitData *initData = CTR_SCRATCHPAD_PTR(struct CsThreadInitData, 0x108);

				CS_Instance_GetFrameData(instance, (s32)opcodeMeta->animIndex, opcodeMeta->arg0.i, SVec3Slot_AsVec3(&initData->podiumPos),
				                         SVec3Slot_AsVec3(&initData->rot), 0);

				initData->podiumPos.x += (s16)instance->matrix.t[0];
				initData->podiumPos.y += (s16)instance->matrix.t[1];
				initData->podiumPos.z += (s16)instance->matrix.t[2];
				initData->characterPos.x = 0;
				initData->characterPos.y = 0;
				initData->characterPos.z = 0;

				if (opcodeMeta->arg1.i == NDI_BOX_PARTICLES_01)
				{
					initData->rot.x = 0;
					initData->rot.y = 0;
					initData->rot.z = 0;
				}

				CS_Thread_Init(opcodeMeta->arg1.i, csThreadNames.s_spawn, initData, 0, instance->thread);
			}
			goto nextOpcode;

		case CS_OPCODE_CREDITS_DANCER:
		{
			// Retail builds this credits dancer init data at scratchpad 0x1f800108.
			struct CsThreadInitData *initData = CTR_SCRATCHPAD_PTR(struct CsThreadInitData, 0x108);

			initData->podiumPos.x = 0;
			initData->podiumPos.y = 0;
			initData->podiumPos.z = 0;
			initData->rot.x = 0;
			initData->rot.y = 0;
			initData->rot.z = 0;
			initData->characterPos.x = 0;
			initData->characterPos.y = 0;
			initData->characterPos.z = 0;

			GAME_TRACKER->podium_modelIndex_First = (u8)opcodeMeta->arg1.i;
			GAME_TRACKER->podium_modelIndex_Second = 0;
			GAME_TRACKER->podium_modelIndex_Third = 0;

			if (opcodeMeta->arg1.i == STATIC_OXIDEDANCE)
			{
				GAME_TRACKER->podium_modelIndex_First = 0;
				GAME_TRACKER->podium_modelIndex_Second = STATIC_OXIDEDANCE;
			}
			if (opcodeMeta->arg1.i == STATIC_CRASHDANCE)
			{
				initData->rot.y += ANG_PI;
			}

			initData->rot.x += CS_SCRIPT(creditsDancerRotOffset).x;
			initData->rot.y += CS_SCRIPT(creditsDancerRotOffset).y;
			initData->rot.z += CS_SCRIPT(creditsDancerRotOffset).z;

			dancerThread = (struct Thread *)CS_Thread_Init(opcodeMeta->arg1.i, csThreadNames.s_g_dancer, initData, 0, 0);
			CS_Credits_NewDancer(dancerThread, (s32)(s16)opcodeMeta->arg1.i);
		}
			goto nextOpcode;

		case CS_OPCODE_HIDE_INSTANCE_AND_END_THREAD:
			if (instance != 0)
			{
				instance->flags |= HIDE_MODEL;
			}
			cs->decodedOpcode = metadataBackup;
			return 1;

		case CS_OPCODE_ANIM_ROT_RANGE:
		case CS_OPCODE_ANIM_SYNC_MARKER:
		case CS_OPCODE_ANIM_RANGE:
			if (instance != 0)
			{
				cutsceneFlags = cs->flags;
				if ((cutsceneFlags & CS_FLAG_XA_SYNC_ANIMATION) != 0)
				{
					if (((cutsceneFlags & CS_FLAG_XA_PLAYBACK_STARTED) == 0) && (CS_XA_STATE == XA_PLAYING))
					{
						cs->flags = cutsceneFlags | CS_FLAG_XA_PLAYBACK_STARTED;
					}
					if (CS_XA_STATE != XA_IDLE)
					{
						if ((cs->flags & CS_FLAG_XA_PLAYBACK_STARTED) == 0)
						{
							animFrame32 = 0;
						}
						else
						{
							animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState,
							                                             (CS_XA_OFFSET * CS_XA_ANIM_SYNC_NUMERATOR) / CS_XA_ANIM_SYNC_DENOMINATOR);
							animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
						}
						if (animFrame32 <= opcodeMeta->arg1.i << CS_FRAME32_SHIFT)
							goto updateInstanceAndReturn;
						goto nextOpcode;
					}
					goto nextOpcode;
				}
			}
			if (opcodeChanged != 0)
			{
				s32 rng;
				animIndex = (s32)opcodeMeta->animIndex;
				animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, opcodeMeta->arg0.i);
				animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
				rng = MixRNG_Scramble();
				opcodeChanged = 0;
				opcodeDuration =
				    ((s32)(((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) * (((s32)opcodeMeta->frameEnd - (s32)opcodeMeta->frameStart) + 1)) >>
				     FRACTIONAL_BITS) +
				    (s32)opcodeMeta->frameStart;
			}

			frameBoundaryHit = 0;
			if (opcodeMeta->arg0.i <= opcodeMeta->arg1.i)
			{
				s32 endFrame;
				endFrame = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, opcodeMeta->arg1.i) + 1;

				nextFrameTime = endFrame << CS_FRAME32_SHIFT;
				animFrame32 += elapsedTimeRemaining;
				if (animFrame32 >= nextFrameTime)
				{
					frameBoundaryHit = 1;
					elapsedTimeRemaining = 0;
					if (nextFrameTime != 0)
					{
						nextFrameTime = animFrame32 - nextFrameTime;
						goto saveRemainingTime;
					}
				}
			}
			else
			{
				nextFrameTime = opcodeMeta->arg1.i << CS_FRAME32_SHIFT;
				animFrame32 -= elapsedTimeRemaining;
				if (animFrame32 < nextFrameTime)
				{
					CTR_PSX_OBSERVE_VALUE(nextFrameTime);
					frameBoundaryHit = 1;

					nextFrameTime -= animFrame32;
				saveRemainingTime:
					elapsedTimeRemaining = nextFrameTime;
				}
			}

			if ((frameBoundaryHit) || (opcodeDuration < 1))
			{
				opcodeDuration = opcodeDuration + -1;
				if (opcodeDuration < 1)
				{
					CS_ScriptCmd_OpcodeNext(cs);
					opcodeChanged = 1;
				}
				else
				{
					animFrame32 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, opcodeMeta->arg0.i);
					animFrame32 = animFrame32 << CS_FRAME32_SHIFT;
				}
			}
			else
			{
				elapsedTimeRemaining = 0;
			}
			goto finishOpcodeStep;
		case CS_OPCODE_BRANCH_ADV_CHAR_SELECT:
		{
			struct CutsceneObj *branchCs;
			char *branchTarget;
			if (opcodeMeta->frameEnd == 0)
			{
				if (opcodeMeta->arg0.i != (s32)CS_GARAGE_CHARACTERS[CS_GARAGE_CURRENT] || CS_GARAGE_SELECTED == 0)
				{
					branchCs = cs;
					branchTarget = opcodeMeta->arg1.ptr;
					opcodeChanged = 1;
					goto invokeGarageBranch;
				}
			}
			else
			{
				if (opcodeMeta->arg0.i == (s32)CS_GARAGE_CHARACTERS[CS_GARAGE_CURRENT] && CS_GARAGE_SELECTED == 1)
				{
					branchCs = cs;
					branchTarget = opcodeMeta->arg1.ptr;
					opcodeChanged = 1;
				// Both garage conditions branch to the requested script, then advance its first opcode.
				invokeGarageBranch:
					CS_ScriptCmd_OpcodeAt(branchCs, branchTarget);
				}
			}
			goto nextOpcode;
		}

		case CS_OPCODE_GOTO:
			CS_ScriptCmd_OpcodeAt(cs, opcodeMeta->arg1.ptr);
			opcodeChanged = 1;
			goto finishOpcodeStep;
		case CS_OPCODE_BRANCH_IF_RANDOM_LE:
		{
			s32 rng = MixRNG_Scramble();
			if (opcodeMeta->arg0.i >= (s32)((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_BYTE_MASK))
				CS_ScriptCmd_OpcodeAt(cs, opcodeMeta->arg1.ptr);
			else
				CS_ScriptCmd_OpcodeNext(cs);
			opcodeChanged = 1;
			goto finishOpcodeStep;
		}

		case CS_OPCODE_PLAY_CONTEXT_FX:
			if (GAME_TRACKER->levelID == ADVENTURE_GARAGE)
			{
				if (instance != 0)
				{
					Garage_PlayFX(opcodeMeta->arg1.u, (s32)instance->model->id - STATIC_CRASHSELECT);
				}
			}
			else
			{
				if (CS_Instance_BoolPlaySound(cs, instance))
				{
					OtherFX_Play((u32)(u16)(s16)opcodeMeta->arg1.i, 1);
				}
			}
			goto nextOpcode;

		case CS_OPCODE_STOP_FX:
			OtherFX_Stop2((u32)(u16)(s16)opcodeMeta->arg1.i);
			goto nextOpcode;

		case CS_OPCODE_START_MUSIC:
			CseqMusic_Start((u32)(u16)(s16)opcodeMeta->arg1.i, 0, 0, 0, opcodeMeta->arg0.i);
			goto nextOpcode;

		case CS_OPCODE_RESTART_MUSIC:
			CseqMusic_Restart((u32)(u16)(s16)opcodeMeta->arg1.i, 1);
			goto nextOpcode;

		case CS_OPCODE_END_CREDITS:
			CS_Credits_End();
			cs->decodedOpcode = metadataBackup;
			return 1;

		case CS_OPCODE_SET_GAME_MODE_FLAGS:
			switch (opcodeMeta->animIndex)
			{
			case 0:
				GAME_TRACKER->gameMode1 |= opcodeMeta->arg1.u;
				break;
			case 1:
				GAME_TRACKER->gameMode2 |= opcodeMeta->arg1.u;
				break;
			case 2:
				GAME_TRACKER->renderFlags |= opcodeMeta->arg1.u;
				break;
			case 3:
				GAME_TRACKER->renderFlags &= ~opcodeMeta->arg1.u;
				break;
			}
			goto nextOpcode;

		case CS_OPCODE_SET_SUBTITLE:
			cs->Subtitles.textPos.x = opcodeMeta->animIndex;
			cs->Subtitles.textPos.y = opcodeMeta->frameStart;
			cs->Subtitles.lngIndex = opcodeMeta->frameEnd;
			cs->Subtitles.font = opcodeMeta->rotStart;
			cs->Subtitles.colors = opcodeMeta->rotEnd;
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;

		case CS_OPCODE_SET_AUDIO_VOLUME:
		{
			const u8 *opcodeBytes = (const u8 *)opcodeMeta;

			CS_CONTROLS_AUDIO = 1;
			howl_VolumeSet(HOWL_VOLUME_TYPE_FX, opcodeBytes[CS_AUDIO_VOLUME_FX_BYTE_OFFSET]);
			howl_VolumeSet(HOWL_VOLUME_TYPE_MUSIC, opcodeBytes[CS_AUDIO_VOLUME_MUSIC_BYTE_OFFSET]);
			howl_VolumeSet(HOWL_VOLUME_TYPE_VOICE, opcodeBytes[CS_AUDIO_VOLUME_VOICE_BYTE_OFFSET]);
			goto nextOpcode;
		}

		nextOpcode:
			CS_ScriptCmd_OpcodeNext(cs);
			goto finishOpcodeStep;
		default:
			cs->decodedOpcode = metadataBackup;
			return 0;
		}

	finishOpcodeStep:;
	} while ((elapsedTimeRemaining != 0) || (opcodeChanged != 0));

	{
		s32 rotY;
		s32 checkedAnimFrame;
	updateInstanceAndReturn:
		cs->animIndex = (char)animIndex;
		cs->animFrame32 = animFrame32;
		cs->opcodeDuration = (s16)opcodeDuration;
		cs->lodIndex = (s16)lodIndexState;
		rotY = (s32)opcodeMeta->rotStart;
		animFrame32 = animFrame32 >> CS_FRAME32_SHIFT;
		if (rotY != (s32)opcodeMeta->rotEnd)
		{
			s32 last = opcodeMeta->arg1.i;
			s32 first = opcodeMeta->arg0.i;
			if (last != first)
			{
				s32 numerator = ((s32)((((opcodeMeta->rotEnd - rotY) + (u32)ANG_PI) & (ANG_TWO_PI - 1)) - ANG_PI)) * (animFrame32 - first);
				s32 range;

				range = abs(last - first);
				rotY += numerator / range;
			}
		}
		rotY = rotY + cs->baseRotY;
		if ((rotY != (s32)cs->rot.y) && (cs->rot.y = (s16)rotY, instance != 0))
		{
			ConvertRotToMatrix(&instance->matrix, &cs->rot);
		}
		checkedAnimFrame = CS_Instance_SafeCheckAnimFrame(instance, animIndex, lodIndexState, animFrame32);
		if (animFrame32 != checkedAnimFrame)
		{
			animIndex = 0;
			animFrame32 = 0;
		}
		if (instance != 0)
		{
			instance->animIndex = (char)animIndex;
			instance->animFrame = (s16)animFrame32;
		}
		if (cs->frameOverrideRoot != 0)
		{
			const CsInitMatrixHalf *frameHalves;

			frameData = &cs->frameOverrideRoot->data[animFrame32];
			// NOTE(aalhendi): Preserve the frame index while computing its matrix address.
			CTR_PSX_OBSERVE_VALUE(animFrame32);
			frameHalves = CsInitMatrixEntry_ConstHalves(frameData);
			CTR_WriteU32AlignedLE((u8 *)&instance->matrix + 0x00, CTR_ReadU32LE(&frameHalves[0]));
			CTR_WriteU32AlignedLE((u8 *)&instance->matrix + 0x04, CTR_ReadU32LE(&frameHalves[2]));
			CTR_WriteU32AlignedLE((u8 *)&instance->matrix + 0x08, CTR_ReadU32LE(&frameHalves[4]));
			CTR_WriteU32AlignedLE((u8 *)&instance->matrix + 0x0c, CTR_ReadU32LE(&frameHalves[6]));
			CTR_WriteU32AlignedLE((u8 *)&instance->matrix + 0x10, CTR_ReadU32LE(&frameHalves[8]));
			instance->matrix.t[0] = frameData->offset[0];
			instance->matrix.t[1] = frameData->offset[1];
			instance->matrix.t[2] = frameData->offset[2];
		}
		return 0;
	}
}

void CS_Thread_AnimateScale(struct Thread *t)
{
	int speed;
	int newScale;
	int desiredScale;
	struct Instance *inst = t->inst;
	struct CutsceneObj *cs = t->object;
	if (inst == NULL)
		return;
	speed = cs->scaleSpeed;
	if (speed == 0)
		return;
	newScale = inst->scale.x;
	desiredScale = cs->desiredScale;
	newScale += speed;
	if (speed > 0)
	{
		if (newScale >= desiredScale)
		{
			newScale = desiredScale;
			cs->scaleSpeed = 0;
		}
	}
	else
	{
		if (newScale <= desiredScale)
		{
			newScale = desiredScale;
			cs->scaleSpeed = 0;
		}
	}
	inst->scale.x = newScale;
	inst->scale.y = newScale;
	inst->scale.z = newScale;
}

void CS_Thread_MoveOnPath(struct Thread *t)
{
	struct CutsceneObj *cs = t->object;
	struct Instance *inst;
	struct Level *level;
	struct GameTracker *gGT;
	s16 modelID;
	s32 pathModelKind;
	s32 pathIndex;
	// NOTE(aalhendi): Fixed scratch-register lifetimes preserve retail scheduling; native uses ordinary locals.
	struct SpawnType2 *spawnEntry, *staticPath;
	register s32 frameWord CTR_PSX_REGISTER("$3");
	register SVec3 *pathPoints CTR_PSX_REGISTER("$6");

	SVec3 *nextPoint;
	struct SpawnPosRot *posRot;
	register u16 pathFrame32;
	s32 segmentIndex;
	s32 segmentFrac32;
	SVec3 rot;

	if ((cs->flags & CS_FLAG_PATH_MOTION_DISABLED) != 0)
	{
		return;
	}

	inst = t->inst;
	if (inst == 0)
	{
		return;
	}

	modelID = inst->model->id;
	pathModelKind = (s16)(modelID - STATIC_PPOINTTHINGINTRO);

	if ((u32)pathModelKind >= CS_PATH_MODEL_KIND_COUNT)
	{
		return;
	}

	switch (pathModelKind)
	{
	case CS_PATH_MODEL_COCO_SELECT:
	{
		struct Level *cocoLevel = GAME_TRACKER->level1;
		struct SpawnType2 *entry;
		SVec3 *points, *next;
		s32 prog;
		register s32 cocoFraction CTR_PSX_REGISTER("$7");
		if (cocoLevel->numSpawnType2 <= 0)
			return;
		entry = cocoLevel->ptrSpawnType2;
		points = entry->coords.positions;
		staticPath = entry;
		if (!points)
			return;
		prog = 0;
		if (cs->animIndex == 3)
			prog = cs->animFrame32;
		cocoFraction = prog & CS_FRAME32_MASK;
		frameWord = staticPath->numCoords;
		prog >>= CS_FRAME32_SHIFT;
		if (prog >= frameWord - 1)
		{
			next = &points[frameWord - 1];
			points = next;
		}
		else if (prog < 0)
		{
			next = points;
		}
		else
		{
			points = &points[prog];
			next = points + 1;
		}
		inst->matrix.t[0] = points->x + ((cocoFraction * (next->x - points->x)) >> CS_FRAME32_SHIFT);
		inst->matrix.t[1] = points->y + ((cocoFraction * (next->y - points->y)) >> CS_FRAME32_SHIFT);
		inst->matrix.t[2] = points->z + ((cocoFraction * (next->z - points->z)) >> CS_FRAME32_SHIFT);
		return;
	}
	case CS_PATH_MODEL_PPOINT_THING_INTRO:
	case CS_PATH_MODEL_OXIDE_SPEAKER:
	{
		register s32 numCoords CTR_PSX_REGISTER("$7");

		pathIndex = (u8)inst->name[strlen(inst->name) - 1] - '0';
		gGT = GAME_TRACKER;
		level = gGT->level1;

		if (level->numSpawnType2 <= pathIndex)
		{
			return;
		}

		spawnEntry = &level->ptrSpawnType2[pathIndex];
		pathPoints = spawnEntry->coords.positions;

		if (pathPoints == 0)
		{
			return;
		}

		// The clock wraps as a halfword; its signed high bits select the path segment.
		pathFrame32 = cs->pathProgress32;
		frameWord = (s32)((u32)pathFrame32 << 16);
		segmentIndex = frameWord >> 21;
		cs->pathProgress32 = (u16)(pathFrame32 + (u16)gGT->elapsedTimeMS);
		segmentFrac32 = pathFrame32 & CS_FRAME32_MASK;

		if (segmentIndex >= (numCoords = spawnEntry->numCoords) - 1)
		{
			segmentIndex = 0;

			if (inst->model->id == STATIC_OXIDESPEAKER)
			{
				segmentIndex = numCoords - 2;
				cs->pathProgress32 = segmentIndex << CS_FRAME32_SHIFT;
			}
			else
			{
				cs->pathProgress32 = 0;
			}
		}

		// NOTE(aalhendi): Keep address calculation separate from advancing the current point.
		{
			register SVec3 *indexed CTR_PSX_REGISTER("$2");
			indexed = &pathPoints[segmentIndex];
			CTR_PSX_KEEP_VALUE_RELAXED(indexed);
			pathPoints = indexed;
		}
		nextPoint = &pathPoints[1];

		inst->matrix.t[0] = pathPoints->x + ((segmentFrac32 * (nextPoint->x - pathPoints->x)) >> CS_FRAME32_SHIFT);
		inst->matrix.t[1] = pathPoints->y + ((segmentFrac32 * (nextPoint->y - pathPoints->y)) >> CS_FRAME32_SHIFT);
		inst->matrix.t[2] = pathPoints->z + ((segmentFrac32 * (nextPoint->z - pathPoints->z)) >> CS_FRAME32_SHIFT);

		if (segmentIndex >= spawnEntry->numCoords - 1)
		{
			return;
		}

		if (inst->model->id == STATIC_OXIDESPEAKER)
		{
			return;
		}

		rot.x = cs->rot.x;
		rot.y = cs->rot.y + ratan2(pathPoints[1].x - pathPoints[0].x, pathPoints[1].z - pathPoints[0].z);
		rot.z = cs->rot.z;

		ConvertRotToMatrix(&inst->matrix, &rot);
		return;
	}
	case CS_PATH_MODEL_PR_THING_INTRO:
	case CS_PATH_MODEL_OXIDE_LIL_SHIP:
	case CS_PATH_MODEL_END_OXIDE_BIG_SHIP:
	case CS_PATH_MODEL_END_OXIDE_LIL_SHIP:

		pathIndex = (u8)inst->name[strlen(inst->name) - 1] - '0';
		gGT = GAME_TRACKER;
		level = gGT->level1;

		if (level->numSpawnType2_PosRot <= pathIndex)
		{
			return;
		}

		staticPath = &level->ptrSpawnType2_PosRot[pathIndex];
		posRot = staticPath->coords.posRot;

		if (posRot == 0)
		{
			return;
		}

		frameWord = cs->pathProgress32;
		frameWord = (s32)((u32)frameWord << 16);
		pathIndex = frameWord >> 16;
		cs->pathProgress32 = (u16)gGT->elapsedTimeMS + pathIndex;
		pathIndex = frameWord >> 21;

		if (pathIndex >= staticPath->numCoords - 1)
		{
			pathIndex = 0;
			cs->pathProgress32 = 0;
		}

		{
			posRot = &posRot[pathIndex];

			inst->matrix.t[0] = posRot->pos.x;
			inst->matrix.t[1] = posRot->pos.y;
			inst->matrix.t[2] = posRot->pos.z;

			rot.x = posRot->rot.x;
			rot.y = posRot->rot.y;
			rot.z = posRot->rot.z;
		}

		break;

	default:
		return;
	}

	ConvertRotToMatrix(&inst->matrix, &rot);
}

void CS_Thread_Particles(struct Thread *t)
{
	struct CutsceneObj *cs;
	struct Instance *inst = t->inst;
	const struct CsParticleConfig *entry;
	s8 particleID;

	if (inst == NULL)
	{
		return;
	}

	if ((inst->flags & HIDE_MODEL) != 0)
	{
		return;
	}

	cs = t->object;
	particleID = cs->particleID;
	if ((u8)particleID >= 9)
	{
		return;
	}

#ifdef CTR_NATIVE
	// NOTE(aalhendi): Retail also admits ID 8, but that points into opcode metadata,
	// not a particle record. Reject it before dereferencing the table on native.
	if ((u8)particleID >= sizeof(D233.particleConfigs) / sizeof(D233.particleConfigs[0]))
	{
		return;
	}
#endif

	entry = &D233.particleConfigs[(int)particleID];

	while (1)
	{
		{
			int i;
			for (i = 0; i < entry->meta.count; i++)
			{
				struct Particle *p = Particle_Init(0, GAME_TRACKER->iconGroup[entry->meta.iconGroupIndex], entry->emitter);

				if (p != NULL)
				{
					SVec3 pos;

					CS_Instance_GetFrameData(inst, inst->animIndex, (s16)inst->animFrame, &pos, NULL, entry->meta.frameOffset);

					p->axis[0].startVal = CTR_MipsAddLo(p->axis[0].startVal, CTR_MipsSll(CTR_MipsAddLo(pos.x, inst->matrix.t[0]), 8));
					p->axis[1].startVal = CTR_MipsAddLo(p->axis[1].startVal, CTR_MipsSll(CTR_MipsAddLo(pos.y, inst->matrix.t[1]), 8));
					p->axis[2].startVal = CTR_MipsAddLo(p->axis[2].startVal, CTR_MipsSll(CTR_MipsAddLo(pos.z, inst->matrix.t[2]), 8));
					p->otIndexOffset = inst->depthBiasNormal + entry->spawn.modelDelta;
				}
			}
		}

		if ((entry->meta.flags & 1) == 0)
		{
			break;
		}

		entry++;
	}
}

struct CSInterpolateLinePacket
{
	u32 tag;
	u32 drawMode;
	u32 pad;
	u32 colorAndCode;
	u32 xy0;
	u32 xy1;
};

CTR_STATIC_ASSERT(sizeof(struct CSInterpolateLinePacket) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, tag) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, drawMode) == 0x04);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, pad) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, colorAndCode) == 0x0C);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, xy0) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct CSInterpolateLinePacket, xy1) == 0x14);

void CS_Thread_InterpolateFramesMS(struct Thread *t)
{
	struct GameTracker *gGT;
	struct PushBuffer *pb;
	struct Instance *inst = t->inst;
	struct DB *db;
	struct CSInterpolateLinePacket *packet;
	void *end;
	SVec3 curr;
	SVec3 next;
	s32 depth;

	CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &curr, NULL, 0);
	CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &next, NULL, 1);

	curr.x = (s16)((u16)curr.x + (u16)inst->matrix.t[0]);
	curr.y = (s16)((u16)curr.y + (u16)inst->matrix.t[1]);
	curr.z = (s16)((u16)curr.z + (u16)inst->matrix.t[2]);

	next.x = (s16)((u16)next.x + (u16)inst->matrix.t[0]);
	next.y = (s16)((u16)next.y + (u16)inst->matrix.t[1]);
	next.z = (s16)((u16)next.z + (u16)inst->matrix.t[2]);

	// Frame callbacks may replace the active tracker or drawing buffer.
	gGT = GAME_TRACKER;
	db = gGT->backBuffer;
	packet = db->primMem.cursor;
	end = db->primMem.guardEnd;

	if ((u32)(packet + 1) >= (u32)end)
	{
		return;
	}

	pb = &gGT->pushBuffer[0];
	CTR_GteSetRotMatrix(&pb->matrix_ViewProj);
	CTR_GteSetTransMatrix(&pb->matrix_ViewProj);
	CTR_GteLoadPositionsV0V1(&curr, &next);
	gte_rtpt();

	CTR_GteStoreLineXY(&packet->xy0);

	depth = CTR_GteReadDepthZ1();
	if (((u32)depth - CS_INTERPOLATE_LINE_DEPTH_MIN) < CS_INTERPOLATE_LINE_DEPTH_RANGE)
	{
		s32 color;
		u32 *ot;

		packet->drawMode = CS_INTERPOLATE_LINE_DRAW_MODE;
		color = CS_INTERPOLATE_LINE_MAX_COLOR;
		packet->pad = 0;

		if (depth > CS_INTERPOLATE_LINE_FADE_START_DEPTH)
		{
			s32 fade = (CS_INTERPOLATE_LINE_FADE_END_DEPTH - depth) * color;

			color = fade / (1 << CS_INTERPOLATE_LINE_FADE_SHIFT);
		}

		packet->colorAndCode = color | (color << 8) | (color << 16) | CS_INTERPOLATE_LINE_COLOR_CODE;

		depth >>= CS_INTERPOLATE_LINE_OT_SHIFT;
		if (depth > CS_INTERPOLATE_LINE_MAX_OT_INDEX)
		{
			depth = CS_INTERPOLATE_LINE_MAX_OT_INDEX;
		}

		// OT entries already contain a 24-bit link, including on native.
		ot = (u32 *)&pb->ptrOT[depth];
		packet->tag = *ot | CS_INTERPOLATE_LINE_OT_TAG;
		*ot = CtrGpu_PrimToOTLink24(packet);
		packet++;
	}

	GAME_TRACKER->backBuffer->primMem.cursor = packet;
}

void CS_Thread_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct CutsceneObj *cs;
	s16 modelID;

	CS_FINISHED = 0;

	if (inst->thread != 0)
	{
		goto check_polar;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CutsceneObj), NONE, MEDIUM, STATIC), CS_Thread_ThTick, csIntroNames.s_introguy, 0);

	inst->thread = t;

	if (t == 0)
	{
		return;
	}

	cs = t->object;

	t->inst = inst;

	cs->metadataMeta = &cs->decodedOpcode;
	cs->prevOpcode = (char *)-1;
	cs->Subtitles.lngIndex = -1;

	modelID = inst->model->id;

	if (modelID >= NDI_BOX_BOX_01)
		CS_ScriptCmd_OpcodeAt(cs, CS_BOX_MODEL_SCRIPTS[modelID - NDI_BOX_BOX_01]);
	else if ((u16)(modelID - STATIC_CRASHINTRO) < CS_INTRO_MODEL_SCRIPT_COUNT)
		CS_ScriptCmd_OpcodeAt(cs, CS_INTRO_MODEL_SCRIPTS[modelID - STATIC_CRASHINTRO]);
	else
		CS_ScriptCmd_OpcodeAt(cs, (char *)D233.script_default);

	cs->animFrame32 = cs->metadataMeta->arg0.i;

	{
		struct GameTracker *gGT;
		struct IconGroup *icons;
		s32 rng = MixRNG_Scramble();
		struct CsOpcodeMeta *meta = cs->metadataMeta;
		s16 frameStart = meta->frameStart;
		s32 frameEnd = meta->frameEnd;
		s32 duration;

		duration = ((rng >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) * ((frameEnd - meta->frameStart) + 1);
		cs->baseRotY = 0;
		cs->rot.x = 0;
		cs->rot.y = 0;
		cs->rot.z = 0;
		cs->pathProgress32 = 0;
		cs->lodIndex = 0;
		cs->flags = 0;
		cs->scaleSpeed = 0;
		cs->frameOverrideRoot = 0;
		cs->desiredScale = CS_DEFAULT_SCALE;
		cs->particleID = CS_DEFAULT_PARTICLE_ID;

		cs->opcodeDuration = frameStart + (s16)(duration >> FRACTIONAL_BITS);

		gGT = GAME_TRACKER;

		// Read the icon group before initializing the display fields.
		icons = gGT->iconGroup[0];
		cs->unk4 = 0;
		cs->unk6 = 0;
		cs->unk8 = CS_DEFAULT_COLOR_WORD;
		cs->unk_C = 0;
		cs->unk_E = 0;

		cs->ptrIcons = icons + 1;
	}

check_polar:
	if (GAME_TRACKER->levelID == INTRO_POLAR)
	{
		inst->vertSplit = 0;
		inst->flags |= REFLECTIVE;
	}
}

void CS_Thread_ThTick(struct Thread *t)
{
	// Bone probing and subtitle drawing reuse the same local workspace.

	union
	{
		SVec3 bonePos;
		RECT textRect;
	} local;
	struct CutsceneObj *cs = t->object;
	struct Instance *inst;
	struct Instance *effect;
	struct Instance *parentInst;
	struct Thread *parentThread;

	if (CS_Thread_UseOpcode(t->inst, cs))
	{
		t->flags |= THREAD_FLAG_DEAD;

		if ((GAME_TRACKER->gameMode2 & CREDITS) != 0)
		{
			return;
		}
	}

	CS_Thread_MoveOnPath(t);
	CS_Thread_AnimateScale(t);
	CS_Thread_Particles(t);

	if ((cs->flags & CS_FLAG_INTERPOLATE_FRAMES_MS) != 0)
	{
		CS_Thread_InterpolateFramesMS(t);
	}

	if (t->inst != 0)
	{
		parentThread = t->parentThread;

		if (parentThread != 0)
		{
			if ((cs->flags & CS_FLAG_SKIP_PARENT_FRAME_TRANSFORM) == 0)
			{
				struct CSThreadParentFrameScratch *parentFrame = CTR_SCRATCHPAD_PTR(struct CSThreadParentFrameScratch, 0x108);

				parentInst = parentThread->inst;

				CS_Instance_GetFrameData(parentInst, parentInst->animIndex, parentInst->animFrame, SVec3Slot_AsVec3(&parentFrame->parentPos),
				                         SVec3Slot_AsVec3(&parentFrame->parentRot), 0);

				inst = t->inst;
				inst->matrix.t[0] = CTR_MipsAddLo(parentInst->matrix.t[0], parentFrame->parentPos.x);
				{
					s32 pos = parentFrame->parentPos.y;
					s32 base = parentInst->matrix.t[1];
					inst->matrix.t[1] = CTR_MipsAddLo(base, pos);
				}
				{
					s32 pos = parentFrame->parentPos.z;
					s32 base = parentInst->matrix.t[2];
					inst->matrix.t[2] = CTR_MipsAddLo(base, pos);
				}

				if ((cs->flags & CS_FLAG_SKIP_PARENT_ROTATION) == 0)
				{
					ConvertRotToMatrix(&inst->matrix, SVec3Slot_AsVec3(&parentFrame->parentRot));
				}
			}
		}

		// Each callback may replace or remove the thread instance.
		effect = t->inst;
		if (effect == 0)
		{
			goto thTick_subtitles;
		}

		if ((cs->flags & CS_FLAG_WRITE_VERT_SPLIT_LINE) != 0)
		{
			inst = effect;
			CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, &local.bonePos, 0, 0);

			// Publish the split line before reading the possibly replaced instance.
			D233.VertSplitLine = local.bonePos.y;

			effect = t->inst;
			if (effect == 0)
			{
				goto thTick_subtitles;
			}
		}

		if ((cs->flags & CS_FLAG_RANDOM_ALPHA_SCALE) != 0)
		{
			inst = effect;
			inst->alphaScale = 0;

			if ((GAME_TRACKER->timer & 0x1) != 0)
			{
				inst->alphaScale = (MixRNG_Scramble() & CS_RANDOM_ALPHA_MASK) + CS_RANDOM_ALPHA_BASE;
			}
		}
	}

thTick_subtitles:
	if (cs->Subtitles.lngIndex > 0)
	{
		s32 textHeight;

		textHeight = DecalFont_DrawMultiLine(GAME_LANGUAGE_STRINGS[cs->Subtitles.lngIndex], cs->Subtitles.textPos.x, cs->Subtitles.textPos.y,
		                                     CS_SUBTITLE_TEXT_MAX_WIDTH, cs->Subtitles.font, cs->Subtitles.colors);

		// NOTE(aalhendi): Keep retail's low-half text-height conversion.
		local.textRect.x = (s16)((u16)cs->Subtitles.textPos.x - CS_SUBTITLE_BOX_HALF_WIDTH);
		local.textRect.w = CS_SUBTITLE_BOX_WIDTH;
		local.textRect.y = (s16)((u16)cs->Subtitles.textPos.y - CS_SUBTITLE_BOX_Y_OFFSET);
		local.textRect.h = ((s32)((u32)textHeight << 16) >> 16) + CS_SUBTITLE_BOX_HEIGHT_PADDING;

		RECTMENU_DrawInnerRect(&local.textRect, CS_SUBTITLE_BOX_DRAW_STYLE, GAME_TRACKER->backBuffer->otMem.uiOT);
	}

	if (CS_FINISHED != 0)
	{
		t->flags |= THREAD_FLAG_DEAD;
	}
}

struct Thread *CS_Thread_Init(s32 modelID, const char *name, struct CsThreadInitData *initData, s16 yawOffset, struct Thread *parent)
{
	struct CutsceneObj *cs;
	struct Instance *inst;
	struct Thread *t;
	char *scriptPtr;
	u32 bucket;
	struct CsOpcodeMeta *meta;
	s32 value;
	s32 random;

	if (modelID == NOFUNC)
	{
		// Camera scripts have an object and thread, but no rendered instance.
		inst = NULL;

		t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CutsceneObj), NONE, MEDIUM, CAMERA), CS_Thread_ThTick, name, parent);

		if (t == NULL)
		{
			return NULL;
		}
	}
	else
	{
		bucket = OTHER;

		if (((u32)modelID - NDI_KART6) < CS_MASK_KART_MODEL_COUNT)
		{
			bucket = AKUAKU;
		}

		if (((u32)modelID - NDI_KART0) < CS_KART_FRAME_OVERRIDE_COUNT)
		{
			bucket = GHOST;
		}

		inst = INSTANCE_BirthWithThread(modelID, name, MEDIUM, bucket, CS_Thread_ThTick, sizeof(struct CutsceneObj), parent);

		if (inst == NULL)
		{
			return NULL;
		}

		t = inst->thread;
		t->funcThDestroy = PROC_DestroyInstance;
	}

	cs = t->object;

	cs->metadataMeta = &cs->decodedOpcode;
	cs->frameOverrideRoot = NULL;
	cs->prevOpcode = (char *)-1;
	cs->Subtitles.lngIndex = -1;

	if (modelID == NOFUNC)
	{
		s32 level = GAME_TRACKER->levelID;

		if (level == NAUGHTY_DOG_CRATE)
		{
			CS_ScriptCmd_OpcodeAt(cs, &CS_SCRIPT(creditsOpcodeData)[CS_CREDITS_ND_CRATE_SCRIPT_OFFSET]);
		}
		else if (level == OXIDE_ENDING)
		{
			CS_ScriptCmd_OpcodeAt(cs, &CS_SCRIPT(introEndingOpcodeData)[0]);
		}
		else if (level == OXIDE_TRUE_ENDING)
		{
			CS_ScriptCmd_OpcodeAt(cs, &CS_SCRIPT(introEndingOpcodeData)[CS_TRUE_ENDING_SCRIPT_OFFSET]);
		}
		else if ((GAME_TRACKER->gameMode2 & CREDITS) != 0)
		{
			CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(creditsCutsceneOpcodes)[level - CREDITS_CRASH]);
		}
		else
		{
			CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(introCutsceneOpcodes)[level - INTRO_RACE_TODAY]);
		}
	}
	else
	{
		if (modelID >= NDI_BOX_BOX_01)
		{
			if ((u32)(value = CTR_MipsSubLo(modelID, NDI_BOX_BOX_01)) >= CS_BOX_MODEL_SCRIPT_COUNT)
			{
				CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(script_default));
			}
			else
			{
				CS_ScriptCmd_OpcodeAt(cs, CS_BOX_MODEL_SCRIPTS[value]);
			}


			// Intro kart scripts can override frames with the compact matrix table.
			if (((u32)modelID - NDI_KART0) < CS_KART_FRAME_OVERRIDE_COUNT)
			{
				cs->frameOverrideRoot = &CS_MATRIX_TABLE[modelID - NDI_KART0];
			}

			goto after_opcode;
		}

		if (((u32)modelID - STATIC_PINHEAD) < CS_PINHEAD_DEFAULT_SCRIPT_COUNT)
		{
			CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(script_default));
		}
		else if (modelID == STATIC_DINGOFIRE)
		{
			CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(script_dingofire));
		}
		else if (((u32)modelID - STATIC_TAWNA1) < CS_TAWNA_MODEL_SCRIPT_COUNT)
		{
			if (GAME_TRACKER->gameMode2 & CREDITS)
			{
				scriptPtr = CS_SCRIPT(script_tawnaCredits);
			}
			else
			{
				scriptPtr = CS_SCRIPT(script_tawnaNormal);
			}
			CS_ScriptCmd_OpcodeAt(cs, scriptPtr);
		}
		else if ((u32)(value = CTR_MipsSubLo(modelID, STATIC_CRASHDANCE)) >= CS_DANCE_MODEL_SCRIPT_COUNT)
		{
			CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(script_default));
		}
		else
		{
			if (modelID == GAME_TRACKER->podium_modelIndex_First)
				CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(danceFirstScripts)[value]);
			else
				CS_ScriptCmd_OpcodeAt(cs, CS_SCRIPT(danceOtherScripts)[value]);
		}
	}


after_opcode:

	cs->animFrame32 = cs->metadataMeta->arg0.i;

	// Select an inclusive random duration from the decoded opcode's short bounds.
	random = MixRNG_Scramble();
	meta = cs->metadataMeta;
	cs->opcodeDuration =
	    meta->frameStart +
	    (s16)((((random >> CS_RANDOM_DURATION_SHIFT) & CS_RANDOM_DURATION_MASK) * ((meta->frameEnd - meta->frameStart) + 1)) >> FRACTIONAL_BITS);

	if (inst != NULL)
	{
		s32 posY, posZ;

		// FullScene_Init supplies the light matrix used to place podium characters.
		CTR_GteLoadSVec3V0(SVec3Slot_AsVec3(&initData->characterPos));
		CTR_GteLoadDelay();
		gte_llv0();
		value = MFC2_S(25);
		posY = MFC2_S(26);
		posZ = MFC2_S(27);
		inst->matrix.t[0] = CTR_MipsAddLo(value, initData->podiumPos.x);
		inst->matrix.t[1] = CTR_MipsAddLo(posY, initData->podiumPos.y);
		inst->matrix.t[2] = CTR_MipsAddLo(posZ, initData->podiumPos.z);
		if (GAME_TRACKER->levelID != NAUGHTY_DOG_CRATE)
		{
			inst->scale.x = CS_PODIUM_CHARACTER_SCALE;
			inst->scale.y = CS_PODIUM_CHARACTER_SCALE;
			inst->scale.z = CS_PODIUM_CHARACTER_SCALE;
		}

		if ((u32)(GAME_TRACKER->levelID - GEM_STONE_VALLEY) < CS_ADV_HUB_LEVEL_COUNT)
		{
			inst->depthBiasNormal -= 4;
			inst->depthBiasSecondary -= 4;
		}

		{
			initData->derivedRot.x = initData->rot.x;
			initData->derivedRot.y = initData->rot.y + yawOffset;
			initData->derivedRot.z = initData->rot.z;
			ConvertRotToMatrix(&inst->matrix, SVec3Slot_AsVec3(&initData->derivedRot));
		}
		cs->baseRotY = ANG_MODULO_TWO_PI(initData->derivedRot.y);
		cs->rot.x = ANG_MODULO_TWO_PI(initData->derivedRot.x);
		cs->rot.y = ANG_MODULO_TWO_PI(initData->derivedRot.y);
		cs->rot.z = ANG_MODULO_TWO_PI(initData->derivedRot.z);
	}

	cs->particleID = CS_DEFAULT_PARTICLE_ID;
	cs->pathProgress32 = 0;
	cs->lodIndex = 0;
	cs->flags = 0;
	cs->scaleSpeed = 0;
	cs->desiredScale = CS_PODIUM_CHARACTER_SCALE;

	{
		struct IconGroup *icons = GAME_TRACKER->iconGroup[0];
		cs->unk4 = 0;
		cs->unk6 = 0;
		cs->unk8 = CS_DEFAULT_COLOR_WORD;
		cs->unk_C = 0;
		cs->unk_E = 0;

		cs->ptrIcons = icons + 1;
	}

	return t;
}

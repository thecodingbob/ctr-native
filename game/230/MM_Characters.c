#include <common.h>
#include <custom-characters/character_resources.h>
#include <widescreen.h>
#include <platform/custom-characters/native_character_library.h>
#include <platform/native_assets.h>
#include <platform/native_renderer.h>
#include "character_select_grid.h"
#include <game/custom-characters/character_policy.h>

enum
{
	MM_CHARACTER_SELECT_SCREEN_W = 0x200,
	MM_CHARACTER_SELECT_SCREEN_H = 0xd8,
	MM_CHARACTER_SELECT_DISTANCE_TO_SCREEN = 0x100,
	MM_CHARACTER_SELECT_MODEL_MOVE_FP = 0x1000,
	MM_CHARACTER_SELECT_MODEL_MOVE_FP_SHIFT = 0xc,
	MM_CHARACTER_SELECT_MODEL_MOVE_NEXT = 1,
	MM_CHARACTER_SELECT_MODEL_MOVE_PREV = -1,
	MM_CHARACTER_SELECT_ICON_COUNT = 0x10,
	MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT = 8,
	MM_CHARACTER_SELECT_MAX_PLAYERS = 4,
	MM_CHARACTER_SELECT_LIMITED_LAYOUT_OFFSET = 4,
	MM_CHARACTER_SELECT_FULL_LAYOUT_COUNT = 2,
	MM_CHARACTER_SELECT_TRANSITION_FRAMES = 0xc,
	MM_CHARACTER_SELECT_TRANSITION_STEP = 8,
	MM_CHARACTER_SELECT_ANGLE_STEP = 0x400,
	MM_CHARACTER_SELECT_ANGLE_OFFSET = 400,
	MM_CHARACTER_SELECT_SPIN_STEP = 0x40,
	MM_CHARACTER_SELECT_LAYOUT_1P = 0,
	MM_CHARACTER_SELECT_LAYOUT_2P = 1,
	MM_CHARACTER_SELECT_LAYOUT_3P = 2,
	MM_CHARACTER_SELECT_LAYOUT_4P = 3,
	MM_CHARACTER_SELECT_LAYOUT_1P_LIMITED = 4,
	MM_CHARACTER_SELECT_LAYOUT_2P_LIMITED = 5,
	MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX = CHARACTER_SELECT_TITLE_TRANSITION,
	MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST = CHARACTER_SELECT_FIRST_WINDOW_TRANSITION,
	MM_CHARACTER_SELECT_3P_TITLE_X = 0x9c,
	MM_CHARACTER_SELECT_3P_SELECT_Y = 0x14,
	MM_CHARACTER_SELECT_3P_CHARACTER_Y = 0x26,
	MM_CHARACTER_SELECT_4P_TITLE_X = 0xfc,
	MM_CHARACTER_SELECT_4P_SELECT_Y = 8,
	MM_CHARACTER_SELECT_4P_CHARACTER_Y = 0x18,
	MM_CHARACTER_SELECT_LIMITED_TITLE_X = 0xfc,
	MM_CHARACTER_SELECT_LIMITED_TITLE_Y = 10,
	MM_CHARACTER_SELECT_INPUT_DPAD = BTN_RIGHT | BTN_LEFT | BTN_DOWN | BTN_UP,
	MM_CHARACTER_SELECT_INPUT_MENU = BTN_TRIANGLE | BTN_CIRCLE | BTN_SQUARE_one | BTN_CROSS_one,
	MM_CHARACTER_SELECT_INPUT_CONFIRM = BTN_CIRCLE | BTN_CROSS_one,
	MM_CHARACTER_SELECT_INPUT_BACK = BTN_TRIANGLE | BTN_SQUARE_one,
	MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_X = 6,
	MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_Y = 4,
	MM_CHARACTER_SELECT_ICON_RECT_W = 0x34,
	MM_CHARACTER_SELECT_ICON_RECT_H = 0x21,
	MM_CHARACTER_SELECT_WHEEL_SIZE = 0xccc,
	MM_CHARACTER_SELECT_CURSOR_LABEL_OFFSET_X = -6,
	MM_CHARACTER_SELECT_CURSOR_LABEL_OFFSET_Y = -3,
	MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_X = 3,
	MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_Y = 2,
	MM_CHARACTER_SELECT_HIGHLIGHT_W = 0x2e,
	MM_CHARACTER_SELECT_HIGHLIGHT_H = 0x1d,
	MM_CHARACTER_SELECT_SELECTED_BORDER_COUNT = 2,
	MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_X = 3,
	MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_Y = 2,
	MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_W = 6,
	MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_H = 4,
	MM_CHARACTER_SELECT_4P_NAME_BOTTOM_OFFSET = -6,
	MM_CHARACTER_SELECT_COLOR_PHASE_FRAME_STEP = 0x100,
	MM_CHARACTER_SELECT_COLOR_PHASE_PLAYER_STEP = 0x400,
	MM_CHARACTER_SELECT_COLOR_TRIG_MASK = 0x3ff,
	MM_CHARACTER_SELECT_COLOR_TRIG_HIGH_HALF_BIT = 0x400,
	MM_CHARACTER_SELECT_COLOR_TRIG_NEGATE_BIT = 0x800,
	MM_CHARACTER_SELECT_COLOR_PULSE_THRESHOLD = 0xc00,
	MM_CHARACTER_SELECT_COLOR_PULSE_SCALE_SHIFT = 7,
	MM_CHARACTER_SELECT_COLOR_PULSE_FP_SHIFT = 0xc,
	MM_CHARACTER_UNLOCK_TROPY = 5,
	MM_CHARACTER_UNLOCK_PENTA = 6,
	MM_CHARACTER_UNLOCK_ROO = 7,
	MM_CHARACTER_UNLOCK_PAPU = 8,
	MM_CHARACTER_UNLOCK_JOE = 9,
	MM_CHARACTER_UNLOCK_PINSTRIPE = 10,
	MM_CHARACTER_UNLOCK_FAKE_CRASH = 11,
};

static int MM_Characters_GridX(int x)
{
	const int halfWidth = MM_CHARACTER_SELECT_ICON_RECT_W / 2;
	const int center = (MM_ACTIVE_CHARACTER_SELECT_META[0].posX + MM_ACTIVE_CHARACTER_SELECT_META[3].posX) / 2 + halfWidth;
	return center + Widescreen_ScaleWidth(x + halfWidth - center) - halfWidth;
}

// NOTE(aalhendi): These local seams reproduce the few instructions whose
// register choices GCC 2.8.1 cannot express through C alone. Native builds
// retain the same game-state operations without the PSX scheduling controls.
#define MM_CHARACTERS_LOAD_OUTLINE_GAME_TRACKER(gameTracker)    ((gameTracker) = (u32)GAME_TRACKER)
#define MM_CHARACTERS_LOAD_OUTER_LOOP_GAME_TRACKER(gameTracker) ((gameTracker) = GAME_TRACKER)
#define MM_CHARACTERS_BEGIN_ICON_COLOR_COPY(characterMetadata)  ((void)(characterMetadata))
#define MM_CHARACTERS_END_ICON_COLOR_COPY()                     ((void)0)
#define MM_CHARACTERS_LOAD_HIGHLIGHT_GAME_TRACKER(gameTracker, transitionY, metadataY) \
	do                                                                                 \
	{                                                                                  \
		(gameTracker) = GAME_TRACKER;                                                  \
		(transitionY) += (metadataY);                                                  \
	} while (0)

static u32 s_characterSelectIconTexture;
static const struct NativeCharacterMetadata *s_characterSelectCustomEntries[CHARACTER_SELECT_EXTRA_SLOT_CAPACITY];
static int s_characterSelectCustomCount;

static const struct NativeCharacterMetadata *MM_Characters_CustomEntry(int entry)
{
	int index = entry - CHARACTER_SELECT_RETAIL_SLOT_COUNT;
	return index >= 0 && index < s_characterSelectCustomCount ? s_characterSelectCustomEntries[index] : NULL;
}

static struct CharacterGrid *MM_Characters_ActiveGrid(void)
{
	int layout = MM_CHARACTER_SELECT_LAYOUT_INDEX;
	int grid = layout >= MM_CHARACTER_SELECT_LAYOUT_1P_LIMITED ? 3 : layout < MM_CHARACTER_SELECT_LAYOUT_3P ? 0 : layout - 1;
	return &D230.characterSelectGrids[grid];
}

static const char *MM_Characters_GetPreviewIdentity(int playerIndex)
{
	if (playerIndex != 0 || D230.nativeCharacterPreviewCurrent[playerIndex] < CHARACTER_SELECT_RETAIL_SLOT_COUNT)
	{
		return NULL;
	}

	const struct NativeCharacterMetadata *entry = MM_Characters_CustomEntry(D230.nativeCharacterPreviewCurrent[playerIndex]);
	return entry != NULL ? entry->identity : NULL;
}

static b32 MM_Characters_IsUnlocked(const struct CharacterSelectMeta *meta)
{
	for (int entry = CHARACTER_SELECT_RETAIL_SLOT_COUNT; entry < D230.nativeCharacterSlotCount; entry++)
	{
		if (meta == &D230.characterSelectEntries[entry])
		{
			return true;
		}
	}
    if (meta->characterID == NITROS_OXIDE)
    {
      return g_config.unlockNitrosOxide;
    }

    return (s16)meta->unlockFlags == MM_CHARACTER_UNLOCK_ALWAYS ||
           CHECK_ADV_BIT(GAME_PROGRESS.unlocks, meta->unlockFlags) ||
            g_config.unlockAllCharacters;
}

static const u16 s_characterUnlockRequirements[MM_CHARACTER_SELECT_ICON_COUNT] = {
	[CRASH_BANDICOOT] = MM_CHARACTER_UNLOCK_ALWAYS,
	[NEO_CORTEX] = MM_CHARACTER_UNLOCK_ALWAYS,
	[TINY_TIGER] = MM_CHARACTER_UNLOCK_ALWAYS,
	[COCO_BANDICOOT] = MM_CHARACTER_UNLOCK_ALWAYS,
	[N_GIN] = MM_CHARACTER_UNLOCK_ALWAYS,
	[DINGODILE] = MM_CHARACTER_UNLOCK_ALWAYS,
	[POLAR] = MM_CHARACTER_UNLOCK_ALWAYS,
	[PURA] = MM_CHARACTER_UNLOCK_ALWAYS,
	[PINSTRIPE] = MM_CHARACTER_UNLOCK_PINSTRIPE, [PAPU_PAPU] = MM_CHARACTER_UNLOCK_PAPU,
	[RIPPER_ROO] = MM_CHARACTER_UNLOCK_ROO, [KOMODO_JOE] = MM_CHARACTER_UNLOCK_JOE,
	[N_TROPY] = MM_CHARACTER_UNLOCK_TROPY, [PENTA_PENGUIN] = MM_CHARACTER_UNLOCK_PENTA,
	[FAKE_CRASH] = MM_CHARACTER_UNLOCK_FAKE_CRASH,
	[NITROS_OXIDE] = MM_CHARACTER_UNLOCK_ALWAYS,
};

b32 MM_Characters_IsCharacterUnlocked(s16 characterID)
{
	if ((u16)characterID >= MM_CHARACTER_SELECT_ICON_COUNT)
	{
		return false;
	}

	const struct CharacterSelectMeta meta = {.characterID = characterID, .unlockFlags = s_characterUnlockRequirements[characterID]};
	return MM_Characters_IsUnlocked(&meta);
}

void MM_Characters_AnimateColors(u8 *colorData, s16 playerID, s16 flag)
{
	s32 colorAdjustmentValue;
	s32 scaledColorAdjustment;
	register u8 *colorOutput CTR_PSX_REGISTER("$8");
	register u8 *ptrColor CTR_PSX_REGISTER("$7");
	register s32 trigApprox CTR_PSX_REGISTER("$4");
	u32 trigApproximationIndex;

	colorOutput = colorData;

	// access int RGBA as a char array,
	// for editing components of color
	ptrColor = (u8 *)MM_COLOR_POINTERS[playerID + PLAYER_BLUE];

	trigApprox = 0;

	// if player has not selected character yet
	// see MM_Characters_MenuProc
	if (flag == 0)
	{
		trigApprox = (u32)MM_TRIG_APPROX;
		trigApproximationIndex = (s16)MM_FRAME_COUNTER * MM_CHARACTER_SELECT_COLOR_PHASE_FRAME_STEP + playerID * MM_CHARACTER_SELECT_COLOR_PHASE_PLAYER_STEP;

		// approximate trigonometry
		trigApprox = (s32)CTR_ReadU32AlignedLE(&((struct TrigTable *)trigApprox)[trigApproximationIndex & MM_CHARACTER_SELECT_COLOR_TRIG_MASK]);

		if ((trigApproximationIndex & MM_CHARACTER_SELECT_COLOR_TRIG_HIGH_HALF_BIT) == 0)
		{
			trigApprox = trigApprox << 0x10;
		}
		trigApprox = trigApprox >> 0x10;

		if ((trigApproximationIndex & MM_CHARACTER_SELECT_COLOR_TRIG_NEGATE_BIT) != 0)
		{
			trigApprox = -(int)trigApprox;
		}
	}

	colorAdjustmentValue = 0;
	if (MM_CHARACTER_SELECT_COLOR_PULSE_THRESHOLD < trigApprox)
	{
		scaledColorAdjustment = trigApprox << MM_CHARACTER_SELECT_COLOR_PULSE_SCALE_SHIFT;
		colorAdjustmentValue = scaledColorAdjustment >> MM_CHARACTER_SELECT_COLOR_PULSE_FP_SHIFT;
		CTR_PSX_KEEP_VALUE(trigApprox);
	}

	colorOutput[0] = ptrColor[0] | colorAdjustmentValue;
	colorOutput[1] = ptrColor[1] | colorAdjustmentValue;
	colorOutput[2] = ptrColor[2] | colorAdjustmentValue;
	colorOutput[3] = 0;

	return;
}
s32 MM_Characters_GetNextDriver(s32 direction, s16 characterID)
{
	bool selectable[CHARACTER_SELECT_TITLE_TRANSITION] = {false};
	for (int i = 0; i < D230.nativeCharacterSlotCount; i++)
	{
		selectable[i] = MM_Characters_IsUnlocked(&MM_ACTIVE_CHARACTER_SELECT_META[i]);
	}
	return CharacterSelectGrid_Next(MM_Characters_ActiveGrid(), characterID, direction, selectable);
}
b32 MM_Characters_boolIsInvalid(s16 *iconPerPlayer, s16 characterID, s32 player)
{
	register b32 isInvalid CTR_PSX_REGISTER("$8");
	s16 playerToSkip;
	u8 playerCount;
	s16 playerIndex;

	isInvalid = false;
	playerToSkip = (s16)player;
	playerCount = GAME_TRACKER->numPlyrNextGame;

	// loop through players
	for (playerIndex = 0; playerIndex < playerCount; playerIndex++)
	{
		// if driver is taken
		if ((playerIndex != playerToSkip) && (characterID == iconPerPlayer[playerIndex]))
		{
			isInvalid = true;
			break;
		}
	}

	// if driver is not taken
	return isInvalid;
}

void MM_Characters_DrawWindows(b32 showDrivers)
{
	register b32 boolShowDrivers CTR_PSX_REGISTER("$22");
	register s16 *desiredCharacterIDs CTR_PSX_REGISTER("$21");
	register u32 dataPointer CTR_PSX_REGISTER("$2");
	register u32 transitionOffsetOrPointer CTR_PSX_REGISTER("$3");
	register struct PushBuffer *pb CTR_PSX_REGISTER("$8");
	struct Instance *driverInst;
	register struct GameTracker *loopGameTracker CTR_PSX_REGISTER("$5");
	struct Model *model;
	register SVec2 *windowPos CTR_PSX_REGISTER("$7");
	s16 *currCharacterID;
	s16 *moveTimer;
	SVec3 rot;
	s32 indexOrSlideFactor;
	register s32 playerWindowIndex CTR_PSX_REGISTER("$4");
	register s32 pushBufferOffset CTR_PSX_REGISTER("$5");
	register u32 gameTrackerPage CTR_PSX_REGISTER("$6");
	s16 playerIndex;
	s16 moveFrames;
	s16 nextMoveTimer;

	boolShowDrivers = showDrivers;

	if (boolShowDrivers != 0)
	{
		// enable drawing wheels
		GAME_TRACKER->renderFlags |= RENDER_FLAG_TIRES;
	}

	CTR_PSX_LOAD_SYMBOL_PAGE(gameTrackerPage, RETAIL_GAME_TRACKER_ASM_NAME);
	playerIndex = 0;
	if (CTR_PSX_PAGE_LVALUE(struct GameTracker *, gameTrackerPage, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER)->numPlyrNextGame == 0)
	{
		goto LAB_Characters_DrawWindows_End;
	}

	CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_CHARACTER_SELECT_DESIRED_IDS_ASM_NAME);
	CTR_PSX_ADD_SYMBOL_LOW(desiredCharacterIDs, dataPointer, MM_CHARACTER_SELECT_DESIRED_IDS_ASM_NAME, MM_CHARACTER_SELECT_DESIRED_IDS);

LAB_Characters_DrawWindows_Loop:
	playerWindowIndex = playerIndex;
	pushBufferOffset = playerWindowIndex * sizeof(*pb) + offsetof(struct GameTracker, pushBuffer);
	CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS_ASM_NAME);
	transitionOffsetOrPointer = playerWindowIndex * sizeof(*windowPos);
	CTR_PSX_LOAD_WORD_FROM_PAGE(windowPos, dataPointer, MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS_ASM_NAME, MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS);
	CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_CHARACTER_SELECT_TRANSITION_META_ASM_NAME);
	CTR_PSX_LOAD_WORD_FROM_PAGE(dataPointer, dataPointer, MM_CHARACTER_SELECT_TRANSITION_META_ASM_NAME, (u32)MM_CHARACTER_SELECT_TRANSITION_META);
	CTR_PSX_LOAD_WORD_FROM_PAGE(gameTrackerPage, gameTrackerPage, RETAIL_GAME_TRACKER_ASM_NAME, (u32)GAME_TRACKER);
	windowPos = (SVec2 *)(transitionOffsetOrPointer + (u32)windowPos);
	transitionOffsetOrPointer += playerWindowIndex;
	transitionOffsetOrPointer <<= 1;
	transitionOffsetOrPointer += dataPointer;
	transitionOffsetOrPointer += MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST * sizeof(struct TransitionMeta);
	pb = (struct PushBuffer *)(gameTrackerPage + pushBufferOffset);
	pb->rect.x = windowPos->x + ((struct TransitionMeta *)transitionOffsetOrPointer)->currX;
	pb->rect.y = windowPos->y + ((struct TransitionMeta *)transitionOffsetOrPointer)->currY;
	pb->rect.w = MM_CHARACTER_SELECT_WINDOW_WIDTH;
	pb->rect.h = MM_CHARACTER_SELECT_WINDOW_HEIGHT;

	// negative StartX
	if ((s16)pb->rect.x < 0)
	{
		pb->rect.w -= pb->rect.x;
		pb->rect.x = 0;
		if ((s16)pb->rect.w < 0)
		{
			pb->rect.w = 0;
		}
	}

	// negative StartY
	if ((s16)pb->rect.y < 0)
	{
		pb->rect.h -= pb->rect.y;
		pb->rect.y = 0;
		if ((s16)pb->rect.h < 0)
		{
			pb->rect.h = 0;
		}
	}

	{
		register s32 rectPosition CTR_PSX_REGISTER("$4");
		register s32 rectEnd CTR_PSX_REGISTER("$2");
		register s32 rectSize CTR_PSX_REGISTER("$3");

		// startX + sizeX out of bounds
		rectEnd = (s16)pb->rect.x;
		rectSize = (s16)pb->rect.w;
		CTR_PSX_RELOAD(pb->rect.x);
		rectPosition = (u16)pb->rect.x;
		rectEnd += rectSize;
		rectEnd = rectEnd < MM_CHARACTER_SELECT_SCREEN_W + 1;
		CTR_PSX_RELOAD(pb->rect.w);
		rectSize = (u16)pb->rect.w;
		CTR_PSX_OBSERVE_VALUE(rectSize);
		if (rectEnd == 0)
		{
			rectEnd = MM_CHARACTER_SELECT_SCREEN_W;
			rectEnd -= rectPosition;
			pb->rect.w = rectEnd;
			if ((s16)rectEnd < 0)
			{
				rectEnd = MM_CHARACTER_SELECT_SCREEN_W;
				pb->rect.x = rectEnd;
				pb->rect.w = 0;

				// NOTE(aalhendi): Native renderer guard; retail leaves w at zero.
				pb->rect.w = 1;
			}
		}

		// startY + sizeY out of bounds
		rectEnd = (s16)pb->rect.y;
		rectSize = (s16)pb->rect.h;
		CTR_PSX_RELOAD(pb->rect.y);
		rectPosition = (u16)pb->rect.y;
		rectEnd += rectSize;
		rectEnd = rectEnd < MM_CHARACTER_SELECT_SCREEN_H + 1;
		CTR_PSX_RELOAD(pb->rect.h);
		rectSize = (u16)pb->rect.h;
		CTR_PSX_OBSERVE_VALUE(rectSize);
		if (rectEnd == 0)
		{
			rectEnd = MM_CHARACTER_SELECT_SCREEN_H;
			rectEnd -= rectPosition;
			pb->rect.h = rectEnd;
			if ((s16)rectEnd < 0)
			{
				rectEnd = MM_CHARACTER_SELECT_SCREEN_H;
				pb->rect.y = rectEnd;
				pb->rect.h = 0;

				// NOTE(aalhendi): Native renderer guard; retail leaves h at zero.
				pb->rect.h = 1;
			}
		}
	}

	// distanceToScreen
	pb->distanceToScreen_CURR = MM_CHARACTER_SELECT_DISTANCE_TO_SCREEN;
	pb->distanceToScreen_PREV = MM_CHARACTER_SELECT_DISTANCE_TO_SCREEN;

	CTR_PSX_CLOBBER("$2");
	CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, RETAIL_GAME_TRACKER_ASM_NAME);
	CTR_PSX_CLOBBER("$4");
	playerWindowIndex = playerIndex;
	CTR_PSX_OBSERVE_VALUE(playerWindowIndex);
	CTR_PSX_LOAD_WORD_FROM_PAGE_AFTER(loopGameTracker, dataPointer, RETAIL_GAME_TRACKER_ASM_NAME, GAME_TRACKER, playerWindowIndex);
	dataPointer = playerWindowIndex * sizeof(loopGameTracker->drivers[0]);
	CTR_PSX_OBSERVE_VALUE(dataPointer);

	// pushBuffer pos and rot to all zero
	pb->pos.x = 0;
	pb->pos.y = 0;
	pb->pos.z = 0;
	pb->rot.x = 0;
	pb->rot.y = 0;
	pb->rot.z = 0;
	CTR_PSX_DEPEND_MEMORY(&pb->rot.z, dataPointer);

	// player -> instance
	dataPointer = (u32)loopGameTracker + dataPointer;
	driverInst = ((struct GameTracker *)dataPointer)->drivers[0]->instSelf;

	// Make Visible
	driverInst->flags &= ~HIDE_MODEL;

	// if driver is off-screen
	if ((loopGameTracker->numPlyrNextGame <= playerWindowIndex) || (boolShowDrivers == 0))
	{
		// invisible
		driverInst->flags |= HIDE_MODEL;
	}

	indexOrSlideFactor = playerIndex;

	// clear pushBuffer in every InstDrawPerPlayer
	driverInst->idpp[0].pushBuffer = 0;
	driverInst->idpp[1].pushBuffer = 0;
	driverInst->idpp[2].pushBuffer = 0;
	driverInst->idpp[3].pushBuffer = 0;

	// set pushBuffer in InstDrawPerPlayer,
	// so that each camera can only see one driver
	driverInst->idpp[indexOrSlideFactor].pushBuffer = pb;

	CTR_PSX_CLOBBER("$2");
	CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_CHARACTER_SELECT_CURRENT_IDS_ASM_NAME);
	CTR_PSX_ADD_SYMBOL_LOW(dataPointer, dataPointer, MM_CHARACTER_SELECT_CURRENT_IDS_ASM_NAME, (u32)MM_CHARACTER_SELECT_CURRENT_IDS);
	currCharacterID = (s16 *)(indexOrSlideFactor * sizeof(*currCharacterID) + dataPointer);

	// CameraDC, freecam mode
	GAME_TRACKER->cameraDC[indexOrSlideFactor].cameraMode = CAMERA_MODE_FREECAM;

	// Set position of player
	CTR_PSX_CLOBBER("$3");
	CTR_PSX_LOAD_SYMBOL_PAGE(transitionOffsetOrPointer, MM_CHARACTER_SELECT_DRIVER_POS_ASM_NAME);
	dataPointer = CTR_PSX_PAGE_LVALUE(s16, transitionOffsetOrPointer, MM_CHARACTER_SELECT_DRIVER_POS_PAGE_OFFSET, MM_CHARACTER_SELECT_DRIVER_POS.x);
	CTR_PSX_ADD_SYMBOL_LOW(transitionOffsetOrPointer, transitionOffsetOrPointer, MM_CHARACTER_SELECT_DRIVER_POS_ASM_NAME, (u32)&MM_CHARACTER_SELECT_DRIVER_POS);
	driverInst->matrix.t[0] = dataPointer;
	dataPointer = ((s16 *)transitionOffsetOrPointer)[1];
	driverInst->matrix.t[1] = dataPointer;
	dataPointer = ((s16 *)transitionOffsetOrPointer)[2];
	driverInst->matrix.t[2] = dataPointer;

	moveTimer = &MM_CHARACTER_SELECT_MOVE_TIMERS[indexOrSlideFactor];

	// if transition between players
	if (*moveTimer != 0)
	{
		register s32 slideProduct CTR_PSX_REGISTER("$9");

		// get timer
		nextMoveTimer = *moveTimer + -1;
		*moveTimer = nextMoveTimer;
		moveFrames = MM_CHARACTER_SELECT_MOVE_FRAMES;

		// if timer is before midpoint
		if ((int)nextMoveTimer < (int)moveFrames)
		{
			s32 moveFrameScale;

			// make driver fly off screen
			*currCharacterID = desiredCharacterIDs[indexOrSlideFactor];
			D230.nativeCharacterPreviewCurrent[playerIndex] = D230.nativeCharacterPreviewDesired[playerIndex];
			moveFrameScale = RaceFlag_MoveModels((int)nextMoveTimer, (int)moveFrames);

			transitionOffsetOrPointer = (s16)MM_CHARACTER_SELECT_SLIDE_DISTANCE;
			slideProduct = moveFrameScale * (s32)transitionOffsetOrPointer;
			CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_CHARACTER_SELECT_MOVE_DIR_ASM_NAME);
			CTR_PSX_ADD_SYMBOL_LOW(dataPointer, dataPointer, MM_CHARACTER_SELECT_MOVE_DIR_ASM_NAME, (u32)MM_CHARACTER_SELECT_MOVE_DIR);
			dataPointer = (indexOrSlideFactor * (s32)sizeof(s16)) + dataPointer;
			dataPointer = *(s16 *)dataPointer;
			CTR_PSX_OBSERVE_VALUE(slideProduct);
			dataPointer = -(s32)dataPointer;
			CTR_PSX_OBSERVE_VALUE(dataPointer);
			transitionOffsetOrPointer = slideProduct >> MM_CHARACTER_SELECT_MODEL_MOVE_FP_SHIFT;
		}

		// if timer is after midpoint
		else
		{
			register s32 moveFactor CTR_PSX_REGISTER("$4");
			s32 moveFrameScale;

			// make new driver fly on screen
			moveFrameScale = RaceFlag_MoveModels((int)nextMoveTimer - (int)moveFrames, (int)moveFrames);

			moveFactor = MM_CHARACTER_SELECT_MODEL_MOVE_FP;
			transitionOffsetOrPointer = (s16)MM_CHARACTER_SELECT_SLIDE_DISTANCE;
			moveFactor -= moveFrameScale;
			slideProduct = moveFactor * (s32)transitionOffsetOrPointer;
			CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_CHARACTER_SELECT_MOVE_DIR_ASM_NAME);
			CTR_PSX_ADD_SYMBOL_LOW(dataPointer, dataPointer, MM_CHARACTER_SELECT_MOVE_DIR_ASM_NAME, (u32)MM_CHARACTER_SELECT_MOVE_DIR);
			dataPointer = (indexOrSlideFactor * (s32)sizeof(s16)) + dataPointer;
			CTR_PSX_OBSERVE_VALUE(slideProduct);
			transitionOffsetOrPointer = *(s16 *)dataPointer;
			CTR_PSX_OBSERVE_VALUE(transitionOffsetOrPointer);
			dataPointer = slideProduct >> MM_CHARACTER_SELECT_MODEL_MOVE_FP_SHIFT;
		}

		slideProduct = (s32)dataPointer * (s32)transitionOffsetOrPointer;
		driverInst->matrix.t[0] += slideProduct;
	}

	// If no transition between players
	else
	{
		int selectedEntry = CharacterSelectGrid_At(MM_Characters_ActiveGrid(), D230.nativeCharacterCursor[playerIndex]);
		if (*currCharacterID != GAME_CHARACTER_IDS[indexOrSlideFactor] || D230.nativeCharacterPreviewCurrent[playerIndex] != selectedEntry)
		{
			*moveTimer = MM_CHARACTER_SELECT_MOVE_FRAMES * 2;
			desiredCharacterIDs[indexOrSlideFactor] = GAME_CHARACTER_IDS[indexOrSlideFactor];
			D230.nativeCharacterPreviewDesired[playerIndex] = selectedEntry;
		}
	}

	VehBirth_SetWheelSizeForCharacter(loopGameTracker->drivers[indexOrSlideFactor], *currCharacterID);
	driverInst->animFrame = 0;
	driverInst->animIndex = 0;
	model = LOAD_GetCharacterPreviewModel(playerIndex, *currCharacterID, MM_Characters_GetPreviewIdentity(playerIndex));
	driverInst->model = model;

	{
		register s32 rotationY CTR_PSX_REGISTER("$6");
		register MATRIX *driverMatrix CTR_PSX_REGISTER("$4");

		// driver rotation
		CTR_PSX_LOAD_SYMBOL_PAGE(dataPointer, MM_CHARACTER_SELECT_DRIVER_ROT_ASM_NAME);
		CTR_PSX_ADD_SYMBOL_LOW(transitionOffsetOrPointer, dataPointer, MM_CHARACTER_SELECT_DRIVER_ROT_ASM_NAME, (u32)&MM_CHARACTER_SELECT_DRIVER_ROT);
		dataPointer = CTR_PSX_PAGE_LVALUE(u16, dataPointer, MM_CHARACTER_SELECT_DRIVER_ROT_PAGE_OFFSET, (u16)MM_CHARACTER_SELECT_DRIVER_ROT.x);
		rotationY = ((u16 *)transitionOffsetOrPointer)[1];
		transitionOffsetOrPointer = ((u16 *)transitionOffsetOrPointer)[2];
		driverMatrix = &driverInst->matrix;
		rot.z = transitionOffsetOrPointer;
		CTR_PSX_LOAD_SYMBOL_PAGE(transitionOffsetOrPointer, MM_CHARACTER_SELECT_ANGLE_ASM_NAME);
		CTR_PSX_ADD_SYMBOL_LOW(transitionOffsetOrPointer, transitionOffsetOrPointer, MM_CHARACTER_SELECT_ANGLE_ASM_NAME, (u32)MM_CHARACTER_SELECT_ANGLE);
		rot.x = dataPointer;
		rot.y = rotationY;
		dataPointer = ((u16 *)transitionOffsetOrPointer)[playerIndex];
		rotationY += dataPointer;
		rot.y = rotationY;

		ConvertRotToMatrix(driverMatrix, &rot);
	}
	playerIndex++;
	CTR_PSX_LOAD_SYMBOL_PAGE(gameTrackerPage, RETAIL_GAME_TRACKER_ASM_NAME);
	CTR_PSX_LOAD_WORD_FROM_PAGE(transitionOffsetOrPointer, gameTrackerPage, RETAIL_GAME_TRACKER_ASM_NAME, (u32)GAME_TRACKER);
	if (playerIndex < ((struct GameTracker *)transitionOffsetOrPointer)->numPlyrNextGame)
	{
		goto LAB_Characters_DrawWindows_Loop;
	}

LAB_Characters_DrawWindows_End:
	return;
}
void MM_Characters_SetMenuLayout(void)
{
	b16 expandRoster;
	s16 layoutIndex;

	expandRoster = s_characterSelectCustomCount > 0;

	layoutIndex = GAME_TRACKER->numPlyrNextGame - 1;
	// By default, draw "Select character" in 3P menu
	MM_CHARACTER_SELECT_ROSTER_EXPANDED = expandRoster;

	// Loop through bottom characters,
	// if any are unlocked, use expanded
	const int expandedCharacters[] = {KOMODO_JOE, PENTA_PENGUIN, FAKE_CRASH, NITROS_OXIDE};
	for (size_t i = 0; i < len(expandedCharacters); i++)
	{
		if (MM_Characters_IsCharacterUnlocked(expandedCharacters[i]))
		{
			expandRoster = true;
			MM_CHARACTER_SELECT_ROSTER_EXPANDED = true;
		}
	}

	if (
	    // if 1P or 2P
	    (GAME_TRACKER->numPlyrNextGame < MM_CHARACTER_SELECT_FULL_LAYOUT_COUNT + 1) &&

	    // if very few characters are unlocked
	    (!expandRoster))
	{
		// layout [4] and [5] for 1P2P without expansion
		layoutIndex += MM_CHARACTER_SELECT_LIMITED_LAYOUT_OFFSET;
	}

	MM_CHARACTER_SELECT_LAYOUT_INDEX = layoutIndex;

	MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS = MM_CHARACTER_SELECT_WINDOW_POS_BY_LAYOUT[layoutIndex];
	MM_ACTIVE_CHARACTER_SELECT_META = D230.characterSelectEntries;
	D230.nativeCharacterSlotCount = CHARACTER_SELECT_RETAIL_SLOT_COUNT + s_characterSelectCustomCount;
	struct CharacterGrid *grid = MM_Characters_ActiveGrid();
	CharacterSelectGrid_FillCustom(grid, s_characterSelectCustomCount);
	for (int entry = 0; entry < D230.nativeCharacterSlotCount; entry++)
	{
		struct CharacterSelectMeta *meta = &D230.characterSelectEntries[entry];
		struct CharacterGridPosition position = CharacterSelectGrid_Find(grid, entry);
		meta->posX = grid->columnX[position.column];
		meta->posY = grid->rowY[position.row];
		const struct NativeCharacterMetadata *custom = MM_Characters_CustomEntry(entry);
		meta->characterID = custom == NULL ? entry : CustomCharacter_GetBaseID(custom->base);
		meta->unlockFlags = s_characterUnlockRequirements[meta->characterID];
	}

	MM_CHARACTER_SELECT_WINDOW_WIDTH = MM_CHARACTER_SELECT_LAYOUT_WINDOW_W[layoutIndex];
	MM_CHARACTER_SELECT_WINDOW_HEIGHT = MM_CHARACTER_SELECT_LAYOUT_WINDOW_H[layoutIndex];
	MM_CHARACTER_SELECT_NAME_TEXT_Y = MM_CHARACTER_SELECT_LAYOUT_TEXT_Y[layoutIndex];
	MM_CHARACTER_SELECT_DRIVER_POS.y = MM_CHARACTER_SELECT_LAYOUT_DRIVER_POS_Y[layoutIndex];
	MM_CHARACTER_SELECT_DRIVER_POS.z = MM_CHARACTER_SELECT_LAYOUT_DRIVER_POS_Z[layoutIndex];

	MM_CHARACTER_SELECT_TRANSITION_META = MM_CHARACTER_SELECT_TRANSITION_BY_PLAYER_COUNT[GAME_TRACKER->numPlyrNextGame - 1];
}
void MM_Characters_BackupIDs(void)
{
	s16 driverIndex;

	for (driverIndex = 0; driverIndex < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT; driverIndex++)
	{
		// make a backup when you leave character selection,
		// backup is restored when you go back to selection
		MM_CHARACTER_IDS_BACKUP[driverIndex] = GAME_CHARACTER_IDS[driverIndex];
	}
	return;
}
void MM_Characters_PreventOverlap(void)
{
	struct PackedCharacterIDs
	{
		s8 values[MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT];
	};
	struct PackedCharacterIDs availableDefaultCharacters;
	s8 *defaultCharacter;
	s8 freeCharacter;
	s32 characterID;
	s16 defaultIndex;
	s16 previousPlayer;
	s16 playerIndex;

	// default 0,1,2,3,4,5,6,7
	memcpy(&availableDefaultCharacters, MM_DEFAULT_CHARACTER_ID_WORDS, sizeof(availableDefaultCharacters));

	for (playerIndex = 0; playerIndex < GAME_TRACKER->numPlyrNextGame; playerIndex++)
	{
		// get character ID
		characterID = GAME_CHARACTER_IDS[playerIndex];

		// if not a secret character
		if (characterID < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT)
		{
			// character is taken
			availableDefaultCharacters.values[characterID] = -1;
		}
	}

	for (playerIndex = 1; playerIndex < GAME_TRACKER->numPlyrNextGame; playerIndex++)
	{
		for (previousPlayer = 0; previousPlayer < playerIndex; previousPlayer++)
		{
			// if two characters are the same
			if (GAME_CHARACTER_IDS[playerIndex] == GAME_CHARACTER_IDS[previousPlayer])
			{
				// look for a new character
				for (defaultIndex = 0; defaultIndex < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT; defaultIndex++)
				{
					// get default character
					defaultCharacter = &availableDefaultCharacters.values[defaultIndex];
					freeCharacter = *defaultCharacter;

					// if character is not taken
					if (-1 < freeCharacter)
					{
						// assign free character
						GAME_CHARACTER_IDS[playerIndex] = (s16)freeCharacter;

						// character is now taken
						*defaultCharacter = -1;

						break;
					}
				}
			}
		}
	}
	return;
}

void MM_Characters_RestoreIDs(void)
{
	s16 playerIndex;
	s16 driverIndex;
	char characterDirectory[NATIVE_CHARACTER_PATH_BYTES];
	if (NativeAssets_BuildPath("characters", characterDirectory, sizeof(characterDirectory)))
	{
		NativeCharacterLibrary_Refresh(characterDirectory);
	}
	MM_ACTIVE_CHARACTER_SELECT_META = NULL;
	s_characterSelectIconTexture = 0;
	s_characterSelectCustomCount = 0;
	for (size_t index = 0; index < NativeCharacterLibrary_Count() && s_characterSelectCustomCount < CHARACTER_SELECT_EXTRA_SLOT_CAPACITY; index++)
	{
		const struct NativeCharacterMetadata *entry = NativeCharacterLibrary_Get(index);
		if (entry != NULL && CustomCharacter_GetBaseID(entry->base) >= 0)
		{
			s_characterSelectCustomEntries[s_characterSelectCustomCount++] = entry;
		}
	}
	const struct NativeCharacterMetadata *custom = MM_Characters_CustomEntry(CHARACTER_SELECT_RETAIL_SLOT_COUNT);
	if (custom != NULL)
	{
		u16 customIconPixels[44 * 26];
		if (NativeCharacterLibrary_ReadIcon(custom, customIconPixels))
		{
			s_characterSelectIconTexture = NativeRenderer_UploadCharacterIcon(customIconPixels);
		}
	}

	memset(sdata->selectedEngineClass, 0, sizeof(sdata->selectedEngineClass));

	// erase select bits
	MM_CHARACTER_SELECT_FLAGS = 0;
	MM_CHARACTER_SELECT_TRANSITION_FRAME = MM_CHARACTER_SELECT_TRANSITION_FRAMES;
	MM_CHARACTER_SELECT_MENU_STATE = ENTERING_MENU;

	// This uses 80086e84, which controls character IDs
	for (driverIndex = 0; driverIndex < MM_CHARACTER_SELECT_DEFAULT_DRIVER_COUNT; driverIndex++)
	{
		// set character ID to the last ID you entered
		GAME_CHARACTER_IDS[driverIndex] = MM_CHARACTER_IDS_BACKUP[driverIndex];
	}

	MM_Characters_SetMenuLayout();

	{
		struct CharacterSelectMeta *activeMeta;
		struct GameTracker *gGT;
		register struct GameTracker *loopGT CTR_PSX_REGISTER("$7");
		register struct GameProgress *progress CTR_PSX_REGISTER("$9");
		register s32 unlockAlways CTR_PSX_REGISTER("$10");
		u32 progressPage;
		s16 *characterIDs;
		s16 unlockPlayerIndex;

		gGT = GAME_TRACKER;
		unlockPlayerIndex = 0;

		if (gGT->numPlyrNextGame != 0)
		{
			characterIDs = GAME_CHARACTER_IDS;
			activeMeta = MM_ACTIVE_CHARACTER_SELECT_META;
			unlockAlways = MM_CHARACTER_UNLOCK_ALWAYS;
			CTR_PSX_LOAD_SYMBOL_PAGE(progressPage, RETAIL_GAME_SAVE_ASM_NAME);
			CTR_PSX_ADD_SYMBOL_LOW(progress, progressPage, RETAIL_GAME_SAVE_ASM_NAME, &GAME_PROGRESS);
			CTR_PSX_CLOBBER("$7");
			loopGT = gGT;

			do
			{
				s16 *currID;
				s16 unlocked;

				// Determine if this icon is unlocked (and drawing)
				currID = (s16 *)((u32)(unlockPlayerIndex * sizeof(*currID)) + (u32)characterIDs);
				unlocked = activeMeta[(s32)*currID].unlockFlags;

				if (unlocked != unlockAlways)
				{
					// NOTE(aalhendi): Keep the shared loop increment available to both
					// retail branch delay slots without changing native control flow.
					CTR_PSX_MEMORY_BARRIER();
				    if (!MM_Characters_IsUnlocked(&activeMeta[(s32)*currID]))
				    {
				      *currID = CRASH_BANDICOOT;
				    }
				}
				unlockPlayerIndex++;
			} while (unlockPlayerIndex < loopGT->numPlyrNextGame);
		}
	}

	MM_Characters_PreventOverlap();

	for (playerIndex = 0; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
	{
		D230.nativeCharacterCursor[playerIndex] = CharacterSelectGrid_Find(MM_Characters_ActiveGrid(), CRASH_BANDICOOT);
		D230.nativeCharacterPreviewCurrent[playerIndex] = CRASH_BANDICOOT;
		D230.nativeCharacterPreviewDesired[playerIndex] = CRASH_BANDICOOT;
	}

	for (playerIndex = 0; playerIndex < GAME_TRACKER->numPlyrNextGame; playerIndex++)
	{
		s16 characterID;

		// set name string ID to the character ID of each player.
		// The string will only draw if both these variables match
		characterID = MM_CHARACTER_SELECT_DESIRED_IDS[playerIndex] = (MM_CHARACTER_SELECT_CURRENT_IDS[playerIndex] = GAME_CHARACTER_IDS[playerIndex]);
		(void)characterID;

		// something to do with transitioning between icons
		MM_CHARACTER_SELECT_MOVE_TIMERS[playerIndex] = 0;

		// rotation of each driver, 90 degrees difference
		MM_CHARACTER_SELECT_ANGLE[playerIndex] = (playerIndex * MM_CHARACTER_SELECT_ANGLE_STEP) + MM_CHARACTER_SELECT_ANGLE_OFFSET;
		D230.nativeCharacterCursor[playerIndex] = CharacterSelectGrid_Find(MM_Characters_ActiveGrid(), GAME_CHARACTER_IDS[playerIndex]);
		if (playerIndex == 0 && CustomCharacter_IsSelected())
		{
			for (int index = 0; index < s_characterSelectCustomCount; index++)
			{
				if (strcmp(sdata->customPlayerIdentity, s_characterSelectCustomEntries[index]->identity) == 0)
				{
					D230.nativeCharacterCursor[0] = CharacterSelectGrid_Find(MM_Characters_ActiveGrid(), CHARACTER_SELECT_RETAIL_SLOT_COUNT + index);
					break;
				}
			}
		}
		int entry = CharacterSelectGrid_At(MM_Characters_ActiveGrid(), D230.nativeCharacterCursor[playerIndex]);
		D230.nativeCharacterPreviewCurrent[playerIndex] = entry;
		D230.nativeCharacterPreviewDesired[playerIndex] = entry;
	}

	MM_Characters_DrawWindows(0);
	return;
}

void MM_Characters_HideDrivers(void)
{
	s16 playerIndex;

	for (playerIndex = 0; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
	{
		PushBuffer_Init(&GAME_TRACKER->pushBuffer[playerIndex], 0, 1);

		GAME_TRACKER->drivers[playerIndex]->instSelf->flags |= HIDE_MODEL;
	}

	return;
}

static inline u8 MM_Characters_ScaleColor(u8 color)
{
	return (u8)((int)((u32)color << 2) / 5);
}

void MM_Characters_MenuProc(struct RectMenu *unused)
{
	s16 iconPerPlayer[4];
	s16 playerIndex;
	register s16 *iconMetaTail CTR_PSX_REGISTER("$16");
	SVec2 *windowPos;
	RECT drawRect;
	s32 iconPosX;
	u32 outerGameTrackerPage;
	register struct GameTracker *outerGameTracker CTR_PSX_REGISTER("$2");
	register struct GameTracker *outerLoopGameTracker CTR_PSX_REGISTER("$3");
	struct CharacterSelectMeta *preInputCharacterMeta;


	(void)unused;


	for (playerIndex = 0; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
	{
		int entry = CharacterSelectGrid_At(MM_Characters_ActiveGrid(), D230.nativeCharacterCursor[playerIndex]);
		iconPerPlayer[playerIndex] = entry >= 0 ? entry : CRASH_BANDICOOT;
	}

	switch (MM_CHARACTER_SELECT_MENU_STATE)
	{
	case ENTERING_MENU:
		MM_TransitionInOut(MM_CHARACTER_SELECT_TRANSITION_META, (int)MM_CHARACTER_SELECT_TRANSITION_FRAME, MM_CHARACTER_SELECT_TRANSITION_STEP);
		MM_Characters_SetMenuLayout();
		MM_Characters_DrawWindows(1);

		if (MM_CHARACTER_SELECT_TRANSITION_FRAME != 0)
		{
			MM_CHARACTER_SELECT_TRANSITION_FRAME--;
		}
		else
		{
			MM_CHARACTER_SELECT_MENU_STATE = IN_MENU;
		}
		break;

	case IN_MENU:
		MM_Characters_SetMenuLayout();
		MM_Characters_DrawWindows(1);
		break;

	case EXITING_MENU:
		MM_TransitionInOut(MM_CHARACTER_SELECT_TRANSITION_META, (int)MM_CHARACTER_SELECT_TRANSITION_FRAME, MM_CHARACTER_SELECT_TRANSITION_STEP);
		MM_Characters_SetMenuLayout();
		MM_Characters_DrawWindows(1);

		// increase frame
		MM_CHARACTER_SELECT_TRANSITION_FRAME++;

		// if more than 12 frames
		if (MM_CHARACTER_SELECT_TRANSITION_FRAME > MM_CHARACTER_SELECT_TRANSITION_FRAMES)
		{
			// Make a backup of the characters
			// you selected in character selection screen
			MM_Characters_BackupIDs();
			MM_NewLoad_OnCharacterSelectExit(MM_CHARACTER_SELECT_EXITS_FORWARD != 0);

			// if advancing to cup or track selection
			if (MM_CHARACTER_SELECT_EXITS_FORWARD != 0)
			{
				MM_Characters_HideDrivers();

				if (g_config.expandedAdventureOptions &&
				    ((GAME_TRACKER->gameMode1 & ADVENTURE_MODE) != 0))
				{
					GAME_ADV_PROGRESS.characterID = GAME_CHARACTER_IDS[0];

					// The second racer is chosen on the same screen, so the new
					// profile saves the partner together with both racer count and
					// first character.
					if (sdata->advMultiplayer.numPlayers > 1)
					{
						sdata->advMultiplayer.characterID2 = GAME_CHARACTER_IDS[1];
					}

					MM_DESIRED_MENU = &data.menuSubmitName;
					SubmitName_RestoreName(0);
					return;
				}

				// if you are in a cup
				if ((GAME_TRACKER->gameMode2 & CUP_ANY_KIND) != 0)
				{
					MM_DESIRED_MENU = &MM_MENU_CUP_SELECT;
					MM_CupSelect_Init();
					return;
				}

				// if going to track selection
				MM_DESIRED_MENU = &MM_MENU_TRACK_SELECT;
				MM_TrackSelect_Init();
				return;
			}

			MM_Characters_HideDrivers();
			return;
		}
		break;

	default:
		break;
	}

	switch (MM_CHARACTER_SELECT_LAYOUT_INDEX)
	{
	case MM_CHARACTER_SELECT_LAYOUT_1P:
	case MM_CHARACTER_SELECT_LAYOUT_2P:
		goto dontDrawSelectCharacter;

	// 3P character selection
	case MM_CHARACTER_SELECT_LAYOUT_3P:

		// If you have a lot of characters unlocked, do not draw SELECT CHARACTER
		if (MM_CHARACTER_SELECT_ROSTER_EXPANDED)
		{
			goto dontDrawSelectCharacter;
		}

		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_SELECT_CHARACTER_SELECT],
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currX + MM_CHARACTER_SELECT_3P_TITLE_X,
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currY + MM_CHARACTER_SELECT_3P_SELECT_Y, FONT_BIG,
		                   (JUSTIFY_CENTER | ORANGE));
		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_CHARACTER],
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currX + MM_CHARACTER_SELECT_3P_TITLE_X,
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currY + MM_CHARACTER_SELECT_3P_CHARACTER_Y, FONT_BIG,
		                   (JUSTIFY_CENTER | ORANGE));
		break;

	// 4P character selection
	case MM_CHARACTER_SELECT_LAYOUT_4P:

		// If Fake Crash is unlocked, do not draw "Select Character"
		if (D230.nativeCharacterSlotCount > CHARACTER_SELECT_RETAIL_SLOT_COUNT || (GAME_PROGRESS.unlocks[0] & UNLOCK_FAKE_CRASH) != 0)
		{
			goto dontDrawSelectCharacter;
		}

		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_SELECT_CHARACTER_SELECT],
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currX + MM_CHARACTER_SELECT_4P_TITLE_X,
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currY + MM_CHARACTER_SELECT_4P_SELECT_Y,
		                   FONT_CREDITS, (JUSTIFY_CENTER | ORANGE));
		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_CHARACTER],
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currX + MM_CHARACTER_SELECT_4P_TITLE_X,
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currY + MM_CHARACTER_SELECT_4P_CHARACTER_Y,
		                   FONT_CREDITS, (JUSTIFY_CENTER | ORANGE));
		break;

	// If you are in 1P or 2P character selection,
	// when you do NOT have a lot of characters selected
	case MM_CHARACTER_SELECT_LAYOUT_1P_LIMITED:
	case MM_CHARACTER_SELECT_LAYOUT_2P_LIMITED:
		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_SELECT_CHARACTER],
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currX + MM_CHARACTER_SELECT_LIMITED_TITLE_X,
		                   MM_CHARACTER_SELECT_TRANSITION_META[MM_CHARACTER_SELECT_TITLE_TRANSITION_INDEX].currY + MM_CHARACTER_SELECT_LIMITED_TITLE_Y,
		                   FONT_BIG, (JUSTIFY_CENTER | ORANGE));
		break;

	default:
		goto dontDrawSelectCharacter;
	}

dontDrawSelectCharacter:
	outerGameTrackerPage = MM_GAME_TRACKER_PAGE_VALUE;
	outerGameTracker = CTR_PSX_PAGE_LVALUE(struct GameTracker *volatile, outerGameTrackerPage, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER);
	CTR_PSX_OBSERVE_VALUE(outerGameTrackerPage);
	playerIndex = 0;
	if (playerIndex >= outerGameTracker->numPlyrNextGame)
	{
		goto outerPlayerLoopComplete;
	}
	do
	{
		b32 playerSelectedBeforeInput;
		struct TransitionMeta *currentIconTransition;
		struct TransitionMeta *currentIconTransitionBase;
		s32 currentIconTransitionOffset;
		b32 playerSelectedAfterInput;
		const Color *outlineColor;
		u32 outlineGameTrackerPage;
		u32 candidateIcon;
		s32 candidateIconIndex;
		Color playerColor;
		u32 button;

		candidateIcon = (u16)iconPerPlayer[playerIndex];
		MM_Characters_AnimateColors((u8 *)&playerColor, playerIndex, (int)(s16)(MM_CHARACTER_SELECT_FLAGS & (u16)(1 << playerIndex)));

		preInputCharacterMeta = &MM_ACTIVE_CHARACTER_SELECT_META[(s16)candidateIcon];
		if (g_config.engineSelection && MM_CHARACTER_SELECT_MENU_STATE == IN_MENU &&
		    (MM_CHARACTER_SELECT_FLAGS & (1 << playerIndex)) == 0)
		{
			const u32 engineButtons = MM_GAME_BUTTON_TAPS[playerIndex] & (BTN_L1 | BTN_R1);
			if (engineButtons != 0)
			{
				const int defaultClass = GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[playerIndex]].engineID;
				int engineClass = VehBirth_EngineClassForPlayer(playerIndex, defaultClass);
				engineClass = (engineClass + ((engineButtons & BTN_L1) != 0 ? NUM_CLASSES - 1 : 1)) % NUM_CLASSES;
				sdata->selectedEngineClass[playerIndex] = engineClass + 1;
				MM_GAME_BUTTON_TAPS[playerIndex] &= ~(BTN_L1 | BTN_R1);
				OtherFX_Play(0, 1);
			}
		}

		if ((MM_CHARACTER_SELECT_MENU_STATE == IN_MENU) &&
		    // If you press the D-Pad, or Cross, Square, Triangle, Circle
		    (((button = MM_GAME_BUTTON_TAPS[playerIndex]) & (MM_CHARACTER_SELECT_INPUT_DPAD | MM_CHARACTER_SELECT_INPUT_MENU)) != 0))
		{
			playerSelectedBeforeInput = (((int)(s16)MM_CHARACTER_SELECT_FLAGS >> playerIndex) & 1U) != 0;

			// if character has not been selected by this player
			if (!playerSelectedBeforeInput)
			{
				// If you pressed any of the D-pad buttons
				if ((button & MM_CHARACTER_SELECT_INPUT_DPAD) != 0)
				{
					int inputDirection;
					if ((button & BTN_UP) != 0)
					{
						inputDirection = CHARACTER_SELECT_DIR_UP;
						MM_CHARACTER_SELECT_MOVE_DIR[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_PREV;
					}
					else if ((button & BTN_DOWN) != 0)
					{
						inputDirection = CHARACTER_SELECT_DIR_DOWN;
						MM_CHARACTER_SELECT_MOVE_DIR[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_NEXT;
					}
					else if ((button & BTN_LEFT) != 0)
					{
						inputDirection = CHARACTER_SELECT_DIR_LEFT;
						MM_CHARACTER_SELECT_MOVE_DIR[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_PREV;
					}
					else
					{
						inputDirection = CHARACTER_SELECT_DIR_RIGHT;
						MM_CHARACTER_SELECT_MOVE_DIR[playerIndex] = MM_CHARACTER_SELECT_MODEL_MOVE_NEXT;
					}

					bool selectable[CHARACTER_SELECT_TITLE_TRANSITION] = {false};
					for (int entry = 0; entry < D230.nativeCharacterSlotCount; entry++)
					{
						selectable[entry] = MM_Characters_IsUnlocked(&MM_ACTIVE_CHARACTER_SELECT_META[entry]) &&
						                    !MM_Characters_boolIsInvalid(iconPerPlayer, entry, playerIndex);
					}
					int next = CharacterSelectGrid_Next(MM_Characters_ActiveGrid(), candidateIcon, inputDirection, selectable);
					if (next != (int)candidateIcon) { OtherFX_Play(0, 1); }
					candidateIcon = next;
				}

				// If this player pressed Cross or Circle
				if ((MM_GAME_BUTTON_TAPS[playerIndex] & MM_CHARACTER_SELECT_INPUT_CONFIRM) != 0)
				{
					u8 numPlyrNextGame;
					u32 selectedPlayerMask;

					// this player has now selected a character
					MM_CHARACTER_SELECT_FLAGS = MM_CHARACTER_SELECT_FLAGS | (u16)(1 << playerIndex);

					numPlyrNextGame = GAME_TRACKER_RELOAD()->numPlyrNextGame;
					selectedPlayerMask = ((0xffU << numPlyrNextGame) ^ 0xffU) & 0xffU;

					// Play sound
					OtherFX_Play(1, 1);

					// if all players have selected their characters
					if ((u32)(int)(s16)MM_CHARACTER_SELECT_FLAGS == selectedPlayerMask)
					{
						// exit toward cup or track selection
						MM_CHARACTER_SELECT_EXITS_FORWARD = 1;
						MM_CHARACTER_SELECT_MENU_STATE = EXITING_MENU;
					}
				}

				if (
				    // if this is the first iteration of the loop
				    (playerIndex == 0) &&

				    // if you press Square or Triangle
				    ((MM_GAME_BUTTON_TAPS[0] & MM_CHARACTER_SELECT_INPUT_BACK) != 0))
				{
					// return to main menu
					MM_CHARACTER_SELECT_EXITS_FORWARD = 0;
					MM_CHARACTER_SELECT_MENU_STATE = EXITING_MENU;

					// Play sound
					OtherFX_Play(2, 1);
				}
			}
			else if ((button & MM_CHARACTER_SELECT_INPUT_BACK) != 0)
			{
				// if you press Square or Triangle
				// Play sound
				OtherFX_Play(2, 1);

				// this player has de-selected their character
				MM_CHARACTER_SELECT_FLAGS = MM_CHARACTER_SELECT_FLAGS & ~(u16)(1 << playerIndex);
			}

			// clear input
			MM_GAME_BUTTON_TAPS[playerIndex] = 0;
		}

		candidateIconIndex = (s16)candidateIcon;
		if (candidateIconIndex != iconPerPlayer[playerIndex])
		{
			sdata->selectedEngineClass[playerIndex] = 0;
		}
		iconPerPlayer[playerIndex] = candidateIcon;

		// transition of each icon
		currentIconTransitionBase = MM_CHARACTER_SELECT_TRANSITION_META;
		currentIconTransitionOffset = candidateIconIndex * sizeof(*currentIconTransition);
		currentIconTransition = (struct TransitionMeta *)(currentIconTransitionOffset + (u32)currentIconTransitionBase);

		// NOTE(aalhendi): Three observations retain GCC 2.8.1's retail pre-draw schedule.
		CTR_PSX_OBSERVE_VALUE(preInputCharacterMeta);
		CTR_PSX_OBSERVE_VALUE(preInputCharacterMeta);
		CTR_PSX_OBSERVE_VALUE(preInputCharacterMeta);

		drawRect.x = MM_Characters_GridX(currentIconTransition->currX + preInputCharacterMeta->posX);
		drawRect.y = currentIconTransition->currY + preInputCharacterMeta->posY;
		drawRect.w = MM_CHARACTER_SELECT_ICON_RECT_W;
		drawRect.h = MM_CHARACTER_SELECT_ICON_RECT_H;
		Widescreen_CompressRect(&drawRect);

		// if player has not selected a character
		playerSelectedAfterInput = (((int)(s16)MM_CHARACTER_SELECT_FLAGS >> playerIndex) & 1U) != 0;

		if (!playerSelectedAfterInput)
		{
			// draw string
			// "1", "2", "3", "4", above the character icon
			DecalFont_DrawLine(MM_PLAYER_NUMBER_STRINGS[playerIndex], drawRect.x + MM_CHARACTER_SELECT_CURSOR_LABEL_OFFSET_X,
			                   currentIconTransition->currY + (u32)preInputCharacterMeta->posY - 3, FONT_BIG, WHITE);
			outlineColor = &playerColor;
		}
		else
		{
			outlineColor = &MM_CHARACTER_SELECT_OUTLINE_COLOR;
		}

		MM_CHARACTERS_LOAD_OUTLINE_GAME_TRACKER(outlineGameTrackerPage);
		RECTMENU_DrawOuterRect_HighLevel(&drawRect, outlineColor, 0, ((struct GameTracker *)outlineGameTrackerPage)->backBuffer->otMem.uiOT);
		playerIndex++;
		MM_CHARACTERS_LOAD_OUTER_LOOP_GAME_TRACKER(outerLoopGameTracker);
	} while (playerIndex < outerLoopGameTracker->numPlyrNextGame);

outerPlayerLoopComplete:
	MM_Characters_PreventOverlap();

	{
		preInputCharacterMeta = MM_ACTIVE_CHARACTER_SELECT_META;
		iconMetaTail = &preInputCharacterMeta->posY;

		// loop through character icons
		for (playerIndex = 0; playerIndex < D230.nativeCharacterSlotCount; playerIndex++, iconMetaTail += 6, preInputCharacterMeta++)
		{
			if (MM_Characters_IsUnlocked(preInputCharacterMeta))
			{
				register struct TransitionMeta *iconTransition CTR_PSX_REGISTER("$3");
				register struct MetaDataCHAR *iconCharacterMetadata CTR_PSX_REGISTER("$4");
				register u32 iconGameTrackerPage CTR_PSX_REGISTER("$7");
				register u32 iconWork CTR_PSX_REGISTER("$2");
				const Color *iconColor;
				s32 iconTransitionIndex;
				s32 iconTransitionByteOffset;
				s16 selectedPlayerIndex;

				iconColor = &MM_CHARACTER_SELECT_NEUTRAL_COLOR;

				for (selectedPlayerIndex = 0; selectedPlayerIndex < GAME_TRACKER->numPlyrNextGame; selectedPlayerIndex++)
				{
					if (((s16)playerIndex == iconPerPlayer[selectedPlayerIndex]) &&

					    // if player selected a character
					    ((((int)(s16)MM_CHARACTER_SELECT_FLAGS >> selectedPlayerIndex) & 1U) != 0))
					{
						iconColor = &MM_CHARACTER_SELECT_CHOSEN_COLOR;
					}
				}

				iconTransitionIndex = playerIndex;
				iconWork = (s16)iconTransitionIndex;
				iconCharacterMetadata = (struct MetaDataCHAR *)MM_CHARACTER_SELECT_TRANSITION_POINTER_PAGE_VALUE;
				CTR_PSX_FORGET_VALUE(iconCharacterMetadata);
				iconTransitionByteOffset = iconWork * sizeof(*iconTransition);
				CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(iconGameTrackerPage, RETAIL_GAME_TRACKER_ASM_NAME, iconTransitionByteOffset);
				CTR_PSX_LOAD_WORD_FROM_PAGE(iconWork, iconCharacterMetadata, MM_CHARACTER_SELECT_TRANSITION_META_ASM_NAME,
				                            (u32)MM_CHARACTER_SELECT_TRANSITION_META);
				iconCharacterMetadata = (struct MetaDataCHAR *)MM_CHARACTER_METADATA_PAGE_VALUE;
				CTR_PSX_FORGET_VALUE(iconCharacterMetadata);
				iconCharacterMetadata = GAME_CHARACTER_METADATA;
				MM_CHARACTERS_BEGIN_ICON_COLOR_COPY(iconCharacterMetadata);
				iconTransitionByteOffset += iconWork;
				iconTransition = (struct TransitionMeta *)iconTransitionByteOffset;
				iconPosX = MM_Characters_GridX(iconTransition->currX + preInputCharacterMeta->posX) + MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_X;
				struct Icon customIcon = {0};
				b32 drawCustomIcon = playerIndex >= CHARACTER_SELECT_RETAIL_SLOT_COUNT && s_characterSelectIconTexture != 0;
				struct PrimMem *iconMemory = &GAME_TRACKER->backBuffer->primMem;
				if (drawCustomIcon && ((uintptr_t)iconMemory->cursor > (uintptr_t)iconMemory->guardEnd ||
				    (uintptr_t)iconMemory->guardEnd - (uintptr_t)iconMemory->cursor < 2 * sizeof(DR_PSYX_TEX) + sizeof(POLY_GT4)))
				{
					continue;
				}
				if (drawCustomIcon)
				{
					customIcon.texLayout.u1 = customIcon.texLayout.u3 = 43;
					customIcon.texLayout.v2 = customIcon.texLayout.v3 = 25;
					customIcon.texLayout.tpage = 0x100;
					DR_PSYX_TEX *reset = GAME_TRACKER->backBuffer->primMem.cursor;
					SetPsyXTexture(reset, 0, 0, 0);
					AddPrim(GAME_TRACKER->pushBuffer_UI.ptrOT, reset);
					GAME_TRACKER->backBuffer->primMem.cursor = reset + 1;
				}

				MM_RECTMENU_DRAW_POLY_GT4(
				    drawCustomIcon ? &customIcon : CTR_PSX_PAGE_LVALUE(struct GameTracker *, iconGameTrackerPage, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER)
				        ->ptrIcons[iconCharacterMetadata[iconMetaTail[3]].iconID],
				    iconPosX, iconTransition->currY + iconMetaTail[0] + MM_CHARACTER_SELECT_ICON_DECAL_OFFSET_Y,

				    &CTR_PSX_PAGE_LVALUE(struct GameTracker *, iconGameTrackerPage, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER)->backBuffer->primMem,
				    CTR_PSX_PAGE_LVALUE(struct GameTracker *, iconGameTrackerPage, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER)->pushBuffer_UI.ptrOT,

				    *iconColor, *iconColor, *iconColor, *iconColor, drawCustomIcon ? 0 : TRANS_50_DECAL, FP(1.0));
				if (drawCustomIcon)
				{
					DR_PSYX_TEX *bind = GAME_TRACKER->backBuffer->primMem.cursor;
					SetPsyXTexture(bind, s_characterSelectIconTexture, 44, 26);
					AddPrim(GAME_TRACKER->pushBuffer_UI.ptrOT, bind);
					GAME_TRACKER->backBuffer->primMem.cursor = bind + 1;
				}
				MM_CHARACTERS_END_ICON_COLOR_COPY();
			}
		}
	}

	{
		register struct CharacterSelectMeta *characterIDMeta CTR_PSX_REGISTER("$6");
		s16 *characterIDOutput;
		s16 *iconPerPlayerBase;
		s16 *iconPerPlayerAtIndex;
		s32 iconPerPlayerOffset;

		playerIndex = 0;
		characterIDOutput = GAME_CHARACTER_IDS;
		iconPerPlayerBase = iconPerPlayer;
		characterIDMeta = MM_ACTIVE_CHARACTER_SELECT_META;

		for (; playerIndex < MM_CHARACTER_SELECT_MAX_PLAYERS; playerIndex++)
		{
			iconPerPlayerOffset = playerIndex * sizeof(*iconPerPlayerBase);
			iconPerPlayerAtIndex = iconPerPlayerBase;
			iconPerPlayerAtIndex = (s16 *)((u8 *)iconPerPlayerAtIndex + iconPerPlayerOffset);
			characterIDOutput[playerIndex] = characterIDMeta[(int)*iconPerPlayerAtIndex].characterID;
			D230.nativeCharacterCursor[playerIndex] = CharacterSelectGrid_Find(MM_Characters_ActiveGrid(), *iconPerPlayerAtIndex);
			if (playerIndex == 0)
			{
				const struct NativeCharacterMetadata *custom = MM_Characters_CustomEntry(*iconPerPlayerAtIndex);
				CustomCharacter_Select(custom);
			}
		}
	}

	playerIndex = 0;
	if (GAME_TRACKER->numPlyrNextGame != 0)
	{
		Color animatedColor;
		register Color *animatedColorPtr CTR_PSX_REGISTER("$20");

		animatedColorPtr = &animatedColor;

		do
		{
			s32 menuPlayerIndex;
			s16 playerIcon;
			b32 playerSelected;

			menuPlayerIndex = playerIndex;
			playerIcon = iconPerPlayer[menuPlayerIndex];
			preInputCharacterMeta = &MM_ACTIVE_CHARACTER_SELECT_META[playerIcon];
			playerSelected = (((int)(s16)MM_CHARACTER_SELECT_FLAGS >> menuPlayerIndex) & 1U) != 0;

			// if player has not selected a character
			if (!playerSelected)
			{
				register struct GameTracker *highlightGameTracker CTR_PSX_REGISTER("$2");
				register RECT *highlightRect CTR_PSX_REGISTER("$4");
				register s32 highlightTransitionY CTR_PSX_REGISTER("$3");
				register s32 highlightMetadataY CTR_PSX_REGISTER("$5");
				u16 selectedPlayerFlag;

				selectedPlayerFlag = (u16)(1 << menuPlayerIndex);
				MM_Characters_AnimateColors((u8 *)animatedColorPtr, menuPlayerIndex,

				                            // flags of which characters are selected
				                            (int)(s16)(MM_CHARACTER_SELECT_FLAGS & selectedPlayerFlag));

				animatedColor.r = MM_Characters_ScaleColor(animatedColor.r);
				animatedColor.g = MM_Characters_ScaleColor(animatedColor.g);
				animatedColor.b = MM_Characters_ScaleColor(animatedColor.b);

				drawRect.x = MM_Characters_GridX(MM_CHARACTER_SELECT_TRANSITION_META[playerIcon].currX + preInputCharacterMeta->posX) +
				             MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_X;
				highlightRect = &drawRect;
				CTR_PSX_OBSERVE_VALUE(highlightRect);
				highlightTransitionY = (u16)MM_CHARACTER_SELECT_TRANSITION_META[playerIcon].currY;
				highlightMetadataY = (u16)preInputCharacterMeta->posY;
				CTR_PSX_OBSERVE_VALUE(highlightTransitionY);
				CTR_PSX_OBSERVE_VALUE(highlightMetadataY);
				drawRect.w = MM_CHARACTER_SELECT_HIGHLIGHT_W;
				drawRect.h = MM_CHARACTER_SELECT_HIGHLIGHT_H;
				MM_CHARACTERS_LOAD_HIGHLIGHT_GAME_TRACKER(highlightGameTracker, highlightTransitionY, highlightMetadataY);
				drawRect.y = highlightTransitionY + MM_CHARACTER_SELECT_HIGHLIGHT_OFFSET_Y;
				Widescreen_CompressRect(&drawRect);

				// this draws the flashing blue square that appears when you highlight a character in the character select screen
				MM_DRAW_SOLID_BOX_WITH_PRIM_MEM(highlightRect, animatedColorPtr, highlightGameTracker->backBuffer->otMem.uiOT,
				                                &highlightGameTracker->backBuffer->primMem);
			}
			if ((MM_CHARACTER_SELECT_MOVE_TIMERS[menuPlayerIndex] == 0) &&
			    (MM_CHARACTER_SELECT_CURRENT_IDS[menuPlayerIndex] == GAME_CHARACTER_IDS[menuPlayerIndex]) &&
			    D230.nativeCharacterPreviewCurrent[menuPlayerIndex] == playerIcon)
			{
				struct TransitionMeta *driverWindowTransition;
				struct TransitionMeta *transitionBase;
				char **characterNameSlot;
				char **localizedStrings;
				s32 characterNameOffset;
				s32 nameBaseY;
				s32 bottomNameYOffset;
				s32 namePosX;
				s32 namePosY;
				register s32 namePosYWord CTR_PSX_REGISTER("$2");
				s32 signedNameYOffset;
				s32 transitionWordOffset;
				s16 nameYOffset;
				u8 numPlyrNextGame;
				u8 fontType;
				s16 characterSelectWindowWidth;

				// get number of players
				numPlyrNextGame = GAME_TRACKER->numPlyrNextGame;
				nameYOffset = (s16)((((u32)(numPlyrNextGame < 3) ^ 1) << 0x12) >> 0x10);

				// if number of players is 1 or 2
				fontType = FONT_CREDITS;

				// if number of players is 3 or 4
				if (numPlyrNextGame >= 3)
				{
					fontType = FONT_SMALL;
				}

				localizedStrings = GAME_LANGUAGE_STRINGS;
				transitionWordOffset = menuPlayerIndex * (sizeof(*driverWindowTransition) / sizeof(s16));
				characterNameOffset = GAME_CHARACTER_METADATA[preInputCharacterMeta->characterID].name_LNG_long * sizeof(*localizedStrings);
				characterNameSlot = (char **)(characterNameOffset + (u32)localizedStrings);
				transitionBase = MM_CHARACTER_SELECT_TRANSITION_META;
				characterSelectWindowWidth = MM_CHARACTER_SELECT_WINDOW_WIDTH;
				driverWindowTransition =
				    (struct TransitionMeta *)((s16 *)transitionBase - -transitionWordOffset +
				                              MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST * (sizeof(*driverWindowTransition) / sizeof(s16)));
				namePosX = (s16)((u16)driverWindowTransition->currX + (u16)MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS[menuPlayerIndex].x +
				                 (int)characterSelectWindowWidth / 2);
				nameBaseY = driverWindowTransition->currY + MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS[menuPlayerIndex].y;
				signedNameYOffset = (s16)nameYOffset;

				if ((numPlyrNextGame != 4) || (menuPlayerIndex < 2))
				{
					s32 nameTextY;

					nameTextY = MM_CHARACTER_SELECT_NAME_TEXT_Y;
					nameYOffset = nameBaseY + nameTextY + signedNameYOffset;
				}
				else
				{
					bottomNameYOffset = signedNameYOffset + MM_CHARACTER_SELECT_4P_NAME_BOTTOM_OFFSET;
					nameYOffset = nameBaseY + bottomNameYOffset;
				}

				namePosYWord = nameYOffset << 16;
				namePosY = namePosYWord >> 16;
				CTR_PSX_OBSERVE_VALUE(namePosY);

				// draw string
				const struct NativeCharacterMetadata *customName = MM_Characters_CustomEntry(playerIcon);
				DecalFont_DrawLine(customName != NULL ? (char *)customName->name : *characterNameSlot, namePosX, namePosY, fontType, (JUSTIFY_CENTER | ORANGE));
			}

			// spin the character
			MM_CHARACTER_SELECT_ANGLE[playerIndex] += MM_CHARACTER_SELECT_SPIN_STEP;
			playerIndex++;
		} while (playerIndex < GAME_TRACKER->numPlyrNextGame);
	}

	// reset
	playerIndex = 0;
	preInputCharacterMeta = MM_ACTIVE_CHARACTER_SELECT_META;

	// loop through all icons
	for (; playerIndex < D230.nativeCharacterSlotCount; playerIndex++, preInputCharacterMeta++)
	{
		RECT iconRect;

		iconMetaTail = &preInputCharacterMeta->posY;

		if (MM_Characters_IsUnlocked(preInputCharacterMeta))
		{
			iconRect.x = MM_Characters_GridX(MM_CHARACTER_SELECT_TRANSITION_META[playerIndex].currX + preInputCharacterMeta->posX);
			iconRect.y = MM_CHARACTER_SELECT_TRANSITION_META[playerIndex].currY + (s16)iconMetaTail[0];
			iconRect.w = MM_CHARACTER_SELECT_ICON_RECT_W;
			iconRect.h = MM_CHARACTER_SELECT_ICON_RECT_H;
			Widescreen_CompressRect(&iconRect);

			// Draw 2D Menu rectangle background
			RECTMENU_DrawInnerRect(&iconRect, 0, GAME_TRACKER->backBuffer->otMem.uiOT);
		}
	}

	windowPos = MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS;

	for (playerIndex = 0; playerIndex < GAME_TRACKER->numPlyrNextGame; playerIndex++)
	{
		Color animatedColor;
		RECT windowRect;
		s32 menuPlayerIndex;

		menuPlayerIndex = playerIndex;

		// store window width and height in one 4-byte variable
		windowRect.x = MM_CHARACTER_SELECT_TRANSITION_META[menuPlayerIndex + MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST].currX + windowPos->x;
		windowRect.y = MM_CHARACTER_SELECT_TRANSITION_META[menuPlayerIndex + MM_CHARACTER_SELECT_DRIVER_WINDOW_TRANSITION_FIRST].currY + windowPos->y;
		windowRect.w = MM_CHARACTER_SELECT_WINDOW_WIDTH;
		windowRect.h = MM_CHARACTER_SELECT_WINDOW_HEIGHT;
		if (g_config.engineSelection && MM_CHARACTER_SELECT_MENU_STATE == IN_MENU)
		{
			const int defaultClass = GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[playerIndex]].engineID;
			const int engine = VehBirth_EngineClassForPlayer(playerIndex, defaultClass);
			if (windowRect.w >= 120 && windowRect.h >= 48)
			{
				const int overlayY = MM_CHARACTER_SELECT_LAYOUT_INDEX == MM_CHARACTER_SELECT_LAYOUT_4P && playerIndex >= 2
				                         ? windowRect.y + windowRect.h - 36
				                         : windowRect.y + 2;
				UI_EngineStats_Draw(windowRect.x + 4, overlayY, engine, ENGINE_STATS_SMALL, false);
			}
		}

		MM_Characters_AnimateColors((u8 *)&animatedColor, menuPlayerIndex,

		                            // flags of which characters are selected
		                            (((int)(s16)MM_CHARACTER_SELECT_FLAGS >> menuPlayerIndex) ^ 1U) & 1U);

		RECTMENU_DrawOuterRect_HighLevel(&windowRect, &animatedColor, 0, GAME_TRACKER->backBuffer->otMem.uiOT);

		// if player selected a character
		if ((((int)(s16)MM_CHARACTER_SELECT_FLAGS >> menuPlayerIndex) & 1U) != 0)
		{
			RECT r58 = windowRect;
			s16 borderIndex;

			for (borderIndex = 0; borderIndex < MM_CHARACTER_SELECT_SELECTED_BORDER_COUNT; borderIndex++)
			{
				r58.x += MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_X;
				r58.y += MM_CHARACTER_SELECT_SELECTED_BORDER_INSET_Y;
				r58.w -= MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_W;
				r58.h -= MM_CHARACTER_SELECT_SELECTED_BORDER_SHRINK_H;

				animatedColor.r = (u8)((int)((u32)animatedColor.r << 2) / 5);
				animatedColor.g = (u8)((int)((u32)animatedColor.g << 2) / 5);
				animatedColor.b = (u8)((int)((u32)animatedColor.b << 2) / 5);

				RECTMENU_DrawOuterRect_HighLevel(&r58, &animatedColor, 0, GAME_TRACKER->backBuffer->otMem.uiOT);
			}
		}
		// Draw 2D Menu rectangle background
		RECTMENU_DrawInnerRect(&windowRect, 9, &GAME_TRACKER->backBuffer->otMem.uiOT[3]);
		windowPos++;

		// not screen-space anymore,
		// this is viewport-space
		windowRect.x = 0;
		windowRect.y = 0;

		RECTMENU_DrawRwdBlueRect(&windowRect, &MM_CHARACTER_SELECT_BLUE_RECT_COLORS[0], &GAME_TRACKER->pushBuffer[playerIndex].ptrOT[0x3ff],
		                         &GAME_TRACKER->backBuffer->primMem);
	}
	return;
}

#undef MM_CHARACTERS_LOAD_OUTLINE_GAME_TRACKER
#undef MM_CHARACTERS_LOAD_OUTER_LOOP_GAME_TRACKER
#undef MM_CHARACTERS_LOAD_COLLISION_GAME_TRACKER
#undef MM_CHARACTERS_CAPTURE_FINAL_COLLISION_PAGE
#undef MM_CHARACTERS_BEGIN_ICON_COLOR_COPY
#undef MM_CHARACTERS_END_ICON_COLOR_COPY
#undef MM_CHARACTERS_LOAD_HIGHLIGHT_GAME_TRACKER

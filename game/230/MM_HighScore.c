#include <common.h>

enum
{
	MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME = 0xc,
	MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES = 8,
	MM_HIGHSCORE_LAST_ARCADE_TRACK = 0x11,
	MM_HIGHSCORE_TRACK_SLIDE_STEP_X = 0x40,
	MM_HIGHSCORE_ROW_SLIDE_STEP_Y = 0x1b,
	MM_HIGHSCORE_OFFSCREEN_X = 0x200,
	MM_HIGHSCORE_OFFSCREEN_Y = 0xd8,
	MM_HIGHSCORE_WIPE_RECT_W = 0x228,
	MM_HIGHSCORE_WIPE_RECT_H = 0x19,
	MM_HIGHSCORE_WIPE_RECT_X_OFFSET = -0x14,
	MM_HIGHSCORE_WIPE_RECT_Y_OFFSET = 9,
	MM_HIGHSCORE_TEXT_SHADOW_X = 3,
	MM_HIGHSCORE_TEXT_SHADOW_Y = 1,
	MM_HIGHSCORE_ARROW_ICON_GROUP = 4,
	MM_HIGHSCORE_ARROW_ICON_ID = 0x38,
	MM_HIGHSCORE_ARROW_LEFT_X_OFFSET = 0xec,
	MM_HIGHSCORE_ARROW_RIGHT_X_OFFSET = 0x112,
	MM_HIGHSCORE_ARROW_Y_OFFSET = 0x15,
	MM_HIGHSCORE_ARROW_SCALE = 0x1000,
	MM_HIGHSCORE_ARROW_LEFT_ROTATION = 0x800,
	MM_HIGHSCORE_TITLE_X_OFFSET = 0x100,
	MM_HIGHSCORE_TITLE_Y_OFFSET = 0xe,
	MM_HIGHSCORE_FLASH_TIMER_BIT = 4,
	MM_HIGHSCORE_BEST_TRACK_LABEL_X_OFFSET = 0x20,
	MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET = 0x2b,
	MM_HIGHSCORE_SCORE_MODE_TIME_TRIAL = 0,
	MM_HIGHSCORE_GHOST_STAR_COUNT = 2,
	MM_HIGHSCORE_GHOST_STAR_ICON_GROUP = 5,
	MM_HIGHSCORE_GHOST_STAR_ICON_ID = 0x37,
	MM_HIGHSCORE_GHOST_STAR_X_OFFSET = 0xf0,
	MM_HIGHSCORE_GHOST_STAR_X_STEP = 0x10,
	MM_HIGHSCORE_GHOST_STAR_Y_OFFSET = 4,
	MM_HIGHSCORE_GHOST_STAR_SCALE = 0x1000,
	MM_HIGHSCORE_TITLE_META_INDEX = 0,
	MM_HIGHSCORE_BEST_TRACK_META_INDEX = 1,
	MM_HIGHSCORE_BEST_LAP_LABEL_META_INDEX = 7,
	MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX = 8,
	MM_HIGHSCORE_BEST_LAP_LABEL_X_OFFSET = 0x124,
	MM_HIGHSCORE_BEST_LAP_LABEL_Y_OFFSET = 0x2b,
	MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET = 0x160,
	MM_HIGHSCORE_BEST_LAP_NAME_Y_OFFSET = 0x39,
	MM_HIGHSCORE_BEST_LAP_TIME_Y_OFFSET = 0x4a,
	MM_HIGHSCORE_BEST_LAP_ICON_X_OFFSET = 0x124,
	MM_HIGHSCORE_BEST_LAP_ICON_Y_OFFSET = 0x38,
	MM_HIGHSCORE_DRIVER_COLOR_OFFSET = 5,
	MM_HIGHSCORE_ICON_TRANSPARENCY = 1,
	MM_HIGHSCORE_ICON_SCALE = 0x1000,
	MM_HIGHSCORE_VISIBLE_SCORE_ROWS = 5,
	MM_HIGHSCORE_FIRST_VISIBLE_ENTRY = 1,
	MM_HIGHSCORE_FIRST_VISIBLE_META_INDEX = 2,
	MM_HIGHSCORE_SCORE_ROW_Y_STEP = 0x1f,
	MM_HIGHSCORE_SCORE_ICON_X_OFFSET = 0x20,
	MM_HIGHSCORE_SCORE_NAME_X_OFFSET = 0x5c,
	MM_HIGHSCORE_SCORE_NAME_Y_OFFSET = 0x39,
	MM_HIGHSCORE_SCORE_TIME_Y_OFFSET = 0x4a,
	MM_HIGHSCORE_VIDEO_META_INDEX = 9,
	MM_HIGHSCORE_MENU_META_INDEX = 10,
	MM_HIGHSCORE_VIDEO_BOX_W = 0xb0,
	MM_HIGHSCORE_VIDEO_BOX_H = 0x4b,
	MM_HIGHSCORE_VIDEO_BOX_X_OFFSET = 0x124,
	MM_HIGHSCORE_VIDEO_BOX_Y_OFFSET = 0x5a,
	MM_HIGHSCORE_MENU_WIDTH = 0xa4,
	MM_HIGHSCORE_TEXT_JUSTIFICATION_MASK = 0xffffc000u,
};

void MM_HighScore_Text3D(char *string, s32 posX, s32 posY, s16 font, u32 flags)
{
	u32 justificationMask;

	// draw a string
	DecalFont_DrawLine(string, posX, posY, font, flags);

	// draw the same string in a different place
	justificationMask = MM_HIGHSCORE_TEXT_JUSTIFICATION_MASK;
	DecalFont_DrawLine(string, posX + MM_HIGHSCORE_TEXT_SHADOW_X, posY + MM_HIGHSCORE_TEXT_SHADOW_Y, font, (flags & justificationMask) | BLACK);
}

#define MM_HIGHSCORE_COLOR(colorSlot, index) (((Color *)*(colorSlot))[index])

// NOTE(aalhendi): GCC 2.8.1 derives the retail stack and register layout from
// this C body, but its C-only output matches only 255 of 598 words. Source-local
// PSX blocks preserve the retail schedule; CTR_NATIVE executes the C behavior.
void MM_HighScore_Draw(s16 trackIndex, s32 rowIndex, s32 posX, s32 posY)
{
	register s16 trackIndexArg CTR_PSX_REGISTER("$4") = trackIndex;
	register s32 rowIndexArg CTR_PSX_REGISTER("$5") = rowIndex;
	register s32 posYArg CTR_PSX_REGISTER("$7") = posY;
	register s16 offsetY CTR_PSX_REGISTER("$22");
	struct MetaDataLEV *levelMetadata;
	register struct MainMenu_LevelRow *displayData CTR_PSX_REGISTER("$20");
	register u32 sharedS1 CTR_PSX_REGISTER("$17") = 0;
	register u32 sharedS2 CTR_PSX_REGISTER("$18");
	register u32 sharedS5 CTR_PSX_REGISTER("$21");
	struct TransitionMeta *sharedS8;
	s16 lineWidth;
	s16 bestLapLabelY;
	s32 trackOffsetWork;
	s32 zeroValue;
	s32 leftArrowY;
	s32 titleOffsetX;
	s32 titleOffsetY;
	register s16 scoreRowIndex CTR_PSX_REGISTER("$19");
	struct TransitionMeta *bestLapTransitions;
	register u16 scoreModeWork CTR_PSX_REGISTER("$10");
	u16 videoTransitionY;
	register u32 frameCounterPage CTR_PSX_REGISTER("$2");
	register u16 frameCounterWork CTR_PSX_REGISTER("$2");
	struct
	{
		RECT videoBox;
		u16 trackIndex;
		u8 padAfterTrackIndex[6];
		u16 scoreMode;
		u8 padAfterScoreMode[6];
		s32 rowIndex;
		s32 posX;
		s32 posY;
	} draw;
	register s16 offsetX CTR_PSX_REGISTER("$23");

	CTR_PSX_DEPEND_VALUE(rowIndexArg, sharedS1);
	draw.rowIndex = rowIndexArg;
	scoreModeWork = (u16)draw.rowIndex;
	CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(frameCounterPage, MM_FRAME_COUNTER_ASM_NAME, scoreModeWork);
	draw.posX = posX;
	offsetX = (s16)draw.posX;
#ifdef CTR_NATIVE
	CTR_PSX_DEPEND_VALUE(frameCounterPage, offsetX);
#else
	asm("lhu $2,-9872($2)\n\t"
	    "sw $22,112($sp)\n\t"
	    "sw $7,80($sp)\n\t"
	    "lhu $22,80($sp)\n\t"
	    "sw $31,124($sp)\n\t"
	    "sw $fp,120($sp)\n\t"
	    "sw $21,108($sp)\n\t"
	    "sw $20,104($sp)\n\t"
	    "sw $19,100($sp)\n\t"
	    "sw $18,96($sp)\n\t"
	    "sw $16,88($sp)\n\t"
	    "sh $4,56($sp)\n\t"
	    "andi $2,$2,0x0004\n\t"
	    ".set\tnoreorder\n\t"
	    ".set\tnomacro\n\t"
	    "bne $2,$0,$LPSXHighScoreAfterFlash\n\t"
	    "sh $10,64($sp)\n\t"
	    ".set\tmacro\n\t"
	    ".set\treorder\n\t"
	    "li $17,3\n\t"
	    "$LPSXHighScoreAfterFlash:\n\t"
	    ".if 0"
	    : "+r"(frameCounterPage)
	    : "r"(offsetX));
#endif
	frameCounterWork = CTR_PSX_PAGE_LVALUE(u16, frameCounterPage, MM_FRAME_COUNTER_PAGE_OFFSET, MM_FRAME_COUNTER);
	CTR_PSX_DEPEND_VALUE(posYArg, frameCounterWork);
	draw.posY = posYArg;
	offsetY = (s16)draw.posY;
	draw.trackIndex = trackIndex;
	draw.scoreMode = scoreModeWork;

	// get color data
	if ((frameCounterWork & MM_HIGHSCORE_FLASH_TIMER_BIT) == 0)
	{
		sharedS1 = RED;
	}

	{
		register struct MainMenu_LevelRow *trackTable CTR_PSX_REGISTER("$11");
		register struct MetaDataLEV *lineMetadata CTR_PSX_REGISTER("$12");

#ifdef CTR_NATIVE
		trackTable = MM_ARCADE_TRACKS;
		displayData = &trackTable[(s16)draw.trackIndex];
		lineMetadata = MM_LEVEL_METADATA;
#else
		asm(".endif\n\t"
		    "sll %0,%3,16\n\t"
		    "sra %0,%0,12\n\t"
		    "lui %1,%%hi(" MM_ARCADE_TRACKS_ASM_NAME ")\n\t"
		    "addiu %1,%1,%%lo(" MM_ARCADE_TRACKS_ASM_NAME ")\n\t"
		    "addu %0,%0,%1\n\t"
		    "lui %2,%%hi(" MM_LEVEL_METADATA_ASM_NAME ")\n\t"
		    "addiu %2,%2,%%lo(" MM_LEVEL_METADATA_ASM_NAME ")"
		    : "=r"(displayData), "=r"(trackTable), "=r"(lineMetadata)
		    : "r"(trackIndexArg));
#endif
		lineWidth = DecalFont_GetLineWidth(GAME_LANGUAGE_STRINGS[lineMetadata[displayData->levID].name_LNG], FONT_BIG);
	}

	// Draw arrow pointing Left
	{
		register u32 gameTrackerPage CTR_PSX_REGISTER("$10");
		register u32 workV1 CTR_PSX_REGISTER("$3");
		register u32 *orderingTable CTR_PSX_REGISTER("$4");
		register struct IconGroup *iconGroup CTR_PSX_REGISTER("$8");
		register struct DB *backBuffer CTR_PSX_REGISTER("$7");
		register u32 posYShift CTR_PSX_REGISTER("$12");

#ifdef CTR_NATIVE
		CTR_PSX_LOAD_SYMBOL_PAGE(gameTrackerPage, RETAIL_GAME_TRACKER_ASM_NAME);
#else
		asm("lui %0,%%hi(" RETAIL_GAME_TRACKER_ASM_NAME ")\n\t.if 0" : "=r"(gameTrackerPage));
#endif
		sharedS1 <<= 2;
		workV1 = (u32)CTR_PSX_PAGE_LVALUE(struct GameTracker *, gameTrackerPage, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER);
		lineWidth /= 2;
		orderingTable = ((struct GameTracker *)workV1)->pushBuffer_UI.ptrOT;
		iconGroup = ((struct GameTracker *)workV1)->iconGroup[MM_HIGHSCORE_ARROW_ICON_GROUP];
		backBuffer = ((struct GameTracker *)workV1)->backBuffer;
		CTR_PSX_LOAD_SYMBOL_PAGE(workV1, MM_COLOR_POINTERS_ASM_NAME);
		CTR_PSX_ADD_SYMBOL_LOW_IN_PLACE(workV1, MM_COLOR_POINTERS_ASM_NAME, (u32)MM_COLOR_POINTERS);
		sharedS1 += workV1;
		CTR_PSX_KEEP_VALUE(sharedS1);
		posYShift = *(volatile u32 *)&draw.posY;
		sharedS2 = posYShift << 0x10;
		sharedS2 = (s32)sharedS2 >> 0x10;
		leftArrowY = MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX].currY + sharedS2 + MM_HIGHSCORE_ARROW_Y_OFFSET;
		MM_DECALHUD_ARROW_2D((ICONGROUP_GETICONS(iconGroup))[MM_HIGHSCORE_ARROW_ICON_ID],
		                     ((scoreRowIndex = (s16)draw.posX) - lineWidth) + MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX].currX +
		                         MM_HIGHSCORE_ARROW_LEFT_X_OFFSET,
		                     leftArrowY, &backBuffer->primMem, orderingTable, MM_HIGHSCORE_COLOR((u32 **)sharedS1, 0), MM_HIGHSCORE_COLOR((u32 **)sharedS1, 1),
		                     MM_HIGHSCORE_COLOR((u32 **)sharedS1, 2), MM_HIGHSCORE_COLOR((u32 **)sharedS1, 3), 0, (sharedS5 = MM_HIGHSCORE_ARROW_SCALE),
		                     MM_HIGHSCORE_ARROW_LEFT_ROTATION);
#ifndef CTR_NATIVE
		asm volatile(".endif\n\t"
		             ".set\tnoreorder\n\t"
		             ".set\tnomacro\n\t"
		             "sll $17,$17,2\n\t"
		             "sll $2,$2,16\n\t"
		             "sra $16,$2,16\n\t"
		             "srl $2,$2,31\n\t"
		             "addu $16,$16,$2\n\t"
		             "lw $3," MM_GAME_TRACKER_PAGE_OFFSET_ASM "($10)\n\t"
		             "sra $16,$16,1\n\t"
		             "lw $4,5244($3)\n\t"
		             "lw $8,8484($3)\n\t"
		             "lw $7,16($3)\n\t"
		             "lui $3,%%hi(" MM_COLOR_POINTERS_ASM_NAME ")\n\t"
		             "addiu $3,$3,%%lo(" MM_COLOR_POINTERS_ASM_NAME ")\n\t"
		             "addu $17,$17,$3\n\t"
		             "sw $4,16($sp)\n\t"
		             "lw $3,0($17)\n\t"
		             "li $21,4096\n\t"
		             "lwl $11,3($3)\n\t"
		             "lwr $11,0($3)\n\t"
		             "nop\n\t"
		             "swl $11,23($sp)\n\t"
		             "swr $11,20($sp)\n\t"
		             "lw $3,0($17)\n\t"
		             "addu $7,$7,116\n\t"
		             "lwl $11,7($3)\n\t"
		             "lwr $11,4($3)\n\t"
		             "nop\n\t"
		             "swl $11,27($sp)\n\t"
		             "swr $11,24($sp)\n\t"
		             "lui $3,%%hi(" MM_HIGHSCORE_TRANSITIONS_ASM_NAME ")\n\t"
		             "addiu $fp,$3,%%lo(" MM_HIGHSCORE_TRANSITIONS_ASM_NAME ")\n\t"
		             "lw $11,76($sp)\n\t"
		             "lh $5,6($fp)\n\t"
		             "lw $2,0($17)\n\t"
		             "sll $19,$11,16\n\t"
		             "sra $19,$19,16\n\t"
		             "addu $5,$5,$19\n\t"
		             "subu $5,$5,$16\n\t"
		             "addu $5,$5,236\n\t"
		             "lwl $12,11($2)\n\t"
		             "lwr $12,8($2)\n\t"
		             "nop\n\t"
		             "swl $12,31($sp)\n\t"
		             "swr $12,28($sp)\n\t"
		             "lw $12,80($sp)\n\t"
		             "lh $6,8($fp)\n\t"
		             "lw $2,0($17)\n\t"
		             "sll $18,$12,16\n\t"
		             "sra $18,$18,16\n\t"
		             "addu $6,$6,$18\n\t"
		             "lwl $13,15($2)\n\t"
		             "lwr $13,12($2)\n\t"
		             "nop\n\t"
		             "swl $13,35($sp)\n\t"
		             "swr $13,32($sp)\n\t"
		             "li $2,2048\n\t"
		             "sw $0,36($sp)\n\t"
		             "sw $21,40($sp)\n\t"
		             "sw $2,44($sp)\n\t"
		             "lw $4,244($8)\n\t"
		             "jal DecalHUD_Arrow2D\n\t"
		             "addu $6,$6,21\n\t"
		             ".set\tmacro\n\t"
		             ".set\treorder\n\t"
		             ".if 0"
		             :
		             :
		             : "memory");
#endif
	}

	trackOffsetWork = MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX].currY + (s16)sharedS2;
	zeroValue = 0;

	// Draw arrow pointing Right
	MM_DECALHUD_ARROW_2D((ICONGROUP_GETICONS(GAME_TRACKER->iconGroup[MM_HIGHSCORE_ARROW_ICON_GROUP]))[MM_HIGHSCORE_ARROW_ICON_ID],
	                     MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX].currX + (lineWidth + scoreRowIndex) + MM_HIGHSCORE_ARROW_RIGHT_X_OFFSET,
	                     trackOffsetWork + MM_HIGHSCORE_ARROW_Y_OFFSET, &GAME_TRACKER->backBuffer->primMem, GAME_TRACKER->pushBuffer_UI.ptrOT,
	                     MM_HIGHSCORE_COLOR((u32 **)sharedS1, 0), MM_HIGHSCORE_COLOR((u32 **)sharedS1, 1), MM_HIGHSCORE_COLOR((u32 **)sharedS1, 2),
	                     MM_HIGHSCORE_COLOR((u32 **)sharedS1, 3), zeroValue, sharedS5, zeroValue);
#ifndef CTR_NATIVE
	asm volatile(".endif\n\t"
	             ".set\tnoreorder\n\t"
	             ".set\tnomacro\n\t"
	             "lui $13,%%hi(" RETAIL_GAME_TRACKER_ASM_NAME ")\n\t"
	             "lw $2,%%lo(" RETAIL_GAME_TRACKER_ASM_NAME ")($13)\n\t"
	             "nop\n\t"
	             "lw $3,5244($2)\n\t"
	             "lw $4,8484($2)\n\t"
	             "lw $7,16($2)\n\t"
	             "sw $3,16($sp)\n\t"
	             "lw $2,0($17)\n\t"
	             "addu $7,$7,116\n\t"
	             "lwl $10,3($2)\n\t"
	             "lwr $10,0($2)\n\t"
	             "nop\n\t"
	             "swl $10,23($sp)\n\t"
	             "swr $10,20($sp)\n\t"
	             "lh $2,6($fp)\n\t"
	             "lw $3,0($17)\n\t"
	             "addu $2,$2,$19\n\t"
	             "addu $16,$16,$2\n\t"
	             "lwl $10,7($3)\n\t"
	             "lwr $10,4($3)\n\t"
	             "nop\n\t"
	             "swl $10,27($sp)\n\t"
	             "swr $10,24($sp)\n\t"
	             "lh $6,8($fp)\n\t"
	             "lw $2,0($17)\n\t"
	             "addu $5,$16,274\n\t"
	             "lwl $10,11($2)\n\t"
	             "lwr $10,8($2)\n\t"
	             "nop\n\t"
	             "swl $10,31($sp)\n\t"
	             "swr $10,28($sp)\n\t"
	             "lw $2,0($17)\n\t"
	             "addu $6,$6,$18\n\t"
	             "lwl $10,15($2)\n\t"
	             "lwr $10,12($2)\n\t"
	             "nop\n\t"
	             "swl $10,35($sp)\n\t"
	             "swr $10,32($sp)\n\t"
	             "sw $0,36($sp)\n\t"
	             "sw $21,40($sp)\n\t"
	             "sw $0,44($sp)\n\t"
	             "lw $4,244($4)\n\t"
	             "jal DecalHUD_Arrow2D\n\t"
	             "addu $6,$6,21\n\t"
	             ".set\tmacro\n\t"
	             ".set\treorder\n\t"
	             ".if 0"
	             :
	             :
	             : "memory");
#endif

	// draw track name
	levelMetadata = MM_LEVEL_METADATA;
	titleOffsetX = draw.posX + MM_HIGHSCORE_TITLE_X_OFFSET;
	DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[levelMetadata[displayData->levID].name_LNG],
	                   MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX].currX + (s16)titleOffsetX,
	                   MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX].currY + (s16)(titleOffsetY = draw.posY + MM_HIGHSCORE_TITLE_Y_OFFSET), FONT_BIG,
	                   (s16)JUSTIFY_CENTER);
#ifndef CTR_NATIVE
	asm volatile(".endif\n\t"
	             ".set\tnoreorder\n\t"
	             ".set\tnomacro\n\t"
	             "li $7,1\n\t"
	             "lui $11,%%hi(" MM_LEVEL_METADATA_ASM_NAME ")\n\t"
	             "addiu $11,$11,%%lo(" MM_LEVEL_METADATA_ASM_NAME ")\n\t"
	             "lui $12,%%hi(" RETAIL_LANGUAGE_STRINGS_ASM_NAME ")\n\t"
	             "lhu $5,6($fp)\n\t"
	             "lw $10,76($sp)\n\t"
	             "lh $3,0($20)\n\t"
	             "lw $13,80($sp)\n\t"
	             "addu $5,$5,$10\n\t"
	             "addu $5,$5,256\n\t"
	             "sll $5,$5,16\n\t"
	             "sra $5,$5,16\n\t"
	             "sllv $2,$3,$7\n\t"
	             "addu $2,$2,$3\n\t"
	             "sll $2,$2,3\n\t"
	             "addu $2,$2,$11\n\t"
	             "lh $3,8($2)\n\t"
	             "li $2,-32768\n\t"
	             "sw $2,16($sp)\n\t"
	             "lw $2,%%lo(" RETAIL_LANGUAGE_STRINGS_ASM_NAME ")($12)\n\t"
	             "lhu $6,8($fp)\n\t"
	             "sll $3,$3,2\n\t"
	             "addu $3,$3,$2\n\t"
	             "addu $6,$6,$13\n\t"
	             "addu $6,$6,14\n\t"
	             "sll $6,$6,16\n\t"
	             "lw $4,0($3)\n\t"
	             "jal DecalFont_DrawLine\n\t"
	             "sra $6,$6,16\n\t"
	             ".set\tmacro\n\t"
	             ".set\treorder\n\t"
	             ".if 0"
	             :
	             :
	             : "memory");
#endif

	if (((u32)draw.rowIndex << 0x10) == MM_HIGHSCORE_SCORE_MODE_TIME_TRIAL)
	{
		s16 ghostStarIndex;
		register u32 ghostV0 CTR_PSX_REGISTER("$2");

		ghostStarIndex = 0;
		CTR_PSX_KEEP_VALUE_RELAXED(ghostStarIndex);
		CTR_PSX_LOAD_SYMBOL_PAGE(sharedS1, RETAIL_GAME_TRACKER_ASM_NAME);
		CTR_PSX_LOAD_SYMBOL_PAGE(ghostV0, RETAIL_GAME_SAVE_ASM_NAME);
		CTR_PSX_ADD_SYMBOL_LOW(sharedS5, ghostV0, RETAIL_GAME_SAVE_ASM_NAME, (u32)&GAME_PROGRESS);
		CTR_PSX_LOAD_SYMBOL_PAGE(ghostV0, MM_HIGHSCORE_GHOST_STAR_FLAGS_ASM_NAME);
		CTR_PSX_ADD_SYMBOL_LOW(displayData, ghostV0, MM_HIGHSCORE_GHOST_STAR_FLAGS_ASM_NAME, (void *)MM_HIGHSCORE_GHOST_STAR_FLAGS);
		ghostV0 = (u32)CTR_PSX_PAGE_LVALUE(struct GameTracker *, sharedS1, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER);
		sharedS2 = (u32)MM_HIGHSCORE_TRANSITIONS;
		scoreRowIndex = ((struct GameTracker *)ghostV0)->levelID;

		// draw ghost stars
		do
		{
			u32 timeTrialFlags;
			register struct GameTracker *setTracker CTR_PSX_REGISTER("$3");
			register struct GameTracker *drawTracker CTR_PSX_REGISTER("$6");
			register s32 signedGhostStarIndex CTR_PSX_REGISTER("$9");
			register u32 ghostStarIndexShift CTR_PSX_REGISTER("$2");
			register s32 doubledGhostStarIndex CTR_PSX_REGISTER("$7");
			register u16 currentGhostFlag CTR_PSX_REGISTER("$5");
			register s32 flagWordIndex CTR_PSX_REGISTER("$4");
			register u32 trackPage CTR_PSX_REGISTER("$12");

			CTR_PSX_LOAD_SYMBOL_PAGE(trackPage, MM_ARCADE_TRACKS_ASM_NAME);
			CTR_PSX_ADD_SYMBOL_LOW_IN_PLACE(trackPage, MM_ARCADE_TRACKS_ASM_NAME, (u32)MM_ARCADE_TRACKS);
			setTracker = CTR_PSX_PAGE_LVALUE(struct GameTracker *, sharedS1, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER);
			CTR_PSX_KEEP_VALUE_RELAXED(setTracker);
			setTracker->levelID = ((struct MainMenu_LevelRow *)trackPage)[(s16) * (volatile u16 *)&draw.trackIndex].levID;
			GAMEPROG_GetPtrHighScoreTrack();

			ghostStarIndexShift = (u32)ghostStarIndex << 0x10;
			signedGhostStarIndex = (s32)ghostStarIndexShift >> 0x10;
			CTR_PSX_KEEP_VALUE_RELAXED(signedGhostStarIndex);
			doubledGhostStarIndex = signedGhostStarIndex * sizeof(u16);
			CTR_PSX_KEEP_VALUE_RELAXED(doubledGhostStarIndex);
			currentGhostFlag = *(u16 *)((u8 *)displayData + doubledGhostStarIndex);
			CTR_PSX_KEEP_VALUE_RELAXED(currentGhostFlag);
			drawTracker = CTR_PSX_PAGE_LVALUE(struct GameTracker *, sharedS1, MM_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER);
			CTR_PSX_KEEP_VALUE_RELAXED(drawTracker);
			flagWordIndex = (((s16)currentGhostFlag) >> 2) >> 3;
			// NOTE(aalhendi): Retail derives the signed flag-word index in two shifts.
			timeTrialFlags = (&((struct GameProgress *)sharedS5)
			                       ->highScoreTracks[0]
			                       .timeTrialFlags)[drawTracker->levelID * (sizeof(struct HighScoreTrack) / sizeof(u32)) + flagWordIndex];
			if (((timeTrialFlags >> (currentGhostFlag & 0x1f)) & 1) != 0)
			{
				register s16 colorIndex CTR_PSX_REGISTER("$4");
				register u32 colorTablePage CTR_PSX_REGISTER("$2");
				register u32 **ghostColorPtr CTR_PSX_REGISTER("$4");
				register struct IconGroup *ghostIconGroup CTR_PSX_REGISTER("$8");
				register u32 *orderingTable CTR_PSX_REGISTER("$3");
				register struct DB *backBuffer CTR_PSX_REGISTER("$7");
				s32 ghostXBase;
				register s32 ghostX CTR_PSX_REGISTER("$5");
				register s32 ghostY CTR_PSX_REGISTER("$6");

				CTR_PSX_LOAD_SYMBOL_PAGE(colorTablePage, MM_HIGHSCORE_GHOST_STAR_COLORS_ASM_NAME);
				CTR_PSX_ADD_SYMBOL_LOW_IN_PLACE(colorTablePage, MM_HIGHSCORE_GHOST_STAR_COLORS_ASM_NAME, (u32)MM_HIGHSCORE_GHOST_STAR_COLORS);
				colorIndex = *(u16 *)((u8 *)colorTablePage + doubledGhostStarIndex);
				CTR_PSX_KEEP_VALUE_RELAXED(colorIndex);
				CTR_PSX_LOAD_SYMBOL_PAGE(colorTablePage, MM_COLOR_POINTERS_ASM_NAME);
				CTR_PSX_ADD_SYMBOL_LOW_IN_PLACE(colorTablePage, MM_COLOR_POINTERS_ASM_NAME, (u32)MM_COLOR_POINTERS);
				orderingTable = drawTracker->pushBuffer_UI.ptrOT;
				CTR_PSX_KEEP_VALUE_RELAXED(orderingTable);
				ghostIconGroup = drawTracker->iconGroup[MM_HIGHSCORE_GHOST_STAR_ICON_GROUP];
				CTR_PSX_KEEP_VALUE_RELAXED(ghostIconGroup);
				backBuffer = drawTracker->backBuffer;
				CTR_PSX_KEEP_VALUE_RELAXED(backBuffer);
				ghostColorPtr = (u32 **)(colorTablePage + (s16)colorIndex * sizeof(u32));
				CTR_PSX_DEPEND_VALUE(ghostColorPtr, orderingTable);
				CTR_PSX_ORDER_VALUES(ghostIconGroup, backBuffer);
				ghostX = signedGhostStarIndex * MM_HIGHSCORE_GHOST_STAR_X_STEP;
				ghostXBase = ((struct TransitionMeta *)sharedS2)[MM_HIGHSCORE_TITLE_META_INDEX].currX;
				ghostXBase += (s16)offsetX;
				ghostY = ((struct TransitionMeta *)sharedS2)[MM_HIGHSCORE_TITLE_META_INDEX].currY;
				ghostY += (s16)offsetY;
				ghostX += ghostXBase;
				ghostX += MM_HIGHSCORE_GHOST_STAR_X_OFFSET;

				MM_DECALHUD_DRAW_POLY_GT4((ICONGROUP_GETICONS(ghostIconGroup))[MM_HIGHSCORE_GHOST_STAR_ICON_ID], ghostX,
				                          ghostY + MM_HIGHSCORE_GHOST_STAR_Y_OFFSET, &backBuffer->primMem, orderingTable, MM_HIGHSCORE_COLOR(ghostColorPtr, 0),
				                          MM_HIGHSCORE_COLOR(ghostColorPtr, 1), MM_HIGHSCORE_COLOR(ghostColorPtr, 2), MM_HIGHSCORE_COLOR(ghostColorPtr, 3), 0,
				                          MM_HIGHSCORE_GHOST_STAR_SCALE);
			}

			ghostStarIndex++;
		} while (ghostStarIndex < MM_HIGHSCORE_GHOST_STAR_COUNT);

		GAME_TRACKER->levelID = scoreRowIndex;
		GAMEPROG_GetPtrHighScoreTrack();
	}

	{
		register s16 selectedScoreMode CTR_PSX_REGISTER("$16");

		sharedS1 = (u32)MM_HIGHSCORE_TRANSITIONS;
		selectedScoreMode = (s16)draw.scoreMode;
		sharedS5 = MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET;
		CTR_PSX_KEEP_VALUE(sharedS5);

		// first entry: Time Trial or Relic
		displayData = (struct MainMenu_LevelRow *)&GAME_PROGRESS.highScoreTracks[MM_ARCADE_TRACKS[(s16)draw.trackIndex].levID].scoreEntry[selectedScoreMode][0];
		CTR_PSX_KEEP_VALUE_RELAXED(displayData);

		MM_HighScore_Text3D(
		    GAME_LANGUAGE_STRINGS[LNG_BEST_TRACK_TIMES],
		    (s16)(((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_TRACK_META_INDEX].currX + offsetX + MM_HIGHSCORE_BEST_TRACK_LABEL_X_OFFSET),
		    (s16)(((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_TRACK_META_INDEX].currY + offsetY + MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET), FONT_SMALL,
		    zeroValue);

		scoreRowIndex = zeroValue;

		// if Time Trial
		// with ghost stars, and Best Lap
		if (selectedScoreMode == MM_HIGHSCORE_SCORE_MODE_TIME_TRIAL)
		{
			bestLapLabelY = ((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_LAP_LABEL_META_INDEX].currY + offsetY;
			MM_HighScore_Text3D(
			    GAME_LANGUAGE_STRINGS[LNG_BEST_LAP_TIME],
			    (s16)(((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_LAP_LABEL_META_INDEX].currX + offsetX + MM_HIGHSCORE_BEST_LAP_LABEL_X_OFFSET),
			    (s16)(bestLapLabelY + sharedS5), FONT_SMALL, 0);

			// Character Name
			MM_HighScore_Text3D(
			    ((struct HighScoreEntry *)displayData)[0].name,
			    (s16)(((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX].currX + offsetX + MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET),
			    (s16)(((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX].currY + offsetY + MM_HIGHSCORE_BEST_LAP_NAME_Y_OFFSET),
			    FONT_BIG, (s16)(((struct HighScoreEntry *)displayData)[0].characterID + MM_HIGHSCORE_DRIVER_COLOR_OFFSET));
			bestLapTransitions = (struct TransitionMeta *)sharedS1;

			// Draw time string
			// NOTE(aalhendi): Retail also uses currX as the Y transition base here.
			MM_HighScore_Text3D(
			    RECTMENU_DrawTime(((struct HighScoreEntry *)displayData)[0].time),
			    (s16)(bestLapTransitions[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX].currX + offsetX + MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET),
			    (s16)(((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX].currX + offsetY + MM_HIGHSCORE_BEST_LAP_TIME_Y_OFFSET),
			    FONT_SMALL, zeroValue);

			// Character Icon
			MM_RECTMENU_DRAW_POLY_GT4(GAME_TRACKER->ptrIcons[GAME_CHARACTER_METADATA[(s16)((struct HighScoreEntry *)displayData)[0].characterID].iconID],
			                          bestLapTransitions[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX].currX + (s16)offsetX + MM_HIGHSCORE_BEST_LAP_ICON_X_OFFSET,
			                          ((struct TransitionMeta *)sharedS1)[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX].currY + (s16)offsetY +
			                              MM_HIGHSCORE_BEST_LAP_ICON_Y_OFFSET,
			                          &GAME_TRACKER->backBuffer->primMem, (GAME_TRACKER->pushBuffer_UI).ptrOT, MM_HIGHSCORE_ICON_COLOR, MM_HIGHSCORE_ICON_COLOR,
			                          MM_HIGHSCORE_ICON_COLOR, MM_HIGHSCORE_ICON_COLOR, MM_HIGHSCORE_ICON_TRANSPARENCY, MM_HIGHSCORE_ICON_SCALE);
		}
	}

	// Draw five "best track times"
	// Icon, Name, and Time
	CTR_PSX_COPY_VALUE(sharedS8, (struct TransitionMeta *)sharedS1);
	CTR_PSX_KEEP_VALUE(sharedS8);
	for (scoreRowIndex = 0; scoreRowIndex <= MM_HIGHSCORE_VISIBLE_SCORE_ROWS - 1;)
	{
		struct HighScoreEntry *rowEntry;
		s32 currentRowIndex = (s16)scoreRowIndex;
		s32 metaIndex;
		metaIndex = currentRowIndex + MM_HIGHSCORE_FIRST_VISIBLE_META_INDEX;
		sharedS1 = (u32)&sharedS8[metaIndex];
		sharedS2 = currentRowIndex * MM_HIGHSCORE_SCORE_ROW_Y_STEP;
		rowEntry = (struct HighScoreEntry *)displayData;
		rowEntry += currentRowIndex + MM_HIGHSCORE_FIRST_VISIBLE_ENTRY;
		CTR_PSX_KEEP_VALUE(sharedS1);

		// Character Icon
		MM_RECTMENU_DRAW_POLY_GT4(GAME_TRACKER->ptrIcons[GAME_CHARACTER_METADATA[(s16)rowEntry->characterID].iconID],
		                          ((struct TransitionMeta *)sharedS1)[0].currX + (s16)offsetX + MM_HIGHSCORE_SCORE_ICON_X_OFFSET,
		                          ((struct TransitionMeta *)sharedS1)[0].currY + (s16)offsetY + sharedS5 + (s16)sharedS2 +
		                              (MM_HIGHSCORE_SCORE_NAME_Y_OFFSET - MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET),
		                          &GAME_TRACKER->backBuffer->primMem, GAME_TRACKER->pushBuffer_UI.ptrOT, MM_HIGHSCORE_ICON_COLOR, MM_HIGHSCORE_ICON_COLOR,
		                          MM_HIGHSCORE_ICON_COLOR, MM_HIGHSCORE_ICON_COLOR, MM_HIGHSCORE_ICON_TRANSPARENCY, MM_HIGHSCORE_ICON_SCALE);

		// draw the name string
		{
			s32 nameRowYOffset = (s32)sharedS2 + (MM_HIGHSCORE_SCORE_NAME_Y_OFFSET - MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET);

			MM_HighScore_Text3D(rowEntry->name, (s16)(((struct TransitionMeta *)sharedS1)[0].currX + offsetX + MM_HIGHSCORE_SCORE_NAME_X_OFFSET),
			                    (s16)(((struct TransitionMeta *)sharedS1)[0].currY + offsetY + sharedS5 + nameRowYOffset), FONT_BIG,
			                    (s16)(rowEntry->characterID + MM_HIGHSCORE_DRIVER_COLOR_OFFSET));
		}

		// draw the Time string
		{
			char *timeString = RECTMENU_DrawTime(rowEntry->time);

			sharedS2 = sharedS2 + MM_HIGHSCORE_SCORE_ROW_Y_STEP;
			CTR_PSX_KEEP_VALUE(sharedS2);
			MM_HighScore_Text3D(timeString, (s16)(((struct TransitionMeta *)sharedS1)[0].currX + offsetX + MM_HIGHSCORE_SCORE_NAME_X_OFFSET),
			                    (s16)(((struct TransitionMeta *)sharedS1)[0].currY + offsetY + sharedS5 + sharedS2), FONT_SMALL, zeroValue);
		}

		{
			register s16 nextScoreRowIndex CTR_PSX_REGISTER("$2");
			nextScoreRowIndex = scoreRowIndex + 1;
			scoreRowIndex = nextScoreRowIndex;
		}
	}

	draw.videoBox.w = MM_HIGHSCORE_VIDEO_BOX_W;
	draw.videoBox.h = MM_HIGHSCORE_VIDEO_BOX_H;
	draw.videoBox.x = (u16)MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_VIDEO_META_INDEX].currX + (u16)offsetX + MM_HIGHSCORE_VIDEO_BOX_X_OFFSET;
	videoTransitionY = (u16)MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_VIDEO_META_INDEX].currY;
	draw.videoBox.y = videoTransitionY + (u16)offsetY + MM_HIGHSCORE_VIDEO_BOX_Y_OFFSET;

	MM_TrackSelect_Video_Draw(&draw.videoBox, &MM_ARCADE_TRACKS[0], (s16) * (volatile u16 *)&draw.trackIndex, (MM_HIGHSCORE_TRANSITION_STATE == EXITING_MENU),
	                          0);
#ifndef CTR_NATIVE
	// NOTE(aalhendi): The C above remains the native implementation. On PSX,
	// this block replaces GCC's divergent tail with the retail sequence.
	asm volatile(".endif\n"
	             "\tlw\t$10,72($sp)\n"
	             "\tnop\n"
	             "\tsll\t$2,$10,16\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tbne\t$2,$0,$LMM_HighScore_Draw_RestoreTrack\n"
	             "\tlui\t$3,%%hi(" MM_ARCADE_TRACKS_ASM_NAME ")\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tmove\t$16,$0\n"
	             " #APP\n"
	             "\tlui $17,%%hi(" RETAIL_GAME_TRACKER_ASM_NAME ")\n"
	             "\tlui $2,%%hi(" RETAIL_GAME_SAVE_ASM_NAME ")\n"
	             "\taddiu $21,$2,%%lo(" RETAIL_GAME_SAVE_ASM_NAME ")\n"
	             "\tlui $2,%%hi(" MM_HIGHSCORE_GHOST_STAR_FLAGS_ASM_NAME ")\n"
	             "\taddiu $20,$2,%%lo(" MM_HIGHSCORE_GHOST_STAR_FLAGS_ASM_NAME ")\n"
	             " #NO_APP\n"
	             "\tlw\t$2," MM_GAME_TRACKER_PAGE_OFFSET_ASM "($17)\n"
	             "\tmove\t$18,$fp\n"
	             "\tlhu\t$19,6672($2)\n"
	             "\tlui\t$12,%%hi(" MM_ARCADE_TRACKS_ASM_NAME ")\n"
	             "$LMM_HighScore_Draw_GhostStarLoop:\n"
	             "\taddiu\t$12,$12,%%lo(" MM_ARCADE_TRACKS_ASM_NAME ")\n"
	             "\tlhu\t$11,56($sp)\n"
	             "\tlw\t$3," MM_GAME_TRACKER_PAGE_OFFSET_ASM "($17)\n"
	             "\tsll\t$2,$11,16\n"
	             "\tsra\t$2,$2,12\n"
	             "\taddu\t$2,$2,$12\n"
	             "\tlh\t$2,0($2)\n"
	             " #APP\n"
	             " #NO_APP\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tGAMEPROG_GetPtrHighScoreTrack\n"
	             "\tsw\t$2,6672($3)\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tsll\t$2,$16,16\n"
	             "\tsra\t$9,$2,16\n"
	             " #APP\n"
	             " #NO_APP\n"
	             "\tsll\t$7,$9,1\n"
	             " #APP\n"
	             " #NO_APP\n"
	             "\taddu\t$2,$7,$20\n"
	             "\tlhu\t$5,0($2)\n"
	             " #APP\n"
	             " #NO_APP\n"
	             "\tlw\t$6," MM_GAME_TRACKER_PAGE_OFFSET_ASM "($17)\n"
	             " #APP\n"
	             " #NO_APP\n"
	             "\tsll\t$4,$5,16\n"
	             "\tlw\t$3,6672($6)\n"
	             "\tsra\t$4,$4,21\n"
	             "\tsll\t$2,$3,3\n"
	             "\taddu\t$2,$2,$3\n"
	             "\tsll\t$2,$2,3\n"
	             "\taddu\t$2,$2,$3\n"
	             "\taddu\t$2,$2,$4\n"
	             "\tsll\t$2,$2,2\n"
	             "\taddu\t$2,$2,$21\n"
	             "\tlw\t$2,300($2)\n"
	             "\tandi\t$5,$5,0x001f\n"
	             "\tsrl\t$2,$2,$5\n"
	             "\tandi\t$2,$2,0x0001\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tbeq\t$2,$0,$LMM_HighScore_Draw_NextGhostStar\n"
	             "\taddu\t$2,$16,1\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tlui\t$2,%%hi(" MM_HIGHSCORE_GHOST_STAR_COLORS_ASM_NAME ")\n"
	             "\taddiu\t$2,$2,%%lo(" MM_HIGHSCORE_GHOST_STAR_COLORS_ASM_NAME ")\n"
	             "\taddu\t$2,$7,$2\n"
	             "\tlhu\t$4,0($2)\n"
	             "\tlui\t$2,%%hi(" MM_COLOR_POINTERS_ASM_NAME ")\n"
	             "\taddiu\t$2,$2,%%lo(" MM_COLOR_POINTERS_ASM_NAME ")\n"
	             "\tlw\t$3,5244($6)\n"
	             "\tlw\t$8,8488($6)\n"
	             "\tlw\t$7,16($6)\n"
	             "\tsll\t$4,$4,16\n"
	             "\tsra\t$4,$4,14\n"
	             "\taddu\t$4,$4,$2\n"
	             "\tsw\t$3,16($sp)\n"
	             "\tlw\t$2,0($4)\n"
	             "\tsll\t$5,$9,4\n"
	             "\tlwl\t$13,3($2)\n"
	             "\tlwr\t$13,0($2)\n"
	             "\tnop\n"
	             "\tswl\t$13,23($sp)\n"
	             "\tswr\t$13,20($sp)\n"
	             "\tlw\t$2,0($4)\n"
	             "\taddu\t$5,$5,240\n"
	             "\tlwl\t$13,7($2)\n"
	             "\tlwr\t$13,4($2)\n"
	             "\tnop\n"
	             "\tswl\t$13,27($sp)\n"
	             "\tswr\t$13,24($sp)\n"
	             "\tsll\t$2,$23,16\n"
	             "\tlh\t$3,6($18)\n"
	             "\tsra\t$2,$2,16\n"
	             "\taddu\t$3,$3,$2\n"
	             "\tlw\t$2,0($4)\n"
	             "\taddu\t$7,$7,116\n"
	             "\tlwl\t$13,11($2)\n"
	             "\tlwr\t$13,8($2)\n"
	             "\tnop\n"
	             "\tswl\t$13,31($sp)\n"
	             "\tswr\t$13,28($sp)\n"
	             "\tsll\t$2,$22,16\n"
	             "\tlh\t$6,8($18)\n"
	             "\tsra\t$2,$2,16\n"
	             "\taddu\t$6,$6,$2\n"
	             "\tlw\t$2,0($4)\n"
	             "\taddu\t$5,$3,$5\n"
	             "\tlwl\t$13,15($2)\n"
	             "\tlwr\t$13,12($2)\n"
	             "\tnop\n"
	             "\tswl\t$13,35($sp)\n"
	             "\tswr\t$13,32($sp)\n"
	             "\tli\t$2,4096\t\t\t# 0x00001000\n"
	             "\tsw\t$0,36($sp)\n"
	             "\tsw\t$2,40($sp)\n"
	             "\tlw\t$4,240($8)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tDecalHUD_DrawPolyGT4\n"
	             "\taddu\t$6,$6,4\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\taddu\t$2,$16,1\n"
	             "$LMM_HighScore_Draw_NextGhostStar:\n"
	             "\tmove\t$16,$2\n"
	             "\tsll\t$2,$2,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\tslt\t$2,$2,2\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tbne\t$2,$0,$LMM_HighScore_Draw_GhostStarLoop\n"
	             "\tlui\t$12,%%hi(" MM_ARCADE_TRACKS_ASM_NAME ")\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tlui\t$2,%%hi(" RETAIL_GAME_TRACKER_ASM_NAME ")\n"
	             "\tsll\t$3,$19,16\n"
	             "\tlw\t$2,%%lo(" RETAIL_GAME_TRACKER_ASM_NAME ")($2)\n"
	             "\tsra\t$3,$3,16\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tGAMEPROG_GetPtrHighScoreTrack\n"
	             "\tsw\t$3,6672($2)\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tlui\t$3,%%hi(" MM_ARCADE_TRACKS_ASM_NAME ")\n"
	             "$LMM_HighScore_Draw_RestoreTrack:\n"
	             "\taddiu\t$3,$3,%%lo(" MM_ARCADE_TRACKS_ASM_NAME ")\n"
	             "\tli\t$7,2\t\t\t# 0x00000002\n"
	             "\tlui\t$18,%%hi(" RETAIL_LANGUAGE_STRINGS_ASM_NAME ")\n"
	             "\tli\t$21,43\t\t\t# 0x0000002b\n"
	             "\tlhu\t$13,56($sp)\n"
	             "\tlhu\t$10,64($sp)\n"
	             "\tsll\t$2,$13,16\n"
	             "\tsra\t$2,$2,12\n"
	             "\taddu\t$2,$2,$3\n"
	             "\tlui\t$3,%%hi(" MM_HIGHSCORE_TRANSITIONS_ASM_NAME ")\n"
	             "\taddiu\t$17,$3,%%lo(" MM_HIGHSCORE_TRANSITIONS_ASM_NAME ")\n"
	             "\tsll\t$16,$10,16\n"
	             "\tlh\t$3,0($2)\n"
	             "\tlhu\t$5,16($17)\n"
	             "\tsra\t$16,$16,16\n"
	             "\tsw\t$0,16($sp)\n"
	             "\tlhu\t$6,18($17)\n"
	             "\tlw\t$2,%%lo(" RETAIL_LANGUAGE_STRINGS_ASM_NAME ")($18)\n"
	             "\taddu\t$5,$5,$23\n"
	             "\taddiu\t$5,$5,32\n"
	             "\tsll\t$5,$5,16\n"
	             "\tsra\t$5,$5,16\n"
	             "\taddu\t$6,$6,$22\n"
	             "\taddiu\t$6,$6,43\n"
	             "\tsll\t$6,$6,16\n"
	             "\tsra\t$6,$6,16\n"
	             "\tsll\t$8,$3,3\n"
	             "\taddu\t$8,$8,$3\n"
	             "\tsll\t$8,$8,3\n"
	             "\taddu\t$8,$8,$3\n"
	             "\tsll\t$8,$8,$7\n"
	             "\tlw\t$4,716($2)\n"
	             "\tsll\t$2,$16,3\n"
	             "\taddu\t$2,$2,$16\n"
	             "\tsll\t$2,$2,4\n"
	             "\tlui\t$3,%%hi(" RETAIL_GAME_SAVE_ASM_NAME "+12)\n"
	             "\taddiu\t$3,$3,%%lo(" RETAIL_GAME_SAVE_ASM_NAME "+12)\n"
	             "\taddu\t$2,$2,$3\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_HighScore_Text3D\n"
	             "\taddu\t$20,$8,$2\n"
	             "\tbne\t$16,$0,$L244\n"
	             "\tmove\t$19,$0\n"
	             "\tlhu\t$5,76($17)\n"
	             "\tlw\t$2,%%lo(" RETAIL_LANGUAGE_STRINGS_ASM_NAME ")($18)\n"
	             "\tli\t$7,2\t\t\t# 0x00000002\n"
	             "\tsw\t$0,16($sp)\n"
	             "\tlhu\t$6,78($17)\n"
	             "\taddu\t$5,$5,$23\n"
	             "\taddu\t$5,$5,292\n"
	             "\tsll\t$5,$5,16\n"
	             "\tsra\t$5,$5,16\n"
	             "\taddu\t$6,$6,$22\n"
	             "\taddu\t$6,$6,$21\n"
	             "\tsll\t$6,$6,16\n"
	             "\tlw\t$4,720($2)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_HighScore_Text3D\n"
	             "\tsra\t$6,$6,16\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\taddu\t$4,$20,4\n"
	             "\tli\t$7,1\t\t\t# 0x00000001\n"
	             "\tlhu\t$5,86($17)\n"
	             "\tlhu\t$6,88($17)\n"
	             "\tlhu\t$2,22($20)\n"
	             "\taddu\t$5,$5,$23\n"
	             "\taddu\t$5,$5,352\n"
	             "\tsll\t$5,$5,16\n"
	             "\tsra\t$5,$5,16\n"
	             "\taddu\t$6,$6,$22\n"
	             "\taddu\t$6,$6,57\n"
	             "\tsll\t$6,$6,16\n"
	             "\tsra\t$6,$6,16\n"
	             "\taddu\t$2,$2,5\n"
	             "\tsll\t$2,$2,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_HighScore_Text3D\n"
	             "\tsw\t$2,16($sp)\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tlw\t$4,0($20)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tRECTMENU_DrawTime\n"
	             "\tmove\t$19,$0\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tmove\t$4,$2\n"
	             "\tlhu\t$6,86($17)\n"
	             "\tli\t$7,2\t\t\t# 0x00000002\n"
	             "\tsw\t$0,16($sp)\n"
	             "\taddu\t$5,$6,$23\n"
	             "\taddu\t$5,$5,352\n"
	             "\tsll\t$5,$5,16\n"
	             "\tsra\t$5,$5,16\n"
	             "\taddu\t$6,$6,$22\n"
	             "\taddu\t$6,$6,74\n"
	             "\tsll\t$6,$6,16\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_HighScore_Text3D\n"
	             "\tsra\t$6,$6,16\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tsll\t$2,$23,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\tlui\t$4,%%hi(" RETAIL_GAME_TRACKER_ASM_NAME ") # high\n"
	             "\tlh\t$5,86($17)\n"
	             "\tlh\t$3,22($20)\n"
	             "\tlh\t$6,88($17)\n"
	             "\tlw\t$4,%%lo(" RETAIL_GAME_TRACKER_ASM_NAME ")($4)\n"
	             "\taddu\t$5,$5,$2\n"
	             "\taddu\t$5,$5,292\n"
	             "\tlui\t$2,%%hi(" RETAIL_CHARACTER_METADATA_ASM_NAME ") # high\n"
	             "\taddiu\t$2,$2,%%lo(" RETAIL_CHARACTER_METADATA_ASM_NAME ") # low\n"
	             "\tsll\t$3,$3,4\n"
	             "\taddu\t$3,$3,$2\n"
	             "\tsll\t$2,$22,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\taddu\t$6,$6,$2\n"
	             "\tlh\t$3,8($3)\n"
	             "\tlw\t$7,16($4)\n"
	             "\tlw\t$2,5244($4)\n"
	             "\taddu\t$6,$6,56\n"
	             "\tsw\t$2,16($sp)\n"
	             "\tlui\t$2,%%hi(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ") # high\n"
	             "\taddiu\t$10,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$11,3($10)\n"
	             "\tlwr\t$11,0($10)\n"
	             "\tswl\t$11,23($sp)\n"
	             "\tswr\t$11,20($sp)\n"
	             "\taddiu\t$10,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$11,3($10)\n"
	             "\tlwr\t$11,0($10)\n"
	             "\tswl\t$11,27($sp)\n"
	             "\tswr\t$11,24($sp)\n"
	             "\taddiu\t$10,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$11,3($10)\n"
	             "\tlwr\t$11,0($10)\n"
	             "\tswl\t$11,31($sp)\n"
	             "\tswr\t$11,28($sp)\n"
	             "\taddiu\t$10,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$11,3($10)\n"
	             "\tlwr\t$11,0($10)\n"
	             "\tswl\t$11,35($sp)\n"
	             "\tswr\t$11,32($sp)\n"
	             "\tli\t$2,1\t\t\t# 0x00000001\n"
	             "\tsw\t$2,36($sp)\n"
	             "\tli\t$2,4096\t\t\t# 0x00001000\n"
	             "\tsll\t$3,$3,2\n"
	             "\taddu\t$4,$4,$3\n"
	             "\tsw\t$2,40($sp)\n"
	             "\tlw\t$4,7916($4)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tRECTMENU_DrawPolyGT4\n"
	             "\taddu\t$7,$7,116\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "$L244:\n"
	             " #APP\n"
	             "\tmove $fp,$17\n"
	             " #NO_APP\n"
	             "\tsll\t$3,$19,16\n"
	             "$L252:\n"
	             "\tsra\t$3,$3,16\n"
	             "\taddu\t$2,$3,2\n"
	             "\tsll\t$17,$2,2\n"
	             "\taddu\t$17,$17,$2\n"
	             "\tsll\t$17,$17,1\n"
	             "\taddu\t$17,$17,$fp\n"
	             "\tsll\t$2,$23,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\tsll\t$16,$3,1\n"
	             "\taddu\t$16,$16,$3\n"
	             "\tsll\t$16,$16,3\n"
	             "\taddiu\t$16,$16,24\n"
	             "\taddu\t$16,$20,$16\n"
	             "\tsll\t$18,$3,5\n"
	             "\tsubu\t$18,$18,$3\n"
	             "\tlui\t$4,%%hi(" RETAIL_GAME_TRACKER_ASM_NAME ") # high\n"
	             "\tlui\t$11,%%hi(" RETAIL_CHARACTER_METADATA_ASM_NAME ") # high\n"
	             "\taddiu\t$11,$11,%%lo(" RETAIL_CHARACTER_METADATA_ASM_NAME ") # low\n"
	             "\tlh\t$5,6($17)\n"
	             "\tlh\t$3,22($16)\n"
	             "\tlh\t$6,8($17)\n"
	             "\tlw\t$4,%%lo(" RETAIL_GAME_TRACKER_ASM_NAME ")($4)\n"
	             "\taddu\t$5,$5,$2\n"
	             "\taddu\t$5,$5,32\n"
	             "\tsll\t$3,$3,4\n"
	             "\taddu\t$3,$3,$11\n"
	             "\tsll\t$2,$22,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\taddu\t$6,$6,$2\n"
	             "\taddu\t$6,$6,$21\n"
	             "\tsll\t$2,$18,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\taddu\t$6,$6,$2\n"
	             "\tlh\t$3,8($3)\n"
	             "\tlw\t$7,16($4)\n"
	             "\tlw\t$2,5244($4)\n"
	             "\taddu\t$6,$6,14\n"
	             "\tsw\t$2,16($sp)\n"
	             "\tlui\t$2,%%hi(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ") # high\n"
	             "\taddiu\t$11,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$12,3($11)\n"
	             "\tlwr\t$12,0($11)\n"
	             "\tswl\t$12,23($sp)\n"
	             "\tswr\t$12,20($sp)\n"
	             "\taddiu\t$11,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$12,3($11)\n"
	             "\tlwr\t$12,0($11)\n"
	             "\tswl\t$12,27($sp)\n"
	             "\tswr\t$12,24($sp)\n"
	             "\taddiu\t$11,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$12,3($11)\n"
	             "\tlwr\t$12,0($11)\n"
	             "\tswl\t$12,31($sp)\n"
	             "\tswr\t$12,28($sp)\n"
	             "\taddiu\t$11,$2,%%lo(" MM_HIGHSCORE_ICON_COLOR_ASM_NAME ")\n"
	             "\tlwl\t$12,3($11)\n"
	             "\tlwr\t$12,0($11)\n"
	             "\tswl\t$12,35($sp)\n"
	             "\tswr\t$12,32($sp)\n"
	             "\tli\t$2,1\t\t\t# 0x00000001\n"
	             "\tsw\t$2,36($sp)\n"
	             "\tli\t$2,4096\t\t\t# 0x00001000\n"
	             "\tsll\t$3,$3,2\n"
	             "\taddu\t$4,$4,$3\n"
	             "\tsw\t$2,40($sp)\n"
	             "\tlw\t$4,7916($4)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tRECTMENU_DrawPolyGT4\n"
	             "\taddu\t$7,$7,116\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\taddiu\t$4,$16,4\n"
	             "\tli\t$7,1\t\t\t# 0x00000001\n"
	             "\taddiu\t$2,$18,14\n"
	             "\tlhu\t$5,6($17)\n"
	             "\tlhu\t$6,8($17)\n"
	             "\taddu\t$5,$5,$23\n"
	             "\taddiu\t$5,$5,92\n"
	             "\tsll\t$5,$5,16\n"
	             "\tsra\t$5,$5,16\n"
	             "\taddu\t$6,$6,$22\n"
	             "\taddu\t$6,$6,$21\n"
	             "\taddu\t$6,$6,$2\n"
	             "\tsll\t$6,$6,16\n"
	             "\tlhu\t$2,22($16)\n"
	             "\tsra\t$6,$6,16\n"
	             "\taddu\t$2,$2,5\n"
	             "\tsll\t$2,$2,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_HighScore_Text3D\n"
	             "\tsw\t$2,16($sp)\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\tlw\t$4,0($16)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tRECTMENU_DrawTime\n"
	             "\taddu\t$18,$18,31\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             " #APP\n"
	             " #NO_APP\n"
	             "\tmove\t$4,$2\n"
	             "\tli\t$7,2\t\t\t# 0x00000002\n"
	             "\tlhu\t$5,6($17)\n"
	             "\tlhu\t$6,8($17)\n"
	             "\tsw\t$0,16($sp)\n"
	             "\taddu\t$5,$5,$23\n"
	             "\taddiu\t$5,$5,92\n"
	             "\tsll\t$5,$5,16\n"
	             "\tsra\t$5,$5,16\n"
	             "\taddu\t$6,$6,$22\n"
	             "\taddu\t$6,$6,$21\n"
	             "\taddu\t$6,$6,$18\n"
	             "\tsll\t$6,$6,16\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_HighScore_Text3D\n"
	             "\tsra\t$6,$6,16\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\taddu\t$2,$19,1\n"
	             "\tmove\t$19,$2\n"
	             "\tsll\t$2,$2,16\n"
	             "\tsra\t$2,$2,16\n"
	             "\tslt\t$2,$2,5\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tbne\t$2,$0,$L252\n"
	             "\tsll\t$3,$19,16\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             "\n"
	             "\taddu\t$4,$sp,48\n"
	             "\tlui\t$5,%%hi(" MM_ARCADE_TRACKS_ASM_NAME ") # high\n"
	             "\taddiu\t$5,$5,%%lo(" MM_ARCADE_TRACKS_ASM_NAME ") # low\n"
	             "\tli\t$2,176\t\t\t# 0x000000b0\n"
	             "\tsh\t$2,52($sp)\n"
	             "\tli\t$2,75\t\t\t# 0x0000004b\n"
	             "\tlui\t$3,%%hi(" MM_HIGHSCORE_TRANSITIONS_ASM_NAME ") # high\n"
	             "\tsh\t$2,54($sp)\n"
	             "\tlui\t$2,%%hi(" MM_HIGHSCORE_TRANSITION_STATE_ASM_NAME ") # high\n"
	             "\taddiu\t$3,$3,%%lo(" MM_HIGHSCORE_TRANSITIONS_ASM_NAME ") # low\n"
	             "\tlhu\t$12,56($sp)\n"
	             "\tlh\t$7,%%lo(" MM_HIGHSCORE_TRANSITION_STATE_ASM_NAME ")($2)\n"
	             "\tlhu\t$2,96($3)\n"
	             "\tlhu\t$3,98($3)\n"
	             "\tsll\t$6,$12,16\n"
	             "\tsra\t$6,$6,16\n"
	             "\txori\t$7,$7,0x0002\n"
	             "\tsltu\t$7,$7,1\n"
	             "\taddu\t$2,$2,$23\n"
	             "\taddu\t$2,$2,292\n"
	             "\taddu\t$3,$3,$22\n"
	             "\taddu\t$3,$3,90\n"
	             "\tsh\t$2,48($sp)\n"
	             "\tsh\t$3,50($sp)\n"
	             "\t.set\tnoreorder\n"
	             "\t.set\tnomacro\n"
	             "\tjal\tMM_TrackSelect_Video_Draw\n"
	             "\tsw\t$0,16($sp)\n"
	             "\t.set\tmacro\n"
	             "\t.set\treorder\n"
	             :
	             :
	             : "memory");
#endif
}

#undef MM_HIGHSCORE_COLOR

void MM_HighScore_Init(void)
{
	MM_HIGHSCORE_TRANSITION_STATE = ENTERING_MENU;
	MM_HIGHSCORE_TRANSITION_MAIN_FRAME = MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME;
	MM_HIGHSCORE_TARGET_ROW = 0;
	MM_HIGHSCORE_CURRENT_ROW = 0;

	// reset all video variables
	MM_TrackSelect_Video_SetDefaults();
}

void MM_HighScore_MenuProc(struct RectMenu *menu_unused)
{
	b32 videoResetRequested;
	s32 currOffsetY;
	s32 currOffsetX;
	s32 nextOffsetX;
	s32 nextOffsetY;
	register s32 compareOffsetX CTR_PSX_REGISTER("$7");
	register s32 compareOffsetY CTR_PSX_REGISTER("$5");
	register s32 currentTransitionTrackFrame CTR_PSX_REGISTER("$4");
	register s32 sharedV0 CTR_PSX_REGISTER("$2");
	register s32 sharedV1 CTR_PSX_REGISTER("$3");
	register const struct TransitionMeta *titleMeta CTR_PSX_REGISTER("$4");
	register const s16 *activeVerticalMoveAddress CTR_PSX_REGISTER("$2");
	RECT wipeRect;
	s16 pendingVerticalMove;

	(void)menu_unused;

	videoResetRequested = false;
	if (MM_HIGHSCORE_TRANSITION_STATE == IN_MENU)
	{
		goto LAB_HIGHSCORE_PROCESS_INPUT;
	}
	if (MM_HIGHSCORE_TRANSITION_STATE < EXITING_MENU)
	{
		if (MM_HIGHSCORE_TRANSITION_STATE == ENTERING_MENU)
		{
			goto LAB_HIGHSCORE_ENTERING;
		}
		goto LAB_HIGHSCORE_TRANSITION_DONE;
	}
	if (MM_HIGHSCORE_TRANSITION_STATE == EXITING_MENU)
	{
		goto LAB_HIGHSCORE_EXITING;
	}
	goto LAB_HIGHSCORE_TRANSITION_DONE;

LAB_HIGHSCORE_ENTERING:
	MM_TransitionInOut(MM_HIGHSCORE_TRANSITIONS, MM_HIGHSCORE_TRANSITION_MAIN_FRAME, MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES);
	if (MM_HIGHSCORE_TRANSITION_MAIN_FRAME != 0)
	{
		MM_HIGHSCORE_TRANSITION_MAIN_FRAME--;
		goto LAB_HIGHSCORE_TRANSITION_DONE;
	}
	MM_HIGHSCORE_TRANSITION_STATE = IN_MENU;
	goto LAB_HIGHSCORE_TRANSITION_DONE;

LAB_HIGHSCORE_EXITING:
	if ((MM_HIGHSCORE_TRANSITION_TRACK_FRAME == 0) && (MM_HIGHSCORE_TRANSITION_ROW_FRAME == 0))
	{
		MM_TransitionInOut(MM_HIGHSCORE_TRANSITIONS, MM_HIGHSCORE_TRANSITION_MAIN_FRAME, MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES);
		MM_HIGHSCORE_TRANSITION_MAIN_FRAME++;
		if (MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME < MM_HIGHSCORE_TRANSITION_MAIN_FRAME)
		{
			MM_JumpTo_Title_Returning();
			return;
		}
	}

LAB_HIGHSCORE_TRANSITION_DONE:
	if (MM_HIGHSCORE_TRANSITION_STATE != IN_MENU)
	{
		goto LAB_OVR_230__800b3c78;
	}

LAB_HIGHSCORE_PROCESS_INPUT:
	if ((MM_GAME_BUTTON_TAPS[0] & BTN_UP) != 0)
	{
		if (MM_HIGHSCORE_MENU.rowSelected == 1)
		{
			videoResetRequested = true;
		}
	}
	else if (((MM_GAME_BUTTON_TAPS[0] & BTN_DOWN) != 0) && (MM_HIGHSCORE_MENU.rowSelected < 1))
	{
		videoResetRequested = true;
	}

	if ((MM_GAME_BUTTON_TAPS[0] & (BTN_SQUARE_one | BTN_TRIANGLE)) != 0)
	{
		videoResetRequested = true;
		OtherFX_Play(2, 1);
		MM_HIGHSCORE_TRANSITION_STATE = EXITING_MENU;
	}
	else if ((MM_GAME_BUTTON_TAPS[0] & BTN_LEFT) != 0)
	{
		b32 trackOpen;
		videoResetRequested = true;
		MM_HIGHSCORE_PENDING_HORIZONTAL_MOVE = -1;

		do
		{
			MM_HIGHSCORE_TARGET_TRACK = MM_HIGHSCORE_TARGET_TRACK - 1;
			if (MM_HIGHSCORE_TARGET_TRACK < 0)
			{
				MM_HIGHSCORE_TARGET_TRACK = MM_HIGHSCORE_LAST_ARCADE_TRACK;
			}
			trackOpen = MM_TrackSelect_boolTrackOpen(MM_ARCADE_TRACKS + MM_HIGHSCORE_TARGET_TRACK);
		} while (!trackOpen);
	}
	else if ((MM_GAME_BUTTON_TAPS[0] & BTN_RIGHT) != 0)
	{
		b32 trackOpen;
		videoResetRequested = true;
		MM_HIGHSCORE_PENDING_HORIZONTAL_MOVE = 1;

		do
		{
			MM_HIGHSCORE_TARGET_TRACK = MM_HIGHSCORE_TARGET_TRACK + 1;
			if (MM_HIGHSCORE_LAST_ARCADE_TRACK < MM_HIGHSCORE_TARGET_TRACK)
			{
				MM_HIGHSCORE_TARGET_TRACK = 0;
			}
			trackOpen = MM_TrackSelect_boolTrackOpen(MM_ARCADE_TRACKS + MM_HIGHSCORE_TARGET_TRACK);
		} while (!trackOpen);
	}
	else
	{
		s32 menuResult = RECTMENU_ProcessInput(&MM_HIGHSCORE_MENU);
		if ((s16)menuResult == -1)
		{
			goto LAB_HIGHSCORE_MENU_BACK;
		}
		if ((s16)menuResult == 1)
		{
			goto LAB_HIGHSCORE_MENU_CONFIRM;
		}
		goto LAB_HIGHSCORE_MENU_RESULT_DONE;

	LAB_HIGHSCORE_MENU_BACK:
		MM_HIGHSCORE_TRANSITION_STATE = EXITING_MENU;
		goto LAB_HIGHSCORE_MENU_RESULT_DONE;

	LAB_HIGHSCORE_MENU_CONFIRM:
		if (MM_HIGHSCORE_MENU.rowSelected == 2)
		{
			MM_HIGHSCORE_TRANSITION_STATE = EXITING_MENU;
		}

	LAB_HIGHSCORE_MENU_RESULT_DONE:
		if (((u16)MM_HIGHSCORE_MENU.rowSelected < 2) && (MM_HIGHSCORE_TARGET_ROW != MM_HIGHSCORE_MENU.rowSelected))
		{
			pendingVerticalMove = -1;
			if (MM_HIGHSCORE_MENU.rowSelected != 0)
			{
				pendingVerticalMove = 1;
			}
			MM_HIGHSCORE_PENDING_VERTICAL_MOVE = pendingVerticalMove;
			MM_HIGHSCORE_TARGET_ROW = MM_HIGHSCORE_MENU.rowSelected;
		}
	}

LAB_OVR_230__800b3c78:

{
	b32 videoState = (((videoResetRequested) || (MM_HIGHSCORE_TRANSITION_TRACK_FRAME != 0)) || (MM_HIGHSCORE_TRANSITION_ROW_FRAME != 0)) ||
	                 (MM_HIGHSCORE_TRANSITION_STATE == EXITING_MENU);
	MM_TrackSelect_Video_State(videoState);
}
	if (MM_HIGHSCORE_TRANSITION_TRACK_FRAME != 0)
	{
		MM_HIGHSCORE_TRANSITION_TRACK_FRAME--;
		if (MM_HIGHSCORE_TRANSITION_TRACK_FRAME == 0)
		{
			MM_HIGHSCORE_CURRENT_TRACK = MM_HIGHSCORE_TARGET_TRACK;
		}
	}
	else if (MM_HIGHSCORE_TRANSITION_ROW_FRAME == 0)
	{
		if (MM_HIGHSCORE_CURRENT_TRACK != MM_HIGHSCORE_TARGET_TRACK)
		{
			MM_HIGHSCORE_TRANSITION_TRACK_FRAME = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
			MM_HIGHSCORE_ACTIVE_HORIZONTAL_MOVE = MM_HIGHSCORE_PENDING_HORIZONTAL_MOVE;
		}
		else if (MM_HIGHSCORE_CURRENT_ROW != MM_HIGHSCORE_TARGET_ROW)
		{
			MM_HIGHSCORE_TRANSITION_ROW_FRAME = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
			MM_HIGHSCORE_ACTIVE_VERTICAL_MOVE = MM_HIGHSCORE_PENDING_VERTICAL_MOVE;
		}
	}
	else
	{
		MM_HIGHSCORE_TRANSITION_ROW_FRAME--;
		if (MM_HIGHSCORE_TRANSITION_ROW_FRAME == 0)
		{
			MM_HIGHSCORE_CURRENT_ROW = MM_HIGHSCORE_TARGET_ROW;
		}
	}

	RECTMENU_DrawSelf(&MM_HIGHSCORE_MENU, MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_MENU_META_INDEX].currX,
	                  MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_MENU_META_INDEX].currY, MM_HIGHSCORE_MENU_WIDTH);

	currOffsetX = 0;
	currOffsetY = 0;
	currentTransitionTrackFrame = MM_HIGHSCORE_TRANSITION_TRACK_FRAME;

	if (currentTransitionTrackFrame != 0)
	{
		sharedV1 = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
		sharedV0 = MM_HIGHSCORE_ACTIVE_HORIZONTAL_MOVE;
		sharedV1 -= currentTransitionTrackFrame;
		sharedV0 *= MM_HIGHSCORE_TRACK_SLIDE_STEP_X;
		currOffsetX = sharedV1 * sharedV0;
	}
	else
	{
		CTR_PSX_LOAD_SYMBOL_PAGE(sharedV1, MM_HIGHSCORE_TRANSITION_ROW_FRAME_ASM_NAME);
		compareOffsetY = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
		activeVerticalMoveAddress = &MM_HIGHSCORE_ACTIVE_VERTICAL_MOVE;
		currentTransitionTrackFrame = CTR_PSX_PAGE_LVALUE(s16, sharedV1, MM_HIGHSCORE_TRANSITION_ROW_FRAME_PAGE_OFFSET, MM_HIGHSCORE_TRANSITION_ROW_FRAME);
		CTR_PSX_KEEP_VALUE(currentTransitionTrackFrame);
		sharedV1 = *activeVerticalMoveAddress;
		compareOffsetY -= currentTransitionTrackFrame;
		sharedV0 = sharedV1 * MM_HIGHSCORE_ROW_SLIDE_STEP_Y;
		currOffsetY = compareOffsetY * sharedV0;
	}
	if (((currOffsetX != -MM_HIGHSCORE_OFFSCREEN_X) && (currOffsetX != MM_HIGHSCORE_OFFSCREEN_X)) &&
	    ((currOffsetY != -MM_HIGHSCORE_OFFSCREEN_Y && (currOffsetY != MM_HIGHSCORE_OFFSCREEN_Y))))
	{
		MM_HighScore_Draw((int)MM_HIGHSCORE_CURRENT_TRACK, (int)MM_HIGHSCORE_CURRENT_ROW, (int)(s16)currOffsetX, (int)(s16)currOffsetY);
		if (MM_HIGHSCORE_TRANSITION_ROW_FRAME != 0)
		{
			// draw rectangle
			sharedV0 = MM_HIGHSCORE_WIPE_RECT_W;
			CTR_PSX_KEEP_VALUE(sharedV0);
			titleMeta = &MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX];
			sharedV1 = MM_HIGHSCORE_WIPE_RECT_H;
			wipeRect.w = sharedV0;
			sharedV0 = (u16)titleMeta->currX;
			compareOffsetY = currOffsetY + MM_HIGHSCORE_WIPE_RECT_Y_OFFSET;
			wipeRect.h = sharedV1;
			wipeRect.x = sharedV0 + MM_HIGHSCORE_WIPE_RECT_X_OFFSET;
			wipeRect.y = titleMeta->currY + (s16)compareOffsetY;
			RECTMENU_DrawInnerRect(&wipeRect, 0, sdata_static.gGT->backBuffer->otMem.uiOT);
		}
	}
	nextOffsetX = 0;
	nextOffsetY = 0;
	compareOffsetX = currOffsetX;
	compareOffsetY = currOffsetY;
	sharedV0 = MM_HIGHSCORE_TRANSITION_TRACK_FRAME;
	if (sharedV0 != 0)
	{
		sharedV0 *= MM_HIGHSCORE_TRACK_SLIDE_STEP_X;
		sharedV1 = MM_HIGHSCORE_ACTIVE_HORIZONTAL_MOVE;
		sharedV0 = -sharedV0;
		nextOffsetX = sharedV0 * sharedV1;
	}
	else
	{
		currentTransitionTrackFrame = MM_HIGHSCORE_TRANSITION_ROW_FRAME;
		sharedV1 = MM_HIGHSCORE_ACTIVE_VERTICAL_MOVE;
		currentTransitionTrackFrame *= -MM_HIGHSCORE_ROW_SLIDE_STEP_Y;
		nextOffsetY = currentTransitionTrackFrame * sharedV1;
	}
	if (((compareOffsetX != nextOffsetX) || (compareOffsetY != nextOffsetY)) &&
	    ((nextOffsetX != -MM_HIGHSCORE_OFFSCREEN_X &&
	      (((nextOffsetX != MM_HIGHSCORE_OFFSCREEN_X && (nextOffsetY != -MM_HIGHSCORE_OFFSCREEN_Y)) && (nextOffsetY != MM_HIGHSCORE_OFFSCREEN_Y))))))
	{
		MM_HighScore_Draw((int)MM_HIGHSCORE_TARGET_TRACK, (int)MM_HIGHSCORE_TARGET_ROW, (int)(s16)nextOffsetX, (int)(s16)nextOffsetY);
	}

	// draw rectangle
	sharedV0 = MM_HIGHSCORE_WIPE_RECT_W;
	CTR_PSX_KEEP_VALUE(sharedV0);
	titleMeta = &MM_HIGHSCORE_TRANSITIONS[MM_HIGHSCORE_TITLE_META_INDEX];
	sharedV1 = MM_HIGHSCORE_WIPE_RECT_H;
	wipeRect.w = sharedV0;
	sharedV0 = (u16)titleMeta->currX;
	compareOffsetY = nextOffsetY + MM_HIGHSCORE_WIPE_RECT_Y_OFFSET;
	wipeRect.h = sharedV1;
	wipeRect.x = sharedV0 + MM_HIGHSCORE_WIPE_RECT_X_OFFSET;
	wipeRect.y = titleMeta->currY + (s16)compareOffsetY;
	RECTMENU_DrawInnerRect(&wipeRect, 0, sdata_static.gGT->backBuffer->otMem.uiOT);

	return;
}

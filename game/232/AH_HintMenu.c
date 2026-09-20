#include <common.h>
#include "AH_UI.h"

void AH_HintMenu_FiveArrows(int posY, s16 rotation)
{
	{
		s16 i;
		for (i = 0; i < AH_HINTMENU_ARROW_COUNT; i++)
		{
			u32 *ptrColor = AH_FIVE_ARROW_COLOR_1;
			s32 x = (s16)(i * AH_HINTMENU_ARROW_SPACING + AH_HINTMENU_ARROW_START_X);
			s32 y = (s16)(posY + AH_HINTMENU_ARROW_Y_OFFSET);
			SVec2 *arrowPos = D232.fiveArrowPos;
			if ((AH_FRAME_COUNTER & 2) != 0)
			{
				ptrColor = AH_FIVE_ARROW_COLOR_2;
			}
			AH_Map_HubArrow(
			    // posX
			    x,

			    // posY
			    y,

			    arrowPos,

			    (char *)ptrColor, 0x800, (int)rotation);
		}
	}
}

void AH_HintMenu_MaskPosRot(void)
{
	struct Instance *mask;

	ConvertRotToMatrix(&AH_HINT_MASK->matrix, &AH_MASK_ROT);
	mask = AH_HINT_MASK;

	// Set position
	mask->matrix.t[0] = AH_MASK_POS.x;
	mask->matrix.t[1] = AH_MASK_POS.y;
	mask->matrix.t[2] = AH_MASK_POS.z;

	// Apply the default mask model scale from D232.
	((struct MaskHint *)mask->thread->object)->scale = AH_MASK_SCALE;

	return;
}

void AH_HintMenu_MenuProc(struct RectMenu *inputMenu)
{
	struct RectMenu *menu = inputMenu;
	b16 shouldClose = false;
	s16 numHintsFound = 0;
	s16 visibleRows = AH_HINTMENU_VISIBLE_ROWS;
	s16 i;
	s16 rowCount;
	s32 scrollEnd;
	u8 hintsFound[AH_HINTMENU_HISTORY_CAPACITY];
	RECT box;
	register RECT *edgeRect CTR_PSX_REGISTER("$4");
	register s16 left CTR_PSX_REGISTER("$18");

	MainFreeze_SafeAdvDestroy();
	i = 0;
	UNLOCK_ADV_BIT(GAME_ADV_PROGRESS.rewards, ADV_REWARD_HINT_WELCOME_TO_ARENA);
	// The Exit row has no hint-table entry; resolve hint IDs only when needed.
	for (; AH_MASK_HINT_INDICES[i] >= 0; i++)
	{
		u32 *rewards = GAME_ADV_PROGRESS.rewards;
		s32 hintID = (AH_MASK_HINT_INDICES[i] - AH_HINTMENU_HINT_LNG_FIRST) / 2;
		s32 bitIndex = hintID + ADV_REWARD_FIRST_HINT;
		if (CHECK_ADV_BIT(rewards, bitIndex))
			hintsFound[numHintsFound++] = i;
	}
	rowCount = numHintsFound + 1;
	if (menu->rowSelected > numHintsFound)
		menu->rowSelected = numHintsFound;
	if (AH_HINT_MENU_SCROLL > rowCount - AH_HINTMENU_VISIBLE_ROWS)
	{
		AH_HINT_MENU_SCROLL = numHintsFound - AH_HINTMENU_SCROLL_MARGIN;
		if (AH_HINT_MENU_SCROLL < 0)
			AH_HINT_MENU_SCROLL = 0;
	}

	if (!AH_HINT_MENU_VIEW)
	{
		u32 tapP1 = AH_PAUSE_BUTTON_TAP;
		char **title;
		s16 menuHeight;
		s16 totalHeight;
		if (tapP1 & AH_HINTMENU_INPUT_NAV)
		{
			if (tapP1 & BTN_UP)
			{
				if (menu->rowSelected > 0)
				{
					OtherFX_Play(0, 1);
					menu->rowSelected--;
				}
			}
			else if (tapP1 & BTN_DOWN)
			{
				if (menu->rowSelected < numHintsFound)
				{
					OtherFX_Play(0, 1);
					menu->rowSelected++;
				}
			}
			else if (tapP1 & AH_HINTMENU_INPUT_CONFIRM)
			{
				OtherFX_Play(1, 1);
				if (menu->rowSelected == numHintsFound)
					shouldClose = true;
				else if (!AH_LOAD_IN_PROGRESS && !AH_MASK_XA_STATE)
				{
					register struct GameTracker *gGT CTR_PSX_REGISTER("$5");
					s16 j;
					s16 firstPlayer;
					b16 morePlayers;
					struct Instance *drawInst;
					struct Instance *inst;
					AH_HINT_MASK = VehTalkMask_Init();
					AH_MASK_COOLDOWN = AH_HINTMENU_VIEW_COOLDOWN_FRAMES;
					VehTalkMask_PlayXA(AH_HINT_MASK, (s16)((AH_MASK_HINT_INDICES[hintsFound[menu->rowSelected]] - AH_HINTMENU_HINT_LNG_FIRST) / 2));
					AH_HintMenu_MaskPosRot();
					inst = AH_HINT_MASK;
					gGT = GAME_TRACKER;
					// The talking mask belongs only to the UI viewport.
					INST_GETIDPP(inst)[0].pushBuffer = &gGT->pushBuffer_UI;
					inst->flags |= SCREENSPACE_INSTANCE;

					firstPlayer = 1;
					morePlayers = firstPlayer < gGT->numPlyrCurrGame;
					if (morePlayers)
					{
						j = 1;
						drawInst = inst;
						do
						{
							u32 drawAddress = (u32)drawInst;
							drawAddress += j * sizeof(struct InstDrawPerPlayer);
							INST_GETIDPP((struct Instance *)drawAddress)->pushBuffer = NULL;
							j++;
						} while (j < gGT->numPlyrCurrGame);
					}
					AH_HINT_MENU_VIEW = true;
				}
			}
			else if (tapP1 & AH_HINTMENU_INPUT_BACK)
			{
				OtherFX_Play(2, 1);
				shouldClose = true;
			}
			RECTMENU_ClearInput();
		}
		if ((s16)VehPickupItem_MaskBoolGoodGuy(GAME_TRACKER->drivers[0]))
			title = &GAME_LANGUAGE_STRINGS[LNG_AKU_AKU_HINTS_MENU];
		else
			title = &GAME_LANGUAGE_STRINGS[LNG_AKU_AKU_HINTS_MENU + 1];
		DecalFont_DrawLine(*title, 0x100, 0x2c, FONT_BIG, JUSTIFY_CENTER | ORANGE);
		if (AH_HINT_MENU_SCROLL + visibleRows <= menu->rowSelected)
			AH_HINT_MENU_SCROLL = menu->rowSelected - visibleRows + 1;
		if (menu->rowSelected < AH_HINT_MENU_SCROLL)
			AH_HINT_MENU_SCROLL = menu->rowSelected;
		scrollEnd = AH_HINT_MENU_SCROLL + visibleRows;
		if (rowCount < scrollEnd)
			visibleRows = rowCount;
		if (AH_HINT_MENU_SCROLL > 0)
			AH_HintMenu_FiveArrows(0x3f, 0);
		menuHeight = 0x10;
		i = 0;
		if (visibleRows > 0)
		{
			do
			{
				s16 rowLngIndex;
				s32 hintListIndex = AH_HINT_MENU_SCROLL + i;
				s16 rowPosY;
				if (hintListIndex < numHintsFound)
					rowLngIndex = AH_MASK_HINT_INDICES[hintsFound[hintListIndex]];
				else
					rowLngIndex = LNG_HINT_EXIT;
				rowPosY = menuHeight + 0x40;
				DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[rowLngIndex], 0x100, rowPosY, FONT_BIG, JUSTIFY_CENTER | ORANGE);
				menuHeight += 0x10;
				i++;
			} while (i < visibleRows);
		}
		if (AH_HINT_MENU_SCROLL + visibleRows < rowCount)
			AH_HintMenu_FiveArrows((s16)(menuHeight + 0x40), 0x800);
		box.x = -0xe;
		box.w = 0x21c;
		box.y = (menu->rowSelected - AH_HINT_MENU_SCROLL) * 0x10 + 0x4f;
		box.h = 0x11;
		CTR_Box_DrawClearBox(&box, &GAME_MENU_HIGHLIGHT, TRANS_50_DECAL, GAME_TRACKER->backBuffer->otMem.uiOT, &GAME_TRACKER->backBuffer->primMem);
		// NOTE(aalhendi): Keep the frame arguments live through rectangle setup.
		edgeRect = &box;
		left = -0x14;
		CTR_PSX_ORDER_VALUES(edgeRect, left);
		box.y = 0x3c;
		box.x = left;
		box.w = 0x228;
		box.h = 2;
		AH_DrawMenuEdge(edgeRect, GAME_TRACKER->backBuffer->otMem.uiOT);
		box.y = 0x28;
		// NOTE(aalhendi): Preserve the separate halfword sizing step after the Y store.
		CTR_PSX_OBSERVE_MEMORY(box.y);
		totalHeight = menuHeight + 0x10;
		CTR_PSX_KEEP_VALUE_RELAXED(totalHeight);
		box.x = left;
		box.w = 0x228;
		box.h = totalHeight + 0x1b;
		RECTMENU_DrawInnerRect(&box, 4, GAME_TRACKER->backBuffer->otMem.uiOT);
		if (shouldClose || (AH_PAUSE_BUTTON_TAP & AH_HINTMENU_INPUT_CLOSE))
		{
			RECTMENU_ClearInput();
			sdata->ptrDesiredMenu = MainFreeze_GetMenuPtr();
		}
	}
	else
	{
		s16 lngIndex = AH_MASK_HINT_INDICES[hintsFound[menu->rowSelected]];
		s32 textHeight;
		s16 exitBoxWidth;
		AH_HintMenu_MaskPosRot();
		if (AH_MASK_COOLDOWN > 0)
			AH_MASK_COOLDOWN--;
		if ((AH_PAUSE_BUTTON_TAP & AH_HINTMENU_INPUT_VIEW_EXIT) && ((s16)VehTalkMask_boolNoXA() || AH_MASK_COOLDOWN == 0))
		{
			AH_HINT_MENU_VIEW = false;
			RECTMENU_ClearInput();
			VehTalkMask_End();
		}
		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[lngIndex], 0x100, 0x2c, FONT_BIG, JUSTIFY_CENTER | ORANGE);
		textHeight = DecalFont_DrawMultiLine(GAME_LANGUAGE_STRINGS[lngIndex + 1], 0x96, 0x3f, 0x14e, 2, 0);
		DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_HINT_EXIT], 0x100, textHeight + 0x4f, FONT_BIG, JUSTIFY_CENTER | ORANGE);
		exitBoxWidth = DecalFont_GetLineWidth(GAME_LANGUAGE_STRINGS[LNG_HINT_EXIT], 1) + 6;
		box.x = 0xff - exitBoxWidth / 2;
		box.w = exitBoxWidth;
		box.y = textHeight + 0x4e;
		box.h = 0x11;
		CTR_Box_DrawClearBox(&box, &GAME_MENU_HIGHLIGHT, TRANS_50_DECAL, GAME_TRACKER->backBuffer->otMem.uiOT, &GAME_TRACKER->backBuffer->primMem);
		edgeRect = &box;
		left = -0xe;
		CTR_PSX_ORDER_VALUES(edgeRect, left);
		box.y = 0x3c;
		box.x = left;
		box.w = 0x21c;
		box.h = 2;
		AH_DrawMenuEdge(edgeRect, GAME_TRACKER->backBuffer->otMem.uiOT);
		box.y = 0x28;
		box.x = left;
		box.w = 0x21c;
		box.h = textHeight + 0x3b;
		RECTMENU_DrawInnerRect(&box, 4, &GAME_TRACKER->backBuffer->otMem.uiOT[3]);
	}
}

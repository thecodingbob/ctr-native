#include <common.h>

global_variable struct RectMenu menuVS;
global_variable struct RectMenu menuBattle;

enum VersusBattleStandingsPosY
{
	VB_POSY_TITLE = 0,
	VB_POSY_P1,
	VB_POSY_P2,
	VB_POSY_P3,
	VB_POSY_P4,
	VB_POSY_NUM
};

enum VersusBattleEndMenuConstants
{
	VB_MAX_PLAYERS = 4,
	VB_MIN_PLAYERS = 2,
	VB_RESULT_MAX_FRAMES = CTR_SECONDS_TO_FRAMES(8),
	VB_MENU_SHOW_DELAY_FRAMES = 25,
	VB_ROW_INITIAL_DELAY_FRAMES = 30,
	VB_ROW_STAGGER_FRAMES = 5,
	VB_LERP_FRAMES = 5,

	VB_TITLE_ENTRY_X = 0x296,
	VB_TITLE_TARGET_X = 0x180,
	VB_ROW_TARGET_X = 0x165,
	VB_TITLE_TO_ROWS_Y = 0x28,
	VB_STANDINGS_SUBTITLE_Y = 0x11,

	VB_BATTLE_BLOCK_BOTTOM_Y = 0xd8,
	VB_BATTLE_TEAM_SCORE_GAP = 10,
	VB_BATTLE_TITLE_PLAYER_HEIGHT = 0x1a,
	VB_BATTLE_PLAYER_ICON_SPACING = 0x1b,
	VB_BATTLE_BLOCK_HEADER_HEIGHT = 0x28,
	VB_BATTLE_RANK_TEXT_CENTER_Y = 0xd,

	VB_STANDINGS_VISIBLE_PLACES_MIN = 2,
	VB_STANDINGS_EXPANDED_MIN_ENTRIES = 3,
	VB_STANDINGS_POINTS_PER_ENTRY = 3,
	VB_STANDINGS_TEXT_X_OFFSET = 0x79,
	VB_STANDINGS_RANK_X_OFFSET = -0x24,
	VB_STANDINGS_RANK_Y_OFFSET = 5,
	VB_STANDINGS_ROW_HALF_HEIGHT = 4,
	VB_ICON_TRANSPARENCY = 1,
	VB_ICON_SCALE = 0x1000,

	VB_WINNER_TARGET_X = 0x14,
	VB_WINNER_TARGET_Y = 0xc,
	VB_WINNER_LERP_FRAMES = 25,
	VB_WINNER_2P_MIN_WIDE_RECT = 0x100,
	VB_WINNER_2P_WIDTH_STEP = 0xc,
	VB_WINNER_DISTANCE_TO_SCREEN = 0x80,
	VB_WINNER_BOX_X_PAD = 3,
	VB_WINNER_BOX_Y_PAD = 2,

	VB_LOSER_RECT_STEP_X = 5,
	VB_LOSER_RECT_STEP_Y = 3,
	VB_LOSER_RECT_STEP_W = 10,
	VB_LOSER_RECT_STEP_H = 6,

	VB_MENU_READY_SHOW_MENU = 1,
};

global_variable s16 s_vsStandingsYByPlayerCount[3][VB_POSY_NUM];
global_variable s16 s_standingsSuffixStringIds225[VB_MAX_PLAYERS];

// NOTE(aalhendi): The matching build replaces these defaults through its
// private retail-symbol header. Native uses the canonical aggregates below.
#ifdef CTR_NATIVE
#define VB_GAME_TRACKER_PAGE                 0u
#define VB_BATTLE_COLOR_PTR                  (&sdata->battleSetup_Color_UI_1)
#define VB_STANDINGS_SUFFIX_PAGE             s_standingsSuffixStringIds225
#define VB_ADD_STANDINGS_SUFFIX_LOW(value)   ((void)sizeof(value))
#define VB_PLAYER_COUNT_FROM_PAGE(page)      ((void)sizeof(page), GAME_TRACKER->numPlyrCurrGame)
#define VB_VIEW_TYPE                         struct PushBuffer
#define VB_VIEW_FROM_OFFSET(offset)          ((struct PushBuffer *)((u8 *)&GAME_TRACKER->pushBuffer + (offset)))
#define VB_VIEW_PUSH_BUFFER(view)            (*(view))
#define VB_PUSH_BUFFER_FROM_OFFSET(offset)   (*VB_VIEW_FROM_OFFSET(offset))
#define VB_MATCH_ROW_ALLOCATION_BEGIN(value) ((void)sizeof(value))
#define VB_MATCH_ROW_SCHEDULE_BEGIN()        ((void)0)
#define VB_MATCH_ROW_SCHEDULE_END()          ((void)0)
#define VB_MATCH_ROW_SETUP_ORDER(rowCount, configIndex) \
	do                                                  \
	{                                                   \
		(void)sizeof(rowCount);                         \
		(void)sizeof(configIndex);                      \
	} while (0)
#define VB_MATCH_ROW_ALLOCATION_END(value) ((void)sizeof(value))
#define VB_VALIDATE_MATCHING_CONSTANTS()

static inline void VB_DrawOuterRect(RECT *rect, const u32 *packedColor, s16 transparency, u32 *ot)
{
	Color color;

	ColorCode_SetPacked(&color, *packedColor);
	RECTMENU_DrawOuterRect_HighLevel(rect, &color, transparency, ot);
}

#define VB_DRAW_OUTER_RECT VB_DrawOuterRect
#define VB_DRAW_POLY_FT4   DecalHUD_DrawPolyFT4
#endif

VB_VALIDATE_MATCHING_CONSTANTS()

void VB_EndEvent_DrawMenu(void)
{
	char text[24];
	SVec2 pos;
	struct GameTracker *battleGameTracker;
	struct GameTracker *gGT;
	s32 titleString;
	s32 rankTextBaselineOffset;
	u16 rankTextY;
	b16 winnerViewportFound;
	u16 standingsEntryCount;
	s16 standingsPosition;
	s16 previousStandingsScore;
	s16 displayedRankOffset;
	u32 screenY;
	s32 player;
	s32 titleAnimationFrame;
	s32 rowAnimationFrame;
	s32 battleBlockHeight;
	s32 battlePlayerHeight;
	s32 resultIndex;
	s32 teamPlayerHeight;
	s32 pushBufferOffset;
	s32 teamID;

	winnerViewportFound = false;
	standingsPosition = VB_POSY_P1;
	previousStandingsScore = 0;

	{
		s16 teamPlayerCount[VB_MAX_PLAYERS] = {0, 0, 0, 0};

		{
			RECT box;

			displayedRankOffset = 0;

			if (GAME_FRAMES_SINCE_RACE_ENDED < VB_RESULT_MAX_FRAMES)
			{
				GAME_FRAMES_SINCE_RACE_ENDED++;
			}

			titleString = LNG_BATTLE;
			gGT = GAME_TRACKER;
			if ((gGT->gameMode1 & BATTLE_MODE) != 0)
			{
				standingsEntryCount = gGT->battleSetup.numTeams;
				for (resultIndex = 0; gGT->numPlyrCurrGame != 0;)
				{
					battleGameTracker = gGT;
					do
					{
						teamID = battleGameTracker->drivers[resultIndex]->BattleHUD.teamID;
						// NOTE(aalhendi): Preserve retail's temporary-register allocation
						// across the team-count update without changing game state.
						CTR_PSX_CLOBBER("$2");
						teamPlayerCount[teamID]++;
						resultIndex++;
					} while (resultIndex < battleGameTracker->numPlyrCurrGame);
					break;
				}

				battleBlockHeight = (GAME_TRACKER->battleSetup.numTeams - 1) * VB_BATTLE_TEAM_SCORE_GAP;
				battlePlayerHeight = GAME_TRACKER->numPlyrCurrGame * VB_BATTLE_TITLE_PLAYER_HEIGHT + VB_BATTLE_BLOCK_HEADER_HEIGHT;
				battleBlockHeight += battlePlayerHeight;
				screenY = (u32)(VB_BATTLE_BLOCK_BOTTOM_Y - battleBlockHeight) >> 1;
			}
			else
			{
				titleString = LNG_VERSUS;
				standingsEntryCount = gGT->numPlyrCurrGame;
				screenY = (u16)s_vsStandingsYByPlayerCount[gGT->numPlyrCurrGame - VB_MIN_PLAYERS][VB_POSY_TITLE];
			}

			GAME_TRACKER->renderFlags &= ~RENDER_FLAG_SPLIT_SCREEN_LINES;
			RaceFlag_SetFullyOnScreen();

			titleAnimationFrame = GAME_FRAMES_SINCE_RACE_ENDED;
			if (VB_MENU_SHOW_DELAY_FRAMES < titleAnimationFrame)
			{
				UI_Lerp2D_Linear(CTR_VECTOR_DATA(&(pos)), VB_TITLE_ENTRY_X, screenY, VB_TITLE_TARGET_X, screenY,
				                 titleAnimationFrame - VB_MENU_SHOW_DELAY_FRAMES, VB_LERP_FRAMES);
			}
			else
			{
				UI_Lerp2D_Linear(CTR_VECTOR_DATA(&(pos)), VB_TITLE_ENTRY_X, screenY, VB_TITLE_ENTRY_X, screenY, titleAnimationFrame, VB_LERP_FRAMES);
			}

			DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[titleString], pos.x, pos.y, FONT_BIG, (JUSTIFY_CENTER | ORANGE));

			screenY += VB_TITLE_TO_ROWS_Y;
			resultIndex = 0;

			DecalFont_DrawLine(GAME_LANGUAGE_STRINGS[LNG_STANDINGS], pos.x, pos.y + VB_STANDINGS_SUBTITLE_Y, FONT_BIG, (JUSTIFY_CENTER | ORANGE));

			titleAnimationFrame = (s16)standingsEntryCount;
			{
				s32 standingsCount;

				standingsCount = titleAnimationFrame;
				if (standingsCount > 0)
				{
					s32 placeTextColor;
					s16 teamCount;
					s16 displayedRankIndex;
					s32 displayedRankNumber;
					s16 visiblePlaceCount;
					s32 entityRank;
					s16 currRowY;
					s16 iconSlot;
					s32 iconSlotBeforeIncrement;
					s32 allocationAnchor;
					s32 rowCount;
					s32 standingsConfigIndex;
					b32 rowCountBelowThree;
					s32 rowDelay;
					s32 rowStagger;
					s32 rowFrame;
					s16 *rankSuffix;
					// NOTE(aalhendi): These matching-only register annotations preserve
					// retail's final sprintf argument schedule. Native ignores them.
					register s16 *suffixBase CTR_PSX_REGISTER("$2");
					register char *rankText CTR_PSX_REGISTER("$4");
					register const char *rankFormat CTR_PSX_REGISTER("$5");

					VB_MATCH_ROW_ALLOCATION_BEGIN(allocationAnchor);
					VB_MATCH_ROW_SCHEDULE_BEGIN();
					rowDelay = VB_ROW_INITIAL_DELAY_FRAMES;
					standingsConfigIndex = standingsCount - VB_MIN_PLAYERS;
					rowCountBelowThree = standingsCount < VB_STANDINGS_EXPANDED_MIN_ENTRIES;
					rowCount = standingsCount;
					VB_MATCH_ROW_SETUP_ORDER(rowCount, standingsConfigIndex);
					rowStagger = VB_ROW_STAGGER_FRAMES;
					do
					{
						VB_MATCH_ROW_SCHEDULE_END();
						teamCount = teamPlayerCount[GAME_TRACKER->battleSetup.standingsOrder[resultIndex]];
						rankTextBaselineOffset = VB_BATTLE_RANK_TEXT_CENTER_Y;
						teamPlayerHeight = teamCount * VB_BATTLE_PLAYER_ICON_SPACING;
						rowFrame = GAME_FRAMES_SINCE_RACE_ENDED;

						if (rowFrame > rowDelay)
						{
							rowAnimationFrame = rowFrame - VB_MENU_SHOW_DELAY_FRAMES;
							UI_Lerp2D_Linear(CTR_VECTOR_DATA(&(pos)), VB_TITLE_ENTRY_X, (s16)screenY, VB_ROW_TARGET_X, (s16)screenY,
							                 rowAnimationFrame - rowStagger, VB_LERP_FRAMES);
						}
						else
						{
							UI_Lerp2D_Linear(CTR_VECTOR_DATA(&(pos)), VB_TITLE_ENTRY_X, (s16)screenY, VB_TITLE_ENTRY_X, (s16)screenY, rowFrame, VB_LERP_FRAMES);
						}

						iconSlot = 0;
						if ((GAME_TRACKER->gameMode1 & BATTLE_MODE) == 0)
						{
							s16 iconY;
							s16 *vsCharacterIDs;

							iconY = s_vsStandingsYByPlayerCount[standingsConfigIndex][standingsPosition];
							rankTextY = iconY;
							vsCharacterIDs = GAME_CHARACTER_IDS;

							VB_DRAW_POLY_FT4(
							    GAME_TRACKER->ptrIcons[GAME_CHARACTER_METADATA[vsCharacterIDs[GAME_TRACKER->battleSetup.standingsOrder[resultIndex]]].iconID],
							    pos.x, iconY, &GAME_TRACKER->backBuffer->primMem, GAME_TRACKER->pushBuffer_UI.ptrOT, VB_ICON_TRANSPARENCY, VB_ICON_SCALE);
						}
						else
						{
							u32 nextScreenY;

							currRowY = (s16)screenY;
							rankTextY = currRowY + (s16)teamPlayerHeight / 2 - rankTextBaselineOffset;
							player = 0;

							if (GAME_TRACKER->numPlyrCurrGame != 0)
							{
								s32 battleIconY;

								battleIconY = currRowY;
								do
								{
									if (GAME_TRACKER->drivers[player]->BattleHUD.teamID == GAME_TRACKER->battleSetup.standingsOrder[resultIndex])
									{
										iconSlotBeforeIncrement = iconSlot;
										VB_DRAW_POLY_FT4(GAME_TRACKER->ptrIcons[GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[player]].iconID], pos.x,
										                 battleIconY + iconSlotBeforeIncrement * VB_BATTLE_PLAYER_ICON_SPACING,
										                 &GAME_TRACKER->backBuffer->primMem, GAME_TRACKER->pushBuffer_UI.ptrOT, VB_ICON_TRANSPARENCY,
										                 VB_ICON_SCALE);
										iconSlot++;
									}

									player++;
								} while (player < GAME_TRACKER->numPlyrCurrGame);
							}

							nextScreenY = screenY + VB_BATTLE_TEAM_SCORE_GAP;
							screenY = teamPlayerHeight + nextScreenY;
						}

						standingsPosition++;
						visiblePlaceCount = VB_STANDINGS_VISIBLE_PLACES_MIN;
						if (rowCountBelowThree == 0)
						{
							visiblePlaceCount = standingsEntryCount - 1;
						}
						player = 0;
						if (visiblePlaceCount > 0)
						{
							do
							{
								if ((GAME_TRACKER->gameMode1 & BATTLE_MODE) == 0)
								{
									entityRank = GAME_TRACKER->drivers[GAME_TRACKER->battleSetup.standingsOrder[resultIndex]]->driverRank;
									if (player == entityRank)
									{
										goto HighlightCurrentPlace;
									}

									placeTextColor = JUSTIFY_RIGHT | RED;
									goto DrawPlace;
								}
								else
								{
									entityRank = GAME_TRACKER->battleSetup.finishedRankOfEachTeam[GAME_TRACKER->battleSetup.standingsOrder[resultIndex]];
									if (player != entityRank)
									{
										placeTextColor = JUSTIFY_RIGHT | RED;
										goto DrawPlace;
									}
								}

							HighlightCurrentPlace:
								placeTextColor = JUSTIFY_RIGHT | WHITE;
								if ((GAME_TRACKER->timer & 1) != 0)
								{
									placeTextColor = JUSTIFY_RIGHT | RED;
								}

							DrawPlace:
								sprintf(
								    text, "%d%s-%2.02ld", player + 1, GAME_LANGUAGE_STRINGS[s_standingsSuffixStringIds225[player]],
								    CTR_PRINTF_PSX_LONG(
								        GAME_TRACKER
								            ->standingsPoints[GAME_TRACKER->battleSetup.standingsOrder[resultIndex] * VB_STANDINGS_POINTS_PER_ENTRY + player]));

								DecalFont_DrawLine(text, pos.x + VB_STANDINGS_TEXT_X_OFFSET,
								                   rankTextY - (visiblePlaceCount * VB_STANDINGS_ROW_HALF_HEIGHT + -rankTextBaselineOffset) + player * 8,
								                   FONT_SMALL, placeTextColor);
								player++;
							} while (player < visiblePlaceCount);
						}

						if (GAME_TRACKER->battleSetup.standingsScore[GAME_TRACKER->battleSetup.standingsOrder[resultIndex]] == previousStandingsScore)
						{
							displayedRankOffset++;
						}
						else
						{
							displayedRankOffset = 0;
						}

						displayedRankIndex = (s16)resultIndex - displayedRankOffset;
						if (displayedRankOffset == 0)
						{
							displayedRankIndex = (s16)resultIndex;
						}

						rankText = text;
						rankFormat = "%d%s";
						displayedRankNumber = displayedRankIndex + 1;
						CTR_PSX_KEEP_VALUE(displayedRankNumber);
						suffixBase = VB_STANDINGS_SUFFIX_PAGE;
						VB_ADD_STANDINGS_SUFFIX_LOW(suffixBase);
						rankSuffix = suffixBase + displayedRankIndex;
						rowDelay += VB_ROW_STAGGER_FRAMES;
						rowStagger += VB_ROW_STAGGER_FRAMES;
						previousStandingsScore = (s16)GAME_TRACKER->battleSetup.standingsScore[GAME_TRACKER->battleSetup.standingsOrder[resultIndex]];

						sprintf(rankText, rankFormat, displayedRankNumber, GAME_LANGUAGE_STRINGS[*rankSuffix]);
						resultIndex++;
						DecalFont_DrawLine(text, pos.x + VB_STANDINGS_RANK_X_OFFSET, rankTextY + VB_STANDINGS_RANK_Y_OFFSET, FONT_BIG,
						                   (JUSTIFY_CENTER | ORANGE));
					} while (resultIndex < rowCount);
					VB_MATCH_ROW_ALLOCATION_END(allocationAnchor);
				}
			}

			resultIndex = 0;
			if (GAME_TRACKER->numPlyrCurrGame != 0)
			{
				struct Instance *bigNum;
				VB_VIEW_TYPE *viewGameTracker;
				register u32 gameTrackerPage CTR_PSX_REGISTER("$3");

				do
				{
					pushBufferOffset = resultIndex * sizeof(struct PushBuffer);
					bigNum = GAME_TRACKER->drivers[resultIndex]->instBigNum;

					if (bigNum != NULL)
					{
						bigNum->scale.z = 0;
						bigNum->scale.y = 0;
						bigNum->scale.x = 0;
					}

					if (winnerViewportFound == false)
					{
						if ((GAME_TRACKER->gameMode1 & BATTLE_MODE) == 0)
						{
							if (resultIndex != GAME_TRACKER->driversInRaceOrder[0]->driverID)
							{
								goto DrawLoserViewport;
							}

							goto DrawWinnerViewport;
						}
						else
						{
							if (GAME_TRACKER->winnerIndex[0] != resultIndex)
							{
								goto DrawLoserViewport;
							}

							goto DrawWinnerViewport;
						}
					}

				DrawLoserViewport:
					viewGameTracker = VB_VIEW_FROM_OFFSET(pushBufferOffset);
					if (VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.w > 0)
					{
						VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.x += VB_LOSER_RECT_STEP_X;
						VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.y += VB_LOSER_RECT_STEP_Y;
						VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.w -= VB_LOSER_RECT_STEP_W;
						VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.h -= VB_LOSER_RECT_STEP_H;
					}
					goto AdvanceViewport;

				DrawWinnerViewport:
					winnerViewportFound = true;
					if (GAME_TRACKER->numPlyrCurrGame == 2)
					{
						viewGameTracker = VB_VIEW_FROM_OFFSET(pushBufferOffset);
						if (VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.w > VB_WINNER_2P_MIN_WIDE_RECT)
						{
							VB_VIEW_PUSH_BUFFER(viewGameTracker).rect.w -= VB_WINNER_2P_WIDTH_STEP;
							VB_VIEW_PUSH_BUFFER(viewGameTracker).distanceToScreen_CURR = VB_WINNER_DISTANCE_TO_SCREEN;
						}
					}

					UI_Lerp2D_Linear(CTR_VECTOR_DATA(&(pos)), VB_PUSH_BUFFER_FROM_OFFSET(pushBufferOffset).rect.x,
					                 VB_PUSH_BUFFER_FROM_OFFSET(pushBufferOffset).rect.y, VB_WINNER_TARGET_X, VB_WINNER_TARGET_Y, GAME_FRAMES_SINCE_RACE_ENDED,
					                 VB_WINNER_LERP_FRAMES);

					box.x = pos.x - VB_WINNER_BOX_X_PAD;
					box.y = pos.y - VB_WINNER_BOX_Y_PAD;
					box.w = VB_PUSH_BUFFER_FROM_OFFSET(pushBufferOffset).rect.w + 2 * VB_WINNER_BOX_X_PAD;
					box.h = VB_PUSH_BUFFER_FROM_OFFSET(pushBufferOffset).rect.h + 2 * VB_WINNER_BOX_Y_PAD;

					VB_DRAW_OUTER_RECT(&box, VB_BATTLE_COLOR_PTR, 0, GAME_TRACKER->backBuffer->otMem.uiOT);

					{
						VB_VIEW_TYPE *winnerView;

						winnerView = VB_VIEW_FROM_OFFSET(pushBufferOffset);
						VB_VIEW_PUSH_BUFFER(winnerView).rect.x = pos.x;
						VB_VIEW_PUSH_BUFFER(winnerView).rect.y = pos.y;
					}

				AdvanceViewport:
					gameTrackerPage = VB_GAME_TRACKER_PAGE;
					CTR_PSX_KEEP_VALUE(gameTrackerPage);
					resultIndex++;
				} while (resultIndex < VB_PLAYER_COUNT_FROM_PAGE(gameTrackerPage));
			}
		}
	}

	if (((GAME_MENU_READY & VB_MENU_READY_SHOW_MENU) == 0) && (VB_MENU_SHOW_DELAY_FRAMES < GAME_FRAMES_SINCE_RACE_ENDED))
	{
		if ((GAME_TRACKER->gameMode1 & BATTLE_MODE) != 0)
		{
			RECTMENU_Show(&menuBattle);
		}
		else
		{
			RECTMENU_Show(&menuVS);
		}

		GAME_MENU_READY |= VB_MENU_READY_SHOW_MENU;
	}
}

global_variable struct MenuRow rowsVS[5] = {
    // Retry
    {
        .stringIndex = LNG_RETRY,
        .rowOnPressUp = 0,
        .rowOnPressDown = 1,
        .rowOnPressLeft = 0,
        .rowOnPressRight = 0,
    },
    // Change Level
    {
        .stringIndex = LNG_CHANGE_LEVEL,
        .rowOnPressUp = 0,
        .rowOnPressDown = 2,
        .rowOnPressLeft = 1,
        .rowOnPressRight = 1,
    },
    // Change Character
    {
        .stringIndex = LNG_CHANGE_CHARACTER,
        .rowOnPressUp = 1,
        .rowOnPressDown = 3,
        .rowOnPressLeft = 2,
        .rowOnPressRight = 2,
    },
    // Quit
    {
        .stringIndex = LNG_QUIT,
        .rowOnPressUp = 2,
        .rowOnPressDown = 3,
        .rowOnPressLeft = 3,
        .rowOnPressRight = 3,
    },
    // NULL, end of menu
    {
        .stringIndex = RECTMENU_STRING_NONE,
        .rowOnPressUp = 0,
        .rowOnPressDown = 0,
        .rowOnPressLeft = 0,
        .rowOnPressRight = 0,
    }};

global_variable struct RectMenu menuVS = {
    .stringIndexTitle = RECTMENU_STRING_NONE,
    .posX_curr = 143,
    .posY_curr = 162,
    .unk1 = 0,
    .state = RECTMENU_STATE_SMALL_EXEC_CENTERED,
    .rows = rowsVS,
    .funcPtr = UI_RaceEnd_MenuProc,
    .drawStyle = 4,
    // rest of variables all default zero
};

global_variable struct MenuRow rowsBattle[6] = {
    // Retry
    {
        .stringIndex = LNG_RETRY,
        .rowOnPressUp = 0,
        .rowOnPressDown = 1,
        .rowOnPressLeft = 0,
        .rowOnPressRight = 0,
    },
    // Change Level
    {
        .stringIndex = LNG_CHANGE_LEVEL,
        .rowOnPressUp = 0,
        .rowOnPressDown = 2,
        .rowOnPressLeft = 1,
        .rowOnPressRight = 1,
    },
    // Change Setup
    {
        .stringIndex = LNG_CHANGE_SETUP,
        .rowOnPressUp = 1,
        .rowOnPressDown = 3,
        .rowOnPressLeft = 2,
        .rowOnPressRight = 2,
    },
    // Change Character
    {
        .stringIndex = LNG_CHANGE_CHARACTER,
        .rowOnPressUp = 2,
        .rowOnPressDown = 4,
        .rowOnPressLeft = 3,
        .rowOnPressRight = 3,
    },
    // Quit
    {
        .stringIndex = LNG_QUIT,
        .rowOnPressUp = 3,
        .rowOnPressDown = 4,
        .rowOnPressLeft = 4,
        .rowOnPressRight = 4,
    },
    // NULL, end of menu
    {
        .stringIndex = RECTMENU_STRING_NONE,
        .rowOnPressUp = 0,
        .rowOnPressDown = 0,
        .rowOnPressLeft = 0,
        .rowOnPressRight = 0,
    }};

global_variable struct RectMenu menuBattle = {
    .stringIndexTitle = RECTMENU_STRING_NONE,
    .posX_curr = 143,
    .posY_curr = 166,
    .unk1 = 0,
    .state = RECTMENU_STATE_SMALL_EXEC_CENTERED,
    .rows = rowsBattle,
    .funcPtr = UI_RaceEnd_MenuProc,
    .drawStyle = 4,
    // rest of variables all default zero
};

// 2P, 3P, 4P
global_variable s16 s_vsStandingsYByPlayerCount[3][VB_POSY_NUM] = {
    {0x32, 0x5a, 0x82, 0, 0},     // 2P
    {0x1e, 0x46, 0x6e, 0x96, 0},  // 3P
    {0xa, 0x35, 0x5b, 0x81, 0xa7} // 4P
};

global_variable s16 s_standingsSuffixStringIds225[VB_MAX_PLAYERS] = {LNG_ST, LNG_ND, LNG_RD, LNG_TH};

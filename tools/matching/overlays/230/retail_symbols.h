#ifndef CTR_MATCHING_OVERLAY_230_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_230_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

#define MM_GAME_TRACKER_PAGE_VALUE    0x80090000U
#define MM_GAME_TRACKER_PAGE_UPPER_ASM "0x8009"
#define MM_GAME_TRACKER_PAGE_OFFSET   -11604
#define MM_GAME_TRACKER_PAGE_OFFSET_ASM "-11604"
#define MM_OVERLAY_PAGE_VALUE 0x800b0000U
#define MM_OVERLAY_PAGE_UPPER_ASM "0x800b"
#define MM_REQUEST_LEVEL              mm_mainRaceTrackRequestLoad
#define MM_BIGFILE_HEADER             mm_bigfileHeader
#define MM_GAME_UNLOCKS               mm_gameUnlocks
#define MM_CHARACTER_IDS_BACKUP       mm_characterIDsBackup
#define MM_DEFAULT_CHARACTER_ID_WORDS mm_defaultCharacterIDWords
#define MM_TITLE_OBJECT               mm_titleObject
#define MM_TITLE_INTRO_FRAME          mm_titleIntroFrame
#define MM_TITLE_INTRO_FRAME_ASM_NAME "D230+5424"
#define MM_TITLE_INTRO_FRAME_PAGE_OFFSET 0x5a14
#define MM_TITLE_CAMERA_POS           mm_titleCameraPos
#define MM_TITLE_CAMERA_ROT           mm_titleCameraRot
#define MM_TITLE_CAMERA_PATH          mm_titleCameraPath
#define MM_TITLE_SOUNDS               mm_titleSounds
#define MM_TITLE_INSTANCES            mm_titleInstances
#define MM_MENU_MAIN                  mm_menuMain
#define MM_GAME_BUTTON_TAPS           mm_gameButtonTaps
#define MM_TITLE_MENU_STATE           mm_titleMenuState
#define MM_TITLE_OBJECT_NAME          mm_titleObjectName
#define MM_MENU_PLAYERS_1P2P          mm_menuPlayers1P2P
#define MM_MENU_PLAYERS_2P3P4P        mm_menuPlayers2P3P4P
#define MM_MENU_DIFFICULTY             mm_menuDifficulty
#define MM_MENU_RACE_TYPE              mm_menuRaceType
#define MM_MENU_ADVENTURE              mm_menuAdventure
#define MM_MENU_POINTERS               mm_menuPointers
#define MM_MENU_CHARACTER_SELECT       mm_menuCharacterSelect
#define MM_MENU_HIGH_SCORES            mm_menuHighScores
#define MM_MENU_SCRAPBOOK              mm_menuScrapbook
#define MM_TITLE_TRANSITION_DURATION   mm_titleTransitionDuration
#define MM_TITLE_TRANSITION_STEP       mm_titleTransitionStep
#define MM_TITLE_MENU_LAYOUT           mm_titleMenuLayout
#define MM_TITLE_TRANSITIONS           mm_titleTransitions
#define MM_ARCADE_TRACKS               mm_arcadeTracks
#define MM_ARCADE_TRACKS_ASM_NAME      "D230+3788"
#define MM_ARCADE_TRACKS_PAGE_OFFSET   0x53b0
#define MM_DESIRED_MENU_INDEX          mm_desiredMenuIndex
#define MM_TITLE_MENU_TRANSITION_FRAME mm_titleMenuTransitionFrame
#define MM_ADV_PROFILE_INDEX           mm_advProfileIndex
#define MM_MAIN_MENU_STATE             mm_mainMenuState
#define MM_DEMO_MODE_INDEX             mm_demoModeIndex
#define MM_DESIRED_MENU                mm_desiredMenu
#define MM_ACTIVE_MENU                 mm_activeMenu
#define MM_MENU_FOUR_ADV_PROFILES      mm_menuFourAdvProfiles
#define MM_CHEATS                      mm_cheats
#define MM_CHEAT_BUTTON_HISTORY        mm_cheatButtonHistory
#define MM_COLOR_POINTERS              mm_colorPointers
#define MM_COLOR_POINTERS_ASM_NAME     "data+5072"
#define MM_FRAME_COUNTER               mm_frameCounter
#define MM_FRAME_COUNTER_ASM_NAME      "sdata_static+2564"
#define MM_FRAME_COUNTER_PAGE_OFFSET   -9872
#define MM_TRIG_APPROX                 mm_trigApprox
#define MM_ACTIVE_CHARACTER_SELECT_META mm_activeCharacterSelectMeta
#define MM_CHARACTER_METADATA_PAGE_VALUE  0x80080000U
#define MM_CHARACTER_METADATA_PAGE_OFFSET 0x6d84
#define MM_CHARACTER_SELECT_DESIRED_IDS mm_characterSelectDesiredIDs
#define MM_CHARACTER_SELECT_DESIRED_IDS_ASM_NAME "D230+5388"
#define MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS mm_activeCharacterSelectWindowPos
#define MM_ACTIVE_CHARACTER_SELECT_WINDOW_POS_ASM_NAME "D230+5416"
#define MM_CHARACTER_SELECT_TRANSITION_META mm_characterSelectTransitionMeta
#define MM_CHARACTER_SELECT_TRANSITION_META_ASM_NAME "D230+5464"
#define MM_CHARACTER_SELECT_TRANSITION_POINTER_PAGE_VALUE 0x800b0000U
#define MM_CHARACTER_SELECT_WINDOW_WIDTH mm_characterSelectWindowWidth
#define MM_CHARACTER_SELECT_WINDOW_HEIGHT mm_characterSelectWindowHeight
#define MM_CHARACTER_SELECT_NAME_TEXT_Y mm_characterSelectNameTextY
#define MM_CHARACTER_SELECT_CURRENT_IDS mm_characterSelectCurrentIDs
#define MM_CHARACTER_SELECT_CURRENT_IDS_ASM_NAME "D230+5396"
#define MM_CHARACTER_SELECT_DRIVER_POS mm_characterSelectDriverPos
#define MM_CHARACTER_SELECT_DRIVER_POS_ASM_NAME "D230+3708"
#define MM_CHARACTER_SELECT_DRIVER_POS_PAGE_OFFSET 0x5360
#define MM_CHARACTER_SELECT_DRIVER_ROT mm_characterSelectDriverRot
#define MM_CHARACTER_SELECT_DRIVER_ROT_ASM_NAME "D230+3716"
#define MM_CHARACTER_SELECT_DRIVER_ROT_PAGE_OFFSET 0x5368
#define MM_CHARACTER_SELECT_MOVE_TIMERS mm_characterSelectMoveTimers
#define MM_CHARACTER_SELECT_MOVE_FRAMES mm_characterSelectMoveFrames
#define MM_CHARACTER_SELECT_MOVE_DIR mm_characterSelectMoveDir
#define MM_CHARACTER_SELECT_MOVE_DIR_ASM_NAME "D230+5380"
#define MM_CHARACTER_SELECT_SLIDE_DISTANCE mm_characterSelectSlideDistance
#define MM_CHARACTER_SELECT_ANGLE mm_characterSelectAngle
#define MM_CHARACTER_SELECT_ANGLE_ASM_NAME "D230+5404"
#define MM_CHARACTER_SELECT_ROSTER_EXPANDED mm_characterSelectRosterExpanded
#define MM_CHARACTER_SELECT_LAYOUT_INDEX mm_characterSelectLayoutIndex
#define MM_CHARACTER_SELECT_META_1P2P mm_characterSelectMeta1P2P
#define MM_CHARACTER_SELECT_WINDOW_POS_BY_LAYOUT mm_characterSelectWindowPosByLayout
#define MM_CHARACTER_SELECT_META_BY_LAYOUT mm_characterSelectMetaByLayout
#define MM_CHARACTER_SELECT_LAYOUT_WINDOW_W mm_characterSelectLayoutWindowW
#define MM_CHARACTER_SELECT_LAYOUT_WINDOW_H mm_characterSelectLayoutWindowH
#define MM_CHARACTER_SELECT_LAYOUT_DRIVER_POS_Z mm_characterSelectLayoutDriverPosZ
#define MM_CHARACTER_SELECT_LAYOUT_DRIVER_POS_Y mm_characterSelectLayoutDriverPosY
#define MM_CHARACTER_SELECT_LAYOUT_TEXT_Y mm_characterSelectLayoutTextY
#define MM_CHARACTER_SELECT_TRANSITION_BY_PLAYER_COUNT mm_characterSelectTransitionByPlayerCount
#define MM_ROWS_MAIN_WITH_SCRAPBOOK    mm_rowsMainWithScrapbook
#define MM_ROWS_PLAYERS_1P2P           mm_rowsPlayers1P2P
#define MM_ROWS_PLAYERS_2P3P4P         mm_rowsPlayers2P3P4P
#define MM_CUP_DIFFICULTY_FIRST_UNLOCK_BIT mm_cupDifficultyFirstUnlockBit
#define MM_CUP_DIFFICULTY_STRING_INDEX mm_cupDifficultyStringIndex
#define MM_CUP_DIFFICULTY_STRING_INDEX_ASM_NAME "D230+2128"
#define MM_CUP_DIFFICULTY_SPEED        mm_cupDifficultySpeed
#define MM_LANGUAGE_STRINGS_PAGE_OFFSET -0x2788
#define MM_CHARACTER_SELECT_TRANSITION_STATE mm_characterSelectTransitionState
#define MM_CHARACTER_SELECT_FLAGS mm_characterSelectFlags
#define MM_CHARACTER_SELECT_TRANSITION_FRAME mm_characterSelectTransitionFrame
#define MM_CHARACTER_SELECT_MENU_STATE mm_characterSelectMenuState
#define MM_CHARACTER_MENU_ID mm_characterMenuID
#define MM_CHARACTER_SELECT_EXITS_FORWARD mm_characterSelectExitsForward
#define MM_MENU_CUP_SELECT mm_menuCupSelect
#define MM_MENU_TRACK_SELECT mm_menuTrackSelect
#define MM_CHARACTER_SELECT_FALLBACK_1 mm_characterSelectFallback1
#define MM_CHARACTER_SELECT_FALLBACK_2 mm_characterSelectFallback2
#define MM_CHARACTER_SELECT_FALLBACK_1_REPEAT mm_characterSelectFallback1Repeat
#define MM_CHARACTER_SELECT_FALLBACK_2_REPEAT mm_characterSelectFallback2Repeat
#define MM_PLAYER_NUMBER_STRINGS mm_playerNumberStrings
#define MM_CHARACTER_SELECT_OUTLINE_COLOR mm_characterSelectOutlineColor
#define MM_CHARACTER_SELECT_NEUTRAL_COLOR mm_characterSelectNeutralColor
#define MM_CHARACTER_SELECT_CHOSEN_COLOR mm_characterSelectChosenColor
#define MM_CHARACTER_SELECT_BLUE_RECT_COLORS mm_characterSelectBlueRectColors
#define MM_TRACK_VIDEO_PREVIEW_FRAMES mm_trackVideoPreviewFrames
#define MM_TRACK_VIDEO_MEM_ALLOCATED mm_trackVideoMemAllocated
#define MM_TRACK_VIDEO_STATE_CURR mm_trackVideoStateCurr
#define MM_TRACK_VIDEO_STATE_PREV mm_trackVideoStatePrev
#define MM_VIDEO_STR_SRC_X mm_videoStrSrcX
#define MM_VIDEO_STR_DST_X mm_videoStrDstX
#define MM_VIDEO_STR_DST_Y mm_videoStrDstY
#define MM_VIDEO_STR_SRC_RECT_ASM_NAME "sdata_static+2324"
#define MM_VIDEO_STR_SRC_RECT_PAGE_OFFSET -10112
#define MM_VIDEO_COLOR mm_videoColor
#define MM_VIDEO_ST_CD_INTERRUPT (*(u32 *)0x8009ebf8)
#define MM_VIDEO_ST_CD_INTERRUPT_ASM_NAME "0x8009ebf8"
#define MM_VIDEO_ST_CD_INTERRUPT_PAGE_OFFSET -5128
#define MM_VIDEO_LOOP_START_BACKLOC mm_videoLoopStartBackloc
#define MM_VIDEO_LOOP_START_BACKLOC_ASM_NAME "V230+0"
#define MM_VIDEO_LOOP_START_BACKLOC_PAGE_OFFSET 26540
#define MM_VIDEO_LOOP_END_BACKLOC mm_videoLoopEndBackloc
#define MM_VIDEO_LOOP_WRAP_PENDING mm_videoLoopWrapPending
#define MM_VIDEO_LOOP_WRAP_PENDING_ASM_NAME "V230+8"
#define MM_VIDEO_LOOP_WRAP_PENDING_PAGE_OFFSET 26548
#define MM_VIDEO_CD_RETRY_STATE mm_videoCdRetryState
#define MM_VIDEO_FINAL_SLICE_INDEX mm_videoFinalSliceIndex
#define MM_VIDEO_SLICE_INDEX mm_videoSliceIndex
#define MM_VIDEO_DCT_MODE mm_videoDctMode
#define MM_VIDEO_DRAW_NEXT_FRAME mm_videoDrawNextFrame
#define MM_VIDEO_END_OF_STREAM mm_videoEndOfStream
#define MM_VIDEO_DECODE_STATE mm_videoDecodeState
#define MM_VIDEO_DECODE_STATE_ASM_NAME "V230+26"
#define MM_VIDEO_DECODE_STATE_PAGE_OFFSET 26566
#define MM_VIDEO_VLC_BUFFER_INDEX mm_videoVlcBufferIndex
#define MM_VIDEO_VLC_BUFFER_INDEX_ASM_NAME "V230+28"
#define MM_VIDEO_VLC_BUFFER_INDEX_PAGE_OFFSET 26568
#define MM_VIDEO_DCT_OUT_BUFFER_INDEX mm_videoDctOutBufferIndex
#define MM_VIDEO_CD_KICK_STATE mm_videoCdKickState
#define MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT mm_videoStalledBacklocFrames
#define MM_VIDEO_STALL_RECOVERY_FRAME_COUNT mm_videoStallRecoveryFrames
#define MM_VIDEO_STREAM_FRAME_COUNT mm_videoStreamFrameCount
#define MM_VIDEO_SECTOR_FRAME_COUNT mm_videoSectorFrameCount
#define MM_VIDEO_SECTOR_FRAME_COUNT_ASM_NAME "V230+44"
#define MM_VIDEO_SECTOR_FRAME_COUNT_PAGE_OFFSET 26584
#define MM_VIDEO_LAST_SECTOR_FRAME_COUNT mm_videoLastSectorFrameCount
#define MM_VIDEO_LAST_BACKLOC mm_videoLastBackloc
#define MM_VIDEO_UNUSED_0X38 mm_videoUnused0x38
#define MM_VIDEO_FLAGS mm_videoFlags
#define MM_VIDEO_RING_SECTOR_COUNT mm_videoRingSectorCount
#define MM_VIDEO_DCT_OUTPUT_DONE mm_videoDctOutputDone
#define MM_VIDEO_VLC_BUFFER_SIZE mm_videoVlcBufferSize
#define MM_VIDEO_VLC_TABLE mm_videoVlcTable
#define MM_VIDEO_IN_BUFFERS mm_videoInBuffers
#define MM_VIDEO_IN_BUFFERS_ASM_NAME "V230+80"
#define MM_VIDEO_IN_BUFFERS_PAGE_OFFSET 26620
#define MM_VIDEO_DCT_OUT_SLICE_SIZE mm_videoDctOutSliceSize
#define MM_VIDEO_OUT_BUFFERS mm_videoOutBuffers
#define MM_VIDEO_RING_BUFFER mm_videoRingBuffer
#define MM_VIDEO_SLICE mm_videoSlice
#define MM_VIDEO_CD_LOCATION_1 mm_videoCdLocation1
#define MM_VIDEO_CD_LOCATION_2 mm_videoCdLocation2
#define MM_VIDEO_CD_LOCATION_3 mm_videoCdLocation3
#define MM_VIDEO_CD_LOCATION_3_ASM_NAME "V230+128"
#define MM_VIDEO_CD_LOCATION_3_PAGE_OFFSET 26668
#define MM_VIDEO_CD_LOCATION_PTR mm_videoCdLocationPtr
#define MM_VIDEO_SLICE_BUFFER_NAME mm_videoSliceBufferName
#define MM_VIDEO_VLC_BUFFER_NAME mm_videoVlcBufferName
#define MM_VIDEO_RING_BUFFER_NAME mm_videoRingBufferName
#define MM_VIDEO_ALLOC_MEM MEMPACK_AllocMem
#define MM_RECTMENU_DRAW_POLY_GT4 RECTMENU_DrawPolyGT4
#define MM_TRACK_LAP_BOX_OPEN mm_trackLapBoxOpen
#define MM_TRACK_LAP_BOX_OPEN_PAGE_OFFSET 0x59ac
#define MM_TRACK_LAP_BOX_OPEN_PAGE_OFFSET_ASM "22956"
#define MM_TRACK_TRANSITION_STATE mm_trackTransitionState
#define MM_TRACK_TRANSITION_STATE_PAGE_OFFSET 0x59ae
#define MM_TRACK_TRANSITION_FRAME mm_trackTransitionFrame
#define MM_TRACK_TRANSITION_FRAME_ASM_NAME "D230+5326"
#define MM_TRACK_TRANSITION_FRAME_PAGE_OFFSET 0x59b2
#define MM_TRACK_CURRENT_TRACK mm_trackCurrentTrack
#define MM_TRACK_CURRENT_TRACK_ASM_NAME "D230+5316"
#define MM_TRACK_SEL_BACKUP mm_trackSelBackup
#define MM_BATTLE_TRACKS mm_battleTracks
#define MM_BATTLE_TRACKS_PAGE_OFFSET 0x54d0
#define MM_TRACK_CHANGE_FRAMES mm_trackChangeFrames
#define MM_TRACK_CHANGE_FRAMES_ASM_NAME "D230+5312"
#define MM_TRACK_CHANGE_FRAMES_PAGE_OFFSET 0x59a4
#define MM_TRACK_CHANGE_DIRECTION mm_trackChangeDirection
#define MM_TRACK_CHANGE_DIRECTION_ASM_NAME "D230+5318"
#define MM_TRACK_TRANSITION_START_AFTER_EXIT mm_trackTransitionStartAfterExit
#define MM_TRACK_TRANSITION_START_AFTER_EXIT_PAGE_OFFSET 0x59b0
#define MM_TRACK_TRANSITIONS mm_trackTransitions
#define MM_TRACK_TRANSITIONS_ASM_NAME "D230+4188"
#define MM_TRACK_TRANSITIONS_PAGE_OFFSET 0x5540
#define MM_TRACK_TRANSITIONS_PAGE_OFFSET_ASM "0x5540"
#define MM_TRACK_PREVIEW_Y mm_trackPreviewY
#define MM_MENU_LAP_SELECT mm_menuLapSelect
#define MM_MENU_BATTLE_WEAPONS mm_menuBattleWeapons
#define MM_LAP_COUNT_BY_ROW mm_lapCountByRow
#define MM_TIME_TRIAL_STAR_COLORS mm_timeTrialStarColors
#define MM_TIME_TRIAL_STAR_FLAGS mm_timeTrialStarFlags
#define MM_TIME_TRIAL_STAR_FLAGS_ASM_NAME "D230+4324"
#define MM_TRACK_DRAW_MAP_OFFSET mm_trackDrawMapOffset
#define MM_TRACK_DRAW_MAP_OFFSET_ASM_NAME "D230+4328"
#define MM_CUP_TRANSITIONS mm_cupTransitions
#define MM_CUP_TRANSITIONS_ASM_NAME "D230+4364"
#define MM_CUP_STAR_COLORS mm_cupStarColors
#define MM_CUP_STAR_COLORS_ASM_NAME "D230+4424"
#define MM_CUP_STAR_WIN_BITS mm_cupStarWinBits
#define MM_CUP_STAR_WIN_BITS_ASM_NAME "D230+4432"
#define MM_CUP_COLOR mm_cupColor
#define MM_CUP_TRANSITION_STATE mm_cupTransitionState
#define MM_CUP_TRANSITION_START_AFTER_EXIT mm_cupTransitionStartAfterExit
#define MM_CUP_TRANSITION_FRAME mm_cupTransitionFrame
#define MM_BATTLE_TRANSITIONS mm_battleTransitions
#define MM_BATTLE_TRANSITION_STATE mm_battleTransitionState
#define MM_BATTLE_TRANSITION_START_AFTER_EXIT mm_battleTransitionStartAfterExit
#define MM_BATTLE_TRANSITION_FRAME mm_battleTransitionFrame
#define MM_BATTLE_MENU_ARRAY mm_battleMenuArray
#define MM_BATTLE_SETTINGS mm_battleSettings
#define MM_MENU_BATTLE_TYPE mm_menuBattleType
#define MM_MENU_BATTLE_LENGTH_LIFE_TIME mm_menuBattleLengthLifeTime
#define MM_MENU_BATTLE_LENGTH_POINTS mm_menuBattleLengthPoints
#define MM_MENU_BATTLE_LENGTH_TIME_TIME mm_menuBattleLengthTimeTime
#define MM_MENU_BATTLE_LENGTH_LIFE_LIFE mm_menuBattleLengthLifeLife
#define MM_MENU_BATTLE_START_GAME mm_menuBattleStartGame
#define MM_BATTLE_WEAPON_ITEMS mm_battleWeaponItems
#define MM_BATTLE_WEAPON_ITEMS_ASM_NAME "D230+4972"
#define MM_BATTLE_TYPE_MODE_FLAGS mm_battleTypeModeFlags
#define MM_BATTLE_TIME_LIMIT_MINUTES mm_battleTimeLimitMinutes
#define MM_BATTLE_LIFE_TIME_LIMIT_MINUTES mm_battleLifeTimeLimitMinutes
#define MM_BATTLE_LIFE_LIMIT_VALUES mm_battleLifeLimitValues
#define MM_BATTLE_POINT_LIMIT_VALUES mm_battlePointLimitValues
#define MM_BATTLE_WEAPON_ENABLED_COLOR mm_battleWeaponEnabledColor
#define MM_BATTLE_WEAPON_DISABLED_COLOR mm_battleWeaponDisabledColor
#define MM_BATTLE_WEAPON_PANEL_COLOR mm_battleWeaponPanelColor
#define MM_BATTLE_EXPAND_MENU mm_battleExpandMenu
#define MM_BATTLE_EXPAND_MENU_ASM_NAME "sdata_static+1220"
#define MM_BATTLE_ROW_HIGHLIGHTED mm_battleRowHighlighted
#define MM_BATTLE_ROW_HIGHLIGHTED_ASM_NAME "sdata_static+1222"
#define MM_BATTLE_ROW_HIGHLIGHTED_PAGE_OFFSET -11214
#define MM_BATTLE_ROW_HIGHLIGHTED_UPDATE mm_battleRowHighlightedUpdate
#define MM_BATTLE_ROW_HIGHLIGHTED_UNSIGNED mm_battleRowHighlightedUnsigned
#define MM_BATTLE_WEAPON_HIGHLIGHTED mm_battleWeaponHighlighted
#define MM_BATTLE_WEAPON_HIGHLIGHTED_PAGE_OFFSET -11212
#define MM_BATTLE_TEAM_OF_EACH_PLAYER mm_battleTeamOfEachPlayer
#define MM_HIGHSCORE_MENU mm_highScoreMenu
#define MM_HIGHSCORE_TARGET_TRACK mm_highScoreTargetTrack
#define MM_HIGHSCORE_TRANSITION_STATE mm_highScoreTransitionState
#define MM_HIGHSCORE_TRANSITION_STATE_ASM_NAME "D230+5348"
#define MM_HIGHSCORE_TRANSITION_MAIN_FRAME mm_highScoreTransitionMainFrame
#define MM_HIGHSCORE_TRANSITION_TRACK_FRAME mm_highScoreTransitionTrackFrame
#define MM_HIGHSCORE_TRANSITION_ROW_FRAME mm_highScoreTransitionRowFrame
#define MM_HIGHSCORE_TRANSITION_ROW_FRAME_ASM_NAME "D230+5354"
#define MM_HIGHSCORE_TRANSITION_ROW_FRAME_PAGE_OFFSET 0x59ce
#define MM_HIGHSCORE_ACTIVE_HORIZONTAL_MOVE mm_highScoreActiveHorizontalMove
#define MM_HIGHSCORE_PENDING_HORIZONTAL_MOVE mm_highScorePendingHorizontalMove
#define MM_HIGHSCORE_ACTIVE_VERTICAL_MOVE mm_highScoreActiveVerticalMove
#define MM_HIGHSCORE_PENDING_VERTICAL_MOVE mm_highScorePendingVerticalMove
#define MM_HIGHSCORE_TARGET_ROW mm_highScoreTargetRow
#define MM_HIGHSCORE_CURRENT_TRACK mm_highScoreCurrentTrack
#define MM_HIGHSCORE_CURRENT_ROW mm_highScoreCurrentRow
#define MM_HIGHSCORE_ICON_COLOR mm_highScoreIconColor
#define MM_HIGHSCORE_ICON_COLOR_ASM_NAME "D230+5228"
#define MM_HIGHSCORE_TRANSITIONS mm_highScoreTransitions
#define MM_HIGHSCORE_TRANSITIONS_ASM_NAME "D230+5100"
#define MM_HIGHSCORE_GHOST_STAR_COLORS mm_highScoreGhostStarColors
#define MM_HIGHSCORE_GHOST_STAR_COLORS_ASM_NAME "D230+5220"
#define MM_HIGHSCORE_GHOST_STAR_FLAGS mm_highScoreGhostStarFlags
#define MM_HIGHSCORE_GHOST_STAR_FLAGS_ASM_NAME "D230+5224"
#define MM_MENU_GHOST_SELECTION mm_menuGhostSelection
#define MM_MENU_QUEUE_LOAD_TRACK mm_menuQueueLoadTrack
#define MM_LOADED_GHOST_DATA mm_loadedGhostData
#define MM_ALLOC_HIGH_MEM MEMPACK_AllocHighMem
#define MM_LEVEL_METADATA mm_levelMetadata
#define MM_LEVEL_METADATA_ASM_NAME "data+12512"
#define MM_LEVEL_METADATA_PAGE_VALUE 0x80080000U
#define MM_LEVEL_METADATA_PAGE_UPPER_ASM "0x8008"
#define MM_LEVEL_METADATA_PAGE_OFFSET_ASM "0x3a80"
#define MM_ARCADE_CUPS mm_arcadeCups
#define MM_ARCADE_CUPS_ASM_NAME "data+14248"
#define MM_REPLAY_HUMAN_GHOST mm_replayHumanGhost
#define MM_ERROR_MESSAGE_POS_INDEX mm_errorMessagePosIndex
#define MM_GHOST_TAPE_PLAYING mm_ghostTapePlaying
#define MM_USELESS_LAP_ROW_COPY mm_uselessLapRowCopy
#define MM_MENU_ROW_HIGHLIGHT_NORMAL_PAGE_OFFSET -0x26b4
#define MM_DECALHUD_DRAW_POLY_GT4 mm_decalHudDrawPolyGT4
#define MM_DECALHUD_ARROW_2D mm_decalHudArrow2D
#define MM_DRAW_CLEAR_BOX CTR_Box_DrawClearBox
#define MM_DRAW_SOLID_BOX_WITH_PRIM_MEM CTR_Box_DrawSolidBox
#define MM_DRAW_LINE_WIDE_FLAGS mm_drawLineWideFlags

// NOTE(aalhendi): The canonical call grows MM_CupSelect_MenuProc by two
// instructions. This matching-only aggregate reproduces GCC 2.8.1's retail
// outgoing-argument layout; CTR_NATIVE calls RECTMENU_DrawPolyGT4 directly.
struct MMCupSelectDrawTrackArgs
{
	u32 *orderingTable;
	Color color0;
	Color color1;
	Color color2;
	Color color3;
	s32 transparency;
	s32 scale;
};

extern void mm_cupSelectDrawTrack(struct Icon *icon, s32 posX, s32 posY,
	struct PrimMem *primMem, struct MMCupSelectDrawTrackArgs args) asm("RECTMENU_DrawPolyGT4");

#define MM_CUP_SELECT_TRACK_COLOR(color, symbolPage, identity)                                  \
	({                                                                                           \
		CTR_PSX_ADD_SYMBOL_LOW_DISTINCT(color, symbolPage, "D230+4440", &MM_CUP_COLOR, identity); \
		CTR_PSX_RELOAD(*color);                                                                  \
		*color;                                                                                  \
	})
#define MM_CUP_SELECT_FIRST_TRACK_COLOR(color, symbolPage, orderingTable)           \
	({                                                                               \
		__asm__("lui %0,%%hi(D230+4440)" : "=r"(symbolPage) : "0"(orderingTable)); \
		MM_CUP_SELECT_TRACK_COLOR(color, symbolPage, "cup-color-0");                \
	})
#define MM_CUP_SELECT_TRACK_TRANSPARENCY(color, symbolPage) \
	({ __asm__("" : "+r"(symbolPage), "+m"(*color)); 0; })
#define MM_CUP_SELECT_DRAW_TRACK(icon, posX, posY, primMem, ot, colorValue)                      \
	do                                                                                           \
	{                                                                                            \
		register u32 cupColorPage CTR_PSX_REGISTER("$2");                                      \
		register Color *cupColor CTR_PSX_REGISTER("$8");                                      \
		mm_cupSelectDrawTrack(                                                                   \
		    (icon), (posX), (posY), (primMem),                                                   \
		    (struct MMCupSelectDrawTrackArgs){                                                   \
		        (ot),                                                                            \
		        MM_CUP_SELECT_FIRST_TRACK_COLOR(cupColor, cupColorPage, (ot)),                    \
		        MM_CUP_SELECT_TRACK_COLOR(cupColor, cupColorPage, "cup-color-1"),                \
		        MM_CUP_SELECT_TRACK_COLOR(cupColor, cupColorPage, "cup-color-2"),                \
		        MM_CUP_SELECT_TRACK_COLOR(cupColor, cupColorPage, "cup-color-3"),                \
		        MM_CUP_SELECT_TRACK_TRANSPARENCY(cupColor, cupColorPage), FP(0.5)});              \
	} while (0)

// NOTE(aalhendi): Overlay 230 addresses resident EXE state absolutely rather
// than through the executable's gp register. Keep that artifact binding out of
// the shared game ABI.
#define sdata (&sdata_static)

extern u32 mm_gameUnlocks[GAME_PROGRESS_UNLOCK_WORD_COUNT] asm("sdata_static+6016");
extern void mm_mainRaceTrackRequestLoad(s32 levelID) asm("MainRaceTrack_RequestLoad");
extern struct BigHeader *mm_bigfileHeader asm("sdata_static+304");
extern s16 mm_characterIDsBackup[8] asm("sdata_static+55828");
extern const u32 mm_defaultCharacterIDWords[2] asm("R230+36");
extern struct Title *mm_titleObject asm("D230+5308");
extern u16 mm_titleIntroFrame asm("D230+5424");
extern SVec3 mm_titleCameraPos asm("D230+844");
extern SVec3 mm_titleCameraRot asm("D230+852");
extern struct TitleCameraPathFrame *mm_titleCameraPath asm("D230+5456");
extern struct TitleSoundCue mm_titleSounds[TITLE_SOUND_COUNT] asm("D230+992");
extern struct TitleInstanceMeta mm_titleInstances[TITLE_INSTANCE_COUNT] asm("D230+796");
extern struct RectMenu mm_menuMain asm("D230+92");
extern u32 mm_gameButtonTaps[4] asm("sdata_static+55844");
extern s16 mm_titleMenuState asm("D230+5432");
extern const char mm_titleObjectName[8] asm("R230+28");
extern struct RectMenu mm_menuPlayers1P2P asm("D230+156");
extern struct RectMenu mm_menuPlayers2P3P4P asm("D230+224");
extern struct RectMenu mm_menuDifficulty asm("D230+292");
extern struct RectMenu mm_menuRaceType asm("D230+356");
extern struct RectMenu mm_menuAdventure asm("D230+420");
extern struct RectMenu *mm_menuPointers[MM_MENU_RESET_COUNT] asm("D230+760");
extern struct RectMenu mm_menuCharacterSelect asm("D230+464");
extern struct RectMenu mm_menuHighScores asm("D230+672");
extern struct RectMenu mm_menuScrapbook asm("D230+716");
extern s32 mm_titleTransitionDuration asm("D230+860");
extern s16 mm_titleTransitionStep asm("D230+864");
extern struct TitleMenuPositionLayout mm_titleMenuLayout asm("D230+868");
extern struct TransitionMeta mm_titleTransitions[TITLE_TRANSITION_COUNT] asm("D230+896");
extern struct MainMenu_LevelRow mm_arcadeTracks[0x12] asm("D230+3788");
extern s16 mm_desiredMenuIndex asm("D230+5372");
extern s16 mm_titleMenuTransitionFrame asm("D230+5468");
extern s16 mm_advProfileIndex asm("sdata_static+2560");
extern MainMenuState mm_mainMenuState asm("sdata_static+2576");
extern s16 mm_demoModeIndex asm("sdata_static+2580");
extern struct RectMenu *mm_desiredMenu asm("sdata_static+2488");
extern struct RectMenu *mm_activeMenu asm("sdata_static+2460");
extern struct RectMenu mm_menuFourAdvProfiles asm("data+20968");
extern struct MainMenuCheatCode mm_cheats[MM_CHEAT_COUNT] asm("D230+1024");
extern u32 mm_cheatButtonHistory[MM_CHEAT_BUTTON_HISTORY_COUNT] asm("D230+2080");
extern u32 *mm_colorPointers[NUM_COLORS] asm("data+5072");
extern u16 mm_frameCounter asm("sdata_static+2564");
extern struct TrigTable mm_trigApprox[0x400] asm("data+15360");
extern struct CharacterSelectMeta *mm_activeCharacterSelectMeta asm("D230+5428");
extern s16 mm_characterSelectDesiredIDs[4] asm("D230+5388");
extern SVec2 *mm_activeCharacterSelectWindowPos asm("D230+5416");
extern struct TransitionMeta *mm_characterSelectTransitionMeta asm("D230+5464");
extern u16 mm_characterSelectWindowWidth asm("D230+5452");
extern s16 mm_characterSelectWindowHeight asm("D230+5368");
extern s16 mm_characterSelectNameTextY asm("D230+5460");
extern s16 mm_characterSelectCurrentIDs[4] asm("D230+5396");
extern SVec3 mm_characterSelectDriverPos asm("D230+3708");
extern SVec3 mm_characterSelectDriverRot asm("D230+3716");
extern s16 mm_characterSelectMoveTimers[4] asm("D230+5440");
extern s16 mm_characterSelectMoveFrames asm("D230+3722");
extern s16 mm_characterSelectMoveDir[4] asm("D230+5380");
extern s16 mm_characterSelectSlideDistance asm("D230+3724");
extern s16 mm_characterSelectAngle[4] asm("D230+5404");
extern b16 mm_characterSelectRosterExpanded asm("D230+5448");
extern s16 mm_characterSelectLayoutIndex asm("D230+5420");
extern struct CharacterSelectMeta mm_characterSelectMeta1P2P[0xf] asm("D230+2460");
extern SVec2 *mm_characterSelectWindowPosByLayout[6] asm("D230+2196");
extern struct CharacterSelectMeta *mm_characterSelectMetaByLayout[6] asm("D230+3000");
extern s16 mm_characterSelectLayoutWindowW[6] asm("D230+2220");
extern s16 mm_characterSelectLayoutWindowH[6] asm("D230+2232");
extern s16 mm_characterSelectLayoutDriverPosZ[6] asm("D230+2244");
extern s16 mm_characterSelectLayoutDriverPosY[6] asm("D230+2256");
extern s16 mm_characterSelectLayoutTextY[6] asm("D230+2268");
extern struct TransitionMeta *mm_characterSelectTransitionByPlayerCount[4] asm("D230+3692");
extern u16 mm_characterSelectFlags asm("sdata_static+2504");
extern s16 mm_characterSelectTransitionFrame asm("D230+5472");
extern s16 mm_characterSelectMenuState asm("D230+5436");
extern s16 mm_characterMenuID[0x10] asm("D230+3024");
extern s16 mm_characterSelectExitsForward asm("D230+5376");
extern struct RectMenu mm_menuCupSelect asm("D230+584");
extern struct RectMenu mm_menuTrackSelect asm("D230+508");
extern u8 mm_characterSelectFallback1[CHARACTER_SELECT_DIRECTION_COUNT] asm("D230+3744");
extern u8 mm_characterSelectFallback2[CHARACTER_SELECT_DIRECTION_COUNT] asm("D230+3748");
// NOTE(aalhendi): Distinct C identities at the same retail addresses preserve
// the repeated address materializations in MM_Characters_MenuProc.
extern u8 mm_characterSelectFallback1Repeat[CHARACTER_SELECT_DIRECTION_COUNT] asm("D230+3744");
extern u8 mm_characterSelectFallback2Repeat[CHARACTER_SELECT_DIRECTION_COUNT] asm("D230+3748");
extern char *mm_playerNumberStrings[4] asm("D230+3728");
extern Color mm_characterSelectOutlineColor asm("D230+3752");
extern Color mm_characterSelectNeutralColor asm("D230+3756");
extern Color mm_characterSelectChosenColor asm("D230+3760");
extern char mm_characterSelectBlueRectColors[0x18] asm("D230+3764");
extern s16 mm_trackVideoPreviewFrames asm("D230+5328");
extern s16 mm_trackVideoMemAllocated asm("D230+5330");
extern s16 mm_trackVideoStateCurr asm("D230+5332");
extern s16 mm_trackVideoStatePrev asm("D230+5334");
extern s16 mm_videoStrSrcX asm("sdata_static+2324");
extern s16 mm_videoStrDstX asm("sdata_static+2352");
extern s16 mm_videoStrDstY asm("sdata_static+2356");
extern Color mm_videoColor asm("D230+4316");
extern s32 mm_videoLoopStartBackloc asm("V230+0");
extern s32 mm_videoLoopEndBackloc asm("V230+4");
extern s32 mm_videoLoopWrapPending asm("V230+8");
extern s32 mm_videoCdRetryState asm("V230+12");
extern u16 mm_videoFinalSliceIndex asm("V230+16");
extern u16 mm_videoSliceIndex asm("V230+18");
extern u16 mm_videoDctMode asm("V230+20");
extern u16 mm_videoDrawNextFrame asm("V230+22");
extern u16 mm_videoEndOfStream asm("V230+24");
extern s16 mm_videoDecodeState asm("V230+26");
extern u16 mm_videoVlcBufferIndex asm("V230+28");
extern u16 mm_videoDctOutBufferIndex asm("V230+30");
extern u16 mm_videoCdKickState asm("V230+32");
extern s16 mm_videoStalledBacklocFrames asm("V230+34");
extern u16 mm_videoStallRecoveryFrames asm("V230+36");
extern s32 mm_videoStreamFrameCount asm("V230+40");
extern s32 mm_videoSectorFrameCount asm("V230+44");
extern s32 mm_videoLastSectorFrameCount asm("V230+48");
extern s32 mm_videoLastBackloc asm("V230+52");
extern s32 mm_videoUnused0x38 asm("V230+56");
extern u32 mm_videoFlags asm("V230+60");
extern s32 mm_videoRingSectorCount asm("V230+64");
extern volatile u32 mm_videoDctOutputDone asm("V230+68");
extern s32 mm_videoVlcBufferSize asm("V230+72");
extern void *mm_videoVlcTable asm("sdata_static+1168");
extern u32 *mm_videoInBuffers[2] asm("V230+80");
extern s32 mm_videoDctOutSliceSize asm("V230+88");
extern u32 *mm_videoOutBuffers[4] asm("V230+96");
extern u32 *mm_videoRingBuffer asm("V230+104");
extern RECT mm_videoSlice asm("V230+112");
extern CdlLOC mm_videoCdLocation1 asm("V230+120");
extern CdlLOC mm_videoCdLocation2 asm("V230+124");
extern CdlLOC mm_videoCdLocation3 asm("V230+128");
extern CdlLOC *mm_videoCdLocationPtr asm("V230+132");
extern const char mm_videoSliceBufferName[0xc] asm("D230+5476");
extern const char mm_videoVlcBufferName[8] asm("D230+5488");
extern const char mm_videoRingBufferName[8] asm("D230+5496");
extern s16 mm_trackLapBoxOpen asm("D230+5320");
extern s16 mm_trackTransitionState asm("D230+5322");
extern s16 mm_trackTransitionFrame asm("D230+5326");
extern s16 mm_trackCurrentTrack asm("D230+5316");
extern s16 mm_trackSelBackup asm("sdata_static+2500");
extern struct MainMenu_LevelRow mm_battleTracks[7] asm("D230+4076");
extern s32 mm_trackChangeFrames asm("D230+5312");
extern s16 mm_trackChangeDirection asm("D230+5318");
extern s16 mm_trackTransitionStartAfterExit asm("D230+5324");
extern struct TransitionMeta mm_trackTransitions[5] asm("D230+4188");
extern u16 mm_trackPreviewY asm("D230+4206");
extern struct RectMenu mm_menuLapSelect asm("D230+4272");
extern struct RectMenu mm_menuBattleWeapons asm("D230+628");
extern struct LapCountMenuRow mm_lapCountByRow[4] asm("D230+4240");
extern u16 mm_timeTrialStarColors[2] asm("D230+4320");
extern u16 mm_timeTrialStarFlags[2] asm("D230+4324");
extern struct TrackSelectMapOffset mm_trackDrawMapOffset[6] asm("D230+4328");
extern struct TransitionMeta mm_cupTransitions[6] asm("D230+4364");
extern u16 mm_cupStarColors[4] asm("D230+4424");
extern s16 mm_cupStarWinBits[4] asm("D230+4432");
extern Color mm_cupColor asm("D230+4440");
extern s16 mm_cupTransitionState asm("D230+5336");
extern s16 mm_cupTransitionStartAfterExit asm("D230+5338");
extern s16 mm_cupTransitionFrame asm("D230+5340");
extern struct TransitionMeta mm_battleTransitions[11] asm("D230+4444");
extern s16 mm_battleTransitionState asm("D230+5342");
extern s16 mm_battleTransitionStartAfterExit asm("D230+5344");
extern s16 mm_battleTransitionFrame asm("D230+5346");
extern struct RectMenu *mm_battleMenuArray[5] asm("D230+4952");
extern s16 mm_battleSettings[6] asm("sdata_static+55816");
extern struct RectMenu mm_menuBattleType asm("D230+4580");
extern struct RectMenu mm_menuBattleLengthLifeTime asm("D230+4648");
extern struct RectMenu mm_menuBattleLengthPoints asm("D230+4784");
extern struct RectMenu mm_menuBattleLengthTimeTime asm("D230+4716");
extern struct RectMenu mm_menuBattleLengthLifeLife asm("D230+4852");
extern struct RectMenu mm_menuBattleStartGame asm("D230+4908");
extern struct BattleWeaponMenuItem mm_battleWeaponItems[11] asm("D230+4972");
extern u32 mm_battleTypeModeFlags[3] asm("D230+5060");
extern u8 mm_battleTimeLimitMinutes[4] asm("D230+5072");
extern s8 mm_battleLifeTimeLimitMinutes[4] asm("D230+5076");
extern u8 mm_battleLifeLimitValues[4] asm("D230+5080");
extern u8 mm_battlePointLimitValues[4] asm("D230+5084");
extern Color mm_battleWeaponEnabledColor asm("D230+5088");
extern Color mm_battleWeaponDisabledColor asm("D230+5092");
extern Color mm_battleWeaponPanelColor asm("D230+5096");
extern s16 mm_battleExpandMenu asm("sdata_static+1220");
extern s16 mm_battleRowHighlighted asm("sdata_static+1222");
extern s16 mm_battleRowHighlightedUpdate asm("sdata_static+1222");
extern u16 mm_battleRowHighlightedUnsigned asm("sdata_static+1222");
extern s16 mm_battleWeaponHighlighted asm("sdata_static+1224");
extern u8 mm_battleTeamOfEachPlayer[4] asm("sdata_static+2548");
extern struct RectMenu mm_highScoreMenu asm("D230+5256");
extern s16 mm_highScoreTargetTrack asm("D230+5300");
extern s16 mm_highScoreTransitionState asm("D230+5348");
extern s16 mm_highScoreTransitionMainFrame asm("D230+5350");
extern s16 mm_highScoreTransitionTrackFrame asm("D230+5352");
extern s16 mm_highScoreTransitionRowFrame asm("D230+5354");
extern s16 mm_highScoreActiveHorizontalMove asm("D230+5356");
extern s16 mm_highScorePendingHorizontalMove asm("D230+5358");
extern s16 mm_highScoreActiveVerticalMove asm("D230+5360");
extern s16 mm_highScorePendingVerticalMove asm("D230+5362");
extern s16 mm_highScoreTargetRow asm("D230+5302");
extern s16 mm_highScoreCurrentTrack asm("D230+5304");
extern s16 mm_highScoreCurrentRow asm("D230+5306");
extern Color mm_highScoreIconColor asm("D230+5228");
extern struct TransitionMeta mm_highScoreTransitions[0xc] asm("D230+5100");
extern u16 mm_highScoreGhostStarColors[2] asm("D230+5220");
extern u16 mm_highScoreGhostStarFlags[2] asm("D230+5224");
extern struct RectMenu mm_menuGhostSelection asm("data+21012");
extern struct RectMenu mm_menuQueueLoadTrack asm("data+20724");
extern char mm_loadedGhostData[0x18] asm("R230+84");
extern struct MetaDataLEV mm_levelMetadata[0x41] asm("data+12512");
extern __typeof__(data.ArcadeCups) mm_arcadeCups asm("data+14248");
extern b16 mm_replayHumanGhost asm("sdata_static+2540");
extern s16 mm_errorMessagePosIndex asm("sdata_static+2336");
extern struct GhostHeader *mm_ghostTapePlaying asm("sdata_static+2024");
extern u16 mm_uselessLapRowCopy asm("sdata_static+2484");
// NOTE(aalhendi): These retail call sites prepare full-width scalar arguments.
// The callee consumes their low 16 bits; this alias prevents caller-side
// truncation without changing DecalFont_DrawLine's canonical declaration.
extern void mm_drawLineWideFlags(char *text, s32 posX, s32 posY, s32 fontType,
                                 s32 flags) asm("DecalFont_DrawLine");
extern void mm_decalHudDrawPolyGT4(struct Icon *icon, s32 posX, s32 posY,
                                  struct PrimMem *primMem, u32 *ot,
                                  Color color0, Color color1, Color color2, Color color3,
                                  s32 transparency, s32 scale) asm("DecalHUD_DrawPolyGT4");
extern void mm_decalHudArrow2D(struct Icon *icon, s32 posX, s32 posY,
                              struct PrimMem *primMem, u32 *ot,
                              Color color0, Color color1, Color color2, Color color3,
                              s32 transparency, s32 scale, u16 rotation) asm("DecalHUD_Arrow2D");
extern struct MenuRow mm_rowsMainWithScrapbook[8] asm("D230+44");
extern struct MenuRow mm_rowsPlayers1P2P[MM_PLAYER_1P2P_SELECTABLE_ROWS + 1] asm("D230+136");
extern struct MenuRow mm_rowsPlayers2P3P4P[MM_PLAYER_2P3P4P_SELECTABLE_ROWS + 1] asm("D230+200");
extern s16 mm_cupDifficultyFirstUnlockBit[4] asm("D230+2120");
extern s16 mm_cupDifficultyStringIndex[4] asm("D230+2128");
extern s16 mm_cupDifficultySpeed[4] asm("D230+2136");
extern s16 mm_characterSelectTransitionState asm("D230+5412");

#endif

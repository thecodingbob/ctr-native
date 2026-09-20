#ifndef CTR_MATCHING_OVERLAY_232_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_232_RETAIL_SYMBOLS_H

#define AH_MASK_AUDIO_BACKUP       ah_maskAudioBackup
#define AH_MASK_AUDIO_TARGET       ah_maskAudioTarget
#define AH_HINT_MASK               ah_hintMask
#define AH_MASK_ROT                ah_maskRot
#define AH_MASK_POS                ah_maskPos
#define AH_MASK_SCALE              ah_maskScale
#define AH_FIVE_ARROW_COLOR_1      ah_fiveArrowColors
#define AH_FIVE_ARROW_COLOR_2      (ah_fiveArrowColors + 3)
#define AH_FRAME_COUNTER           ah_frameCounter
#define AH_PAUSE_OBJECT            ah_pauseObject
#define AH_PAUSE_STORAGE           ah_pauseStorage
#define AH_PAUSE_DIRECTION         ah_pauseDirection
#define AH_PAUSE_TIMER             ah_pauseTimer
#define AH_PAUSE_PREVIOUS_PAGE     ah_pausePreviousPage
#define AH_PAUSE_CURRENT_PAGE      ah_pauseCurrentPage
#define AH_PAUSE_FLIP_DIRECTION    ah_pauseFlipDirection
#define AH_PAUSE_BUTTON_TAP        ah_pauseButtonTap
#define AH_PAUSE_THREAD_NAME       ah_pauseThreadName
#define AH_PAUSE_INSTANCE_NAME     ah_pauseInstanceName
#define AH_PAUSE_TOTAL_FORMAT      ah_pauseTotalFormat
#define AH_PAUSE_PAGES             ah_pausePages
#define AH_PAUSE_ICONS             ah_pauseIcons
#define AH_PAUSE_X_GLYPH           ah_pauseXGlyph
#define AH_COLOR_POINTERS          ah_colorPointers
#define AH_DECALHUD_ARROW_2D       ah_decalHudArrow2D
#define AH_DECALHUD_DRAW_POLY_GT4  ah_decalHudDrawPolyGT4
#define AH_TRIG_TABLE              ah_trigTable
#define AH_ARROW_PRIM_OFFSETS      ah_arrowPrimOffsets
#define AH_TRIANGLE_COLOR          ah_triangleColor
#define AH_SAVE_PRIM_OFFSETS       ah_savePrimOffsets
#define AH_QUAD_COLOR              ah_quadColor
#define AH_HINT_VISIBLE            ah_hintVisible
#define AH_HINT_ID                 ah_hintID
#define AH_HINT_INTERRUPTS_WARPPAD ah_hintInterruptsWarppad
#define AH_HINT_MODEL              ah_hintModel
#define AH_MASK_SPAWN_FRAME        ah_maskSpawnFrame
#define AH_MASK_OFFSET_POS         ah_maskOffsetPos
#define AH_MASK_OFFSET_ROT         ah_maskOffsetRot
#define AH_MASK_HINT_POSITIONS     ah_maskHintPositions
#define AH_MASK_HINT_ROTATIONS     ah_maskHintRotations
#define AH_ACTIVE_MENU             ah_activeMenu
#define AH_SAVE_MENU               ah_saveMenu
#define AH_SAVE_CAMERA_ROT         ah_saveCameraRot
#define AH_SAVE_OBJECT_NAME        ah_saveObjectName
#define AH_SCAN_NAME               ah_scanName
#define AH_GARAGE_NAME             ah_garageName
#define AH_GARAGE_TOP_NAME         ah_garageTopName
#define AH_HUB_TRACK_IDS           ah_hubTrackIDs
#define AH_BOSS_REWARDS            ah_bossRewards
#define AH_DOOR_NAME               ah_doorName
#define AH_KEY_NAME                ah_keyName
#define AH_STORY_REWARDS           ah_storyRewards
#define AH_MAP_ARROW_DRAWN         ah_mapArrowDrawn
#define AH_HUD_FLAGS               ah_hudFlags
#define AH_HUD_LAYOUTS             ah_hudLayouts
#define AH_MASK_CAM_POS_START      ah_maskCamPosStart
#define AH_MASK_CAM_ROT_START      ah_maskCamRotStart
#define AH_MASK_FRAME_CURR         ah_maskFrameCurr
#define AH_MASK_DELAY              ah_maskDelay
#define AH_MASK_XA_STATE           ah_maskXAState
#define AH_MASK_EYE                ah_maskEye
#define AH_MASK_LOOK               ah_maskLook
#define AH_MASK_SPAWN_EMITTERS     ah_maskSpawnEmitters
#define AH_MASK_LEAVE_EMITTERS     ah_maskLeaveEmitters
#define AH_MASK_HINT_INDICES       ah_maskHintIndices
#define AH_MASK_ANGLE              ah_maskAngle
#define AH_MAP_HUB_ITEMS           ah_mapHubItems
#define AH_MAP_SAVE_POS            ah_mapSavePos
#define AH_MAP_SAVE_COLORS         ah_mapSaveColors
#define AH_MAP_ARROW_POS           ah_mapArrowPos
#define AH_MAP_ARROW_COLORS        ah_mapArrowColors
#define AH_MAP_INNER_OFFSETS       ah_mapInnerOffsets
#define AH_MAP_OUTER_OFFSETS       ah_mapOuterOffsets
#define AH_HINT_MENU_SCROLL        ah_hintMenuScroll
#define AH_HINT_MENU_VIEW          ah_hintMenuView
#define AH_MASK_COOLDOWN           ah_maskCooldown
#define AH_LOAD_IN_PROGRESS        ah_loadInProgress
#define AH_MENU_EDGE_COLOR         ah_menuEdgeColor
#define AH_HINT_STATE              ah_hintState
#define AH_BOSS_CHALLENGE_TEXT     ah_bossChallengeText
#define AH_LEVEL_METADATA          ah_levelMetadata
#define AH_WARP_CUPS               ah_warpCups
#define AH_HUB_REQUIRED_KEYS       ah_hubRequiredKeys
#define AH_WARP_LIGHT_GEM          ah_warpLightGem
#define AH_WARP_LIGHT_RELIC        ah_warpLightRelic
#define AH_WARP_LIGHT_TOKEN        ah_warpLightToken
#define AH_WARP_BATTLE_OFFSETS     ah_warpBattleOffsets
#define AH_WARP_NEAREST_ID         ah_warpNearestID
#define AH_KART_SPAWN_ORDER        ah_kartSpawnOrder
#define AH_ADVENTURE_RNG           ah_adventureRng
#define AH_WARP_MENU_OPENED        ah_warpMenuOpened
#define AH_WARP_MENU               ah_warpMenu
#define AH_WARP_BATTLE_TIMES       ah_warpBattleTimes
#define AH_WARP_CUP_TRACKS         ah_warpCupTracks

#include "../../retail_bindings.h"

// NOTE(aalhendi): Derive ordinary views from the shared fields. Explicit types
// below join the arrow colors, narrow the frame counter, or adapt HUD colors.
extern __typeof__(data.AdvCups) ah_warpCups asm("data+14196");
extern __typeof__(D232.keysNeededByHub) ah_hubRequiredKeys asm("D232+160");
extern __typeof__(D232.lightDirGem) ah_warpLightGem asm("D232");
extern __typeof__(D232.lightDirRelic) ah_warpLightRelic asm("D232+32");
extern __typeof__(D232.lightDirToken) ah_warpLightToken asm("D232+64");
extern __typeof__(R232.battleTrackPurpleTokenOffset) ah_warpBattleOffsets asm("R232");
extern __typeof__(D232.levelID) ah_warpNearestID asm("D232+170");
extern __typeof__(sdata->kartSpawnOrderArray) ah_kartSpawnOrder asm("sdata_static+1840");
extern __typeof__(sdata->advRng) ah_adventureRng asm("sdata_static+1788");
extern __typeof__(sdata->boolOpenTokenRelicMenu) ah_warpMenuOpened asm("sdata_static+1348");
extern __typeof__(D232.menuTokenRelic) ah_warpMenu asm("D232+116");
extern __typeof__(D232.battleCrystalEventTime) ah_warpBattleTimes asm("D232+172");
extern __typeof__(data.advCupTrackIDs) ah_warpCupTracks asm("data+14084");

extern __typeof__(D232.audioBackup) ah_maskAudioBackup asm("D232+2188");
extern __typeof__(D232.maskAudioTargetVolume) ah_maskAudioTarget asm("D232+1808");
extern __typeof__(sdata->instMaskHints3D) ah_hintMask asm("sdata_static+2292");
extern __typeof__(D232.maskRot) ah_maskRot asm("D232+1032");
extern __typeof__(D232.maskPos) ah_maskPos asm("D232+1024");
extern __typeof__(D232.maskScale) ah_maskScale asm("D232+1040");
extern u32 ah_fiveArrowColors[6] asm("D232+1000");
extern u16 ah_frameCounter asm("sdata_static+2564");
extern __typeof__(D232.ptrPauseObject) ah_pauseObject asm("D232+1948");
extern __typeof__(D232.pauseObject) ah_pauseStorage asm("D232+1952");
extern __typeof__(D232.pausePageDir) ah_pauseDirection asm("D232+1904");
extern __typeof__(D232.pausePageTimer) ah_pauseTimer asm("D232+1906");
extern __typeof__(D232.pausePagePrev) ah_pausePreviousPage asm("D232+1908");
extern __typeof__(D232.pausePageCurr) ah_pauseCurrentPage asm("D232+1910");
extern __typeof__(D232.pausePageDir_dup) ah_pauseFlipDirection asm("D232+1912");
extern __typeof__(sdata->buttonTapPerPlayer[0]) ah_pauseButtonTap asm("sdata_static+55844");
extern __typeof__(R232.s_PAUSE) ah_pauseThreadName asm("R232+144");
extern __typeof__(R232.s_pause) ah_pauseInstanceName asm("R232+152");
extern __typeof__(R232.s_format) ah_pauseTotalFormat asm("R232+136");
extern __typeof__(D232.advPausePages) ah_pausePages asm("D232+588");
extern __typeof__(D232.advPauseInst) ah_pauseIcons asm("D232+644");
extern __typeof__(R232.s_x) ah_pauseXGlyph asm("R232+132");
extern __typeof__(data.ptrColor) ah_colorPointers asm("data+5072");
extern void ah_decalHudArrow2D(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, Color color0, Color color1, Color color2, Color color3,
                               s32 transparency, s32 scale, u16 rotation) asm("DecalHUD_Arrow2D");
extern void ah_decalHudDrawPolyGT4(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, Color color0, Color color1, Color color2,
                                   Color color3, s32 transparency, s32 scale) asm("DecalHUD_DrawPolyGT4");
extern __typeof__(data.trigApprox) ah_trigTable asm("data+15360");
extern __typeof__(D232.hubArrowPrimOffset) ah_arrowPrimOffsets asm("D232+228");
extern __typeof__(D232.colorTri) ah_triangleColor asm("D232+1892");
extern __typeof__(D232.loadSavePrimOffset) ah_savePrimOffsets asm("D232+208");
extern __typeof__(D232.colorQuad) ah_quadColor asm("D232+1876");
extern __typeof__(sdata->boolDraw3D_AdvMask) ah_hintVisible asm("sdata_static+2332");
extern __typeof__(D232.maskHintID) ah_hintID asm("D232+1916");
extern __typeof__(D232.maskWarppadBoolInterrupt) ah_hintInterruptsWarppad asm("D232+1944");
extern __typeof__(sdata->modelMaskHints3D) ah_hintModel asm("sdata_static+388");
extern __typeof__(D232.maskSpawnFrame) ah_maskSpawnFrame asm("D232+2192");
extern __typeof__(D232.maskOffsetPos) ah_maskOffsetPos asm("D232+1044");
extern __typeof__(D232.maskOffsetRot) ah_maskOffsetRot asm("D232+1052");
extern __typeof__(D232.maskHintOffsets.pos) ah_maskHintPositions asm("D232+1060");
extern __typeof__(D232.maskHintOffsets.rot) ah_maskHintRotations asm("D232+1072");
extern __typeof__(sdata->ptrActiveMenu) ah_activeMenu asm("sdata_static+2460");
extern __typeof__(data.menuGreenLoadSave) ah_saveMenu asm("data+20924");
extern __typeof__(D232.saveObjCameraOffset) ah_saveCameraRot asm("D232+200");
extern __typeof__(R232.s_saveobj) ah_saveObjectName asm("R232+60");
extern __typeof__(R232.s_scan) ah_scanName asm("R232+68");
extern __typeof__(R232.s_garage) ah_garageName asm("R232+40");
extern __typeof__(R232.s_garagetop) ah_garageTopName asm("R232+48");
extern __typeof__(data.advHubTrackIDs) ah_hubTrackIDs asm("data+14164");
extern __typeof__(data.BeatBossPrize) ah_bossRewards asm("data+14072");
extern __typeof__(R232.s_door) ah_doorName asm("R232+104");
extern __typeof__(R232.s_key) ah_keyName asm("R232+76");
extern __typeof__(GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_STORY]) ah_storyRewards asm("sdata_static+11332");
extern __typeof__(D232.mapPriorityArrowDrawn) ah_mapArrowDrawn asm("D232+2196");
extern __typeof__(sdata->HudAndDebugFlags) ah_hudFlags asm("sdata_static+148");
extern __typeof__(data.hudStructPtr) ah_hudLayouts asm("data+22716");
extern __typeof__(D232.maskCamPosStart) ah_maskCamPosStart asm("D232+1924");
extern __typeof__(D232.maskCamRotStart) ah_maskCamRotStart asm("D232+1932");
extern __typeof__(D232.maskFrameCurr) ah_maskFrameCurr asm("D232+1084");
extern __typeof__(D232.maskWarppadDelayFrames) ah_maskDelay asm("D232+1940");
extern __typeof__(sdata->XA_State) ah_maskXAState asm("sdata_static+1948");
extern __typeof__(D232.eyePos) ah_maskEye asm("D232+1860");
extern __typeof__(D232.lookAtPos) ah_maskLook asm("D232+1868");
extern __typeof__(D232.emSet_maskSpawn) ah_maskSpawnEmitters asm("D232+1088");
extern __typeof__(D232.emSet_maskLeave) ah_maskLeaveEmitters asm("D232+1448");
extern __typeof__(D232.hintMenuLngIndex) ah_maskHintIndices asm("D232+1816");
extern __typeof__(D232.maskAngle) ah_maskAngle asm("D232+1920");
extern __typeof__(D232.hubItemsXY_ptrArray) ah_mapHubItems asm("D232+448");
extern __typeof__(D232.loadSavePos) ah_mapSavePos asm("D232+496");
extern __typeof__(D232.loadSave_col) ah_mapSaveColors asm("D232+512");
extern __typeof__(D232.hubArrowPos) ah_mapArrowPos asm("D232+528");
extern __typeof__(D232.hubArrowColors) ah_mapArrowColors asm("D232+540");
extern __typeof__(D232.hubArrowInnerOffset) ah_mapInnerOffsets asm("D232+468");
extern __typeof__(D232.hubArrowOuterOffset) ah_mapOuterOffsets asm("D232+480");
extern __typeof__(D232.hintMenu_scrollIndex) ah_hintMenuScroll asm("D232+2184");
extern __typeof__(D232.hintMenu_boolViewHint) ah_hintMenuView asm("D232+2180");
extern __typeof__(D232.maskCooldown) ah_maskCooldown asm("D232+1042");
extern __typeof__(sdata->load_inProgress) ah_loadInProgress asm("sdata_static+312");
extern Color ah_menuEdgeColor asm("sdata_static+1228");
extern __typeof__(sdata->AkuAkuHintState) ah_hintState asm("sdata_static+2312");
extern __typeof__(data.lng_challenge) ah_bossChallengeText asm("data+14236");
extern __typeof__(data.metaDataLEV) ah_levelMetadata asm("data+12512");

// NOTE(aalhendi): Adventure Hub code addresses resident state absolutely.
#define sdata (&sdata_static)

#endif

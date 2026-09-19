#ifndef CTR_MATCHING_OVERLAY_233_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_233_RETAIL_SYMBOLS_H

#define CS_VERT_SPLIT             cs_VertSplitLine
#define CS_LOAD_NEXT_SWAP         cs_boolLoadNextSwap
#define CS_CAN_SKIP               cs_boolStartToSkip
#define CS_BOSS_INDEX             cs_bossCutsceneIndex
#define CS_CONTROLS_AUDIO         cs_CutsceneManipulatesAudio
#define CS_MATRIX_TABLE           cs_cs_initMatrixTable
#define CS_MATRIX_INITIALIZED     cs_cs_initMatrixBool
#define CS_FINISHED               cs_isCutsceneOver
#define CS_PODIUM_CAMERA_FRAME    cs_podiumCameraFrame
#define CS_VOLUME_BACKUP          cs_volumeBackup
#define CS_PRIZE_DROP_READY       cs_podiumPrizeDropReady
#define CS_PHASE                  cs_cutsceneState
#define CS_BOSS_MODELS            cs_bossModels
#define CS_BIGFILE_HEADER         cs_bigfileHeader
#define CS_LOAD_IN_PROGRESS       cs_loadInProgress
#define CS_MAIN_MENU_STATE        cs_mainMenuState
#define CS_XA_STATE               cs_xaState
#define CS_XA_OFFSET              cs_xaOffset
#define CS_QUEUE_READY            cs_queueReady
#define CS_QUEUE_LENGTH           cs_queueLength
#define CS_GARAGE_CURRENT         cs_garageCurrent
#define CS_GARAGE_PREVIOUS        cs_garagePrevious
#define CS_FRAME_COUNTER_LOW      cs_frameCounterLow
#define CS_BUTTONS_HOLD           cs_buttonsHold
#define CS_DESIRED_MENU           cs_desiredMenu

#define CS_OPCODE_META_TABLE      cs_opcodeMetaTable
#define CS_INTRO_MODEL_SCRIPTS    cs_introModelScripts
#define CS_BOX_MODEL_SCRIPTS      cs_boxModelScripts
#define CS_SCRIPT(field)          cs_##field

#define CS_PODIUM_FIRST_NAME      cs_podiumFirstName
#define CS_PODIUM_TAWNA_NAME      cs_podiumTawnaName
#define CS_PODIUM_PRIZE_NAME      cs_podiumPrizeName
#define CS_PODIUM_VICTORYCAM_NAME cs_podiumVictoryCamName

#define CS_INTRO_NAME(field)      cs_intro_##field

#define CS_CREDITS_THREAD         cs_creditsThread
#define CS_CREDITS_DANCER_THREAD  cs_dancerThread
#define CS_CREDITS_DANCER         cs_dancer
#define CS_CREDITS_STRING_COUNT   cs_creditsStringCount
#define CS_CREDITS_STRINGS        cs_creditsStrings
#define CS_CREDITS_ALL_BLUE       cs_creditsAllBlue
#define CS_CREDITS_GHOST_POS      cs_creditsGhostPos
#define CS_CREDITS_TEXT_X         cs_creditsTextX

#define CS_COLOR_POINTERS         cs_colorPointers
#define CS_CREDITS_FADE_PALETTE   cs_creditsFadePalette

#define CS_GARAGE_ZOOM_FRAMES     cs_garageZoomFrames
#define CS_GARAGE_MOVE_FRAME      cs_garageMoveFrame
#define CS_GARAGE_ZOOM_IN         cs_garageZoomIn
#define CS_GARAGE_ZOOM_OUT        cs_garageZoomOut
#define CS_GARAGE_SELECTED        cs_garageSelected
#define CS_GARAGE_DELAY           cs_garageDelay
#define CS_GARAGE_CHARACTERS      cs_garageCharacters
#define CS_GARAGE_STAT_LENGTHS    cs_garageStatLengths
#define CS_GARAGE_UNUSED_FRAMES   cs_garageUnusedFrames
#define CS_GARAGE_CLASS_STRINGS   cs_garageClassStrings
#define CS_GARAGE_STAT_TARGETS    cs_garageStatTargets
#define CS_GARAGE_STAT_COLORS     cs_garageStatColors
#define CS_GARAGE_MOVE_FRAMES     cs_garageMoveFrames
#define CS_GARAGE_FOV_MIN         cs_garageFovMin
#define CS_GARAGE_FOV_MAX         cs_garageFovMax
#define CS_DECALHUD_ARROW_2D      cs_decalHudArrow2D

#include "../../retail_bindings.h"

extern const u8 cs_opcodeMetaTable[256] asm("R233+23260");
extern __typeof__(D233.introModelScripts) cs_introModelScripts asm("D233+14848");
extern __typeof__(D233.boxModelScripts) cs_boxModelScripts asm("D233+20224");

extern __typeof__(D233.creditsOpcodeData) cs_creditsOpcodeData asm("D233+15868");
extern __typeof__(D233.introEndingOpcodeData) cs_introEndingOpcodeData asm("D233+15232");
extern __typeof__(D233.introCutsceneOpcodes) cs_introCutsceneOpcodes asm("D233+15196");
extern __typeof__(D233.creditsCutsceneOpcodes) cs_creditsCutsceneOpcodes asm("D233+15788");
extern __typeof__(D233.script_default) cs_script_default asm("D233+8876");
extern __typeof__(D233.script_dingofire) cs_script_dingofire asm("D233+8900");
extern __typeof__(D233.script_tawnaCredits) cs_script_tawnaCredits asm("D233+3168");
extern __typeof__(D233.script_tawnaNormal) cs_script_tawnaNormal asm("D233+3128");
extern __typeof__(D233.danceFirstScripts) cs_danceFirstScripts asm("D233+8956");
extern __typeof__(D233.danceOtherScripts) cs_danceOtherScripts asm("D233+13840");
extern __typeof__(D233.advCharSelectSelectOpcodes) cs_advCharSelectSelectOpcodes asm("D233+20396");
extern __typeof__(D233.advCharSelectDeselectOpcodes) cs_advCharSelectDeselectOpcodes asm("D233+20428");
extern __typeof__(D233.introClearBoxRect) cs_introClearBoxRect asm("D233+26876");
extern __typeof__(D233.introClearBoxColor) cs_introClearBoxColor asm("D233+26872");
extern __typeof__(D233.creditsDancerRotOffset) cs_creditsDancerRotOffset asm("D233+26884");

extern __typeof__(csPodiumNames.s_first) cs_podiumFirstName asm("csPodiumNames+24");
extern __typeof__(csPodiumNames.s_tawna) cs_podiumTawnaName asm("csPodiumNames+32");
extern __typeof__(csPodiumNames.s_prize) cs_podiumPrizeName asm("csPodiumNames+40");
extern __typeof__(csPodiumNames.s_victorycam) cs_podiumVictoryCamName asm("csPodiumNames+48");

extern __typeof__(csIntroNames.s_introcam) cs_intro_s_introcam asm("csIntroNames+12");
extern __typeof__(csIntroNames.s_box1) cs_intro_s_box1 asm("csIntroNames+24");
extern __typeof__(csIntroNames.s_box2) cs_intro_s_box2 asm("csIntroNames+32");
extern __typeof__(csIntroNames.s_box2_bottom) cs_intro_s_box2_bottom asm("csIntroNames+40");
extern __typeof__(csIntroNames.s_box2_front) cs_intro_s_box2_front asm("csIntroNames+56");
extern __typeof__(csIntroNames.s_box2_A) cs_intro_s_box2_A asm("csIntroNames+72");
extern __typeof__(csIntroNames.s_box3) cs_intro_s_box3 asm("csIntroNames+80");
extern __typeof__(csIntroNames.s_code) cs_intro_s_code asm("csIntroNames+88");
extern __typeof__(csIntroNames.s_glow) cs_intro_s_glow asm("csIntroNames+96");
extern __typeof__(csIntroNames.s_lid) cs_intro_s_lid asm("csIntroNames+104");
extern __typeof__(csIntroNames.s_lidb) cs_intro_s_lidb asm("csIntroNames+108");
extern __typeof__(csIntroNames.s_lidc) cs_intro_s_lidc asm("csIntroNames+116");
extern __typeof__(csIntroNames.s_lidd) cs_intro_s_lidd asm("csIntroNames+124");
extern __typeof__(csIntroNames.s_lid2) cs_intro_s_lid2 asm("csIntroNames+132");
extern __typeof__(csIntroNames.s_kart0) cs_intro_s_kart0 asm("csIntroNames+140");
extern __typeof__(csIntroNames.s_kart1) cs_intro_s_kart1 asm("csIntroNames+148");
extern __typeof__(csIntroNames.s_kart2) cs_intro_s_kart2 asm("csIntroNames+156");
extern __typeof__(csIntroNames.s_kart3) cs_intro_s_kart3 asm("csIntroNames+164");
extern __typeof__(csIntroNames.s_kart6) cs_intro_s_kart6 asm("csIntroNames+172");
extern __typeof__(csIntroNames.s_kart7) cs_intro_s_kart7 asm("csIntroNames+180");

extern __typeof__(data.ptrColor) cs_colorPointers asm("data+5072");
extern __typeof__(data.colors[CREDITS_FADE]) cs_creditsFadePalette asm("data+5008");

extern __typeof__(creditsBSS.creditThread) cs_creditsThread asm("creditsBSS+20");
extern __typeof__(creditsBSS.dancerThread) cs_dancerThread asm("creditsBSS+24");
extern __typeof__(creditsBSS.dancerInst_invisible) cs_dancer asm("creditsBSS+28");
extern __typeof__(creditsBSS.numStrings) cs_creditsStringCount asm("creditsBSS+32");
extern __typeof__(creditsBSS.ptrStrings) cs_creditsStrings asm("creditsBSS+36");
extern __typeof__(creditsBSS.boolAllBlue) cs_creditsAllBlue asm("creditsBSS+40");
extern __typeof__(creditsBSS.creditGhostPos) cs_creditsGhostPos asm("creditsBSS");
extern __typeof__(creditsBSS.creditTextPosX) cs_creditsTextX asm("creditsBSS+16");

extern __typeof__(gGarage.numFramesMax_Zoom) cs_garageZoomFrames asm("gGarage+52");
extern __typeof__(gGarage.numFramesCurr_GarageMove) cs_garageMoveFrame asm("gGarage+160");
extern __typeof__(gGarage.numFramesCurr_ZoomIn) cs_garageZoomIn asm("gGarage+162");
extern __typeof__(gGarage.numFramesCurr_ZoomOut) cs_garageZoomOut asm("gGarage+164");
extern __typeof__(gGarage.boolSelected) cs_garageSelected asm("gGarage+168");
extern __typeof__(gGarage.delayOneSecond) cs_garageDelay asm("gGarage+166");
extern __typeof__(gGarage.garageCharacterIDs) cs_garageCharacters asm("gGarage+64");
extern __typeof__(gGarage.statBarLengths) cs_garageStatLengths asm("gGarage+80");
extern __typeof__(gGarage.unusedFrameCount) cs_garageUnusedFrames asm("gGarage+86");
extern __typeof__(gGarage.classStringIDs) cs_garageClassStrings asm("gGarage+88");
extern __typeof__(gGarage.statBarTargetLengths) cs_garageStatTargets asm("gGarage+96");
extern __typeof__(gGarage.statBarSegmentColors) cs_garageStatColors asm("gGarage+132");
extern __typeof__(gGarage.numFramesMax_GarageMove) cs_garageMoveFrames asm("gGarage+44");
extern __typeof__(gGarage.fovMin) cs_garageFovMin asm("gGarage+56");
extern __typeof__(gGarage.fovMax) cs_garageFovMax asm("gGarage+60");

// NOTE(aalhendi): Retail passes four-byte Color aggregates on the stack.
// Native converts these to the canonical packed-color arrow arguments.
extern void cs_decalHudArrow2D(struct Icon *, s32, s32, struct PrimMem *, u32 *, Color, Color, Color, Color, s32, s32, u16) asm("DecalHUD_Arrow2D");

extern __typeof__(D233.VertSplitLine) cs_VertSplitLine asm("R233+20876");
extern __typeof__(D233.boolLoadNextSwap) cs_boolLoadNextSwap asm("R233+20880");
extern __typeof__(D233.boolStartToSkip) cs_boolStartToSkip asm("R233+20884");
extern __typeof__(D233.bossCutsceneIndex) cs_bossCutsceneIndex asm("R233+20888");
extern __typeof__(D233.CutsceneManipulatesAudio) cs_CutsceneManipulatesAudio asm("R233+20892");
extern __typeof__(D233.cs_initMatrixTable) cs_cs_initMatrixTable asm("R233+47424");
extern __typeof__(D233.cs_initMatrixBool) cs_cs_initMatrixBool asm("R233+47456");
extern __typeof__(D233.isCutsceneOver) cs_isCutsceneOver asm("R233+48496");
extern __typeof__(D233.podiumCameraFrame) cs_podiumCameraFrame asm("R233+48500");
extern __typeof__(D233.volumeBackup) cs_volumeBackup asm("R233+48504");
extern __typeof__(D233.podiumPrizeDropReady) cs_podiumPrizeDropReady asm("R233+48512");
extern __typeof__(D233.cutsceneState) cs_cutsceneState asm("R233+48516");
extern struct BigHeader *cs_bigfileHeader asm("sdata_static+304");
extern s32 cs_loadInProgress asm("sdata_static+312");
extern MainMenuState cs_mainMenuState asm("sdata_static+2576");
extern __typeof__(sdata_static.XA_State) cs_xaState asm("sdata_static+1948");
extern __typeof__(sdata_static.XA_CurrOffset) cs_xaOffset asm("sdata_static+1932");
extern __typeof__(sdata_static.queueReady) cs_queueReady asm("sdata_static+308");
extern __typeof__(sdata_static.queueLength) cs_queueLength asm("sdata_static+310");
extern __typeof__(sdata_static.advCharSelectIndex_curr) cs_garageCurrent asm("sdata_static+2472");
extern __typeof__(sdata_static.advCharSelectIndex_prev) cs_garagePrevious asm("sdata_static+2480");
// Only the low half of the full frame counter is read here.
extern u16 cs_frameCounterLow asm("sdata_static+2564");
extern __typeof__(sdata_static.AnyPlayerHold) cs_buttonsHold asm("sdata_static+2568");
extern __typeof__(sdata_static.ptrDesiredMenu) cs_desiredMenu asm("sdata_static+2488");

extern __typeof__(D233.bossModels) cs_bossModels asm("R233+48520");

// NOTE(aalhendi): Cutscene code addresses resident state absolutely.
#define sdata (&sdata_static)

#endif

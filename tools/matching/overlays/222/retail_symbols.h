#ifndef CTR_MATCHING_OVERLAY_222_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_222_RETAIL_SYMBOLS_H

#include <common.h>

// NOTE(aalhendi): Overlay 222 addresses resident EXE state absolutely rather
// than through gp. Keep those artifact bindings out of shared layout headers.
#define GAME_TRACKER_PTR_ASM_NAME        "sdata_static+832"
#define GAME_ADD_CONFIG_0_ASM_NAME       "sdata_static+404"
#define GAME_HUD_T_ASM_NAME              "sdata_static+2648"
#define GAME_HUD_R_ASM_NAME              "sdata_static+2652"
#define GAME_HUD_STRUCTS_ASM_NAME        "data+22716"
#define GAME_CHARACTER_METADATA_ASM_NAME "data+25572"

extern struct GameTracker *aa_gameTrackerPtr asm(GAME_TRACKER_PTR_ASM_NAME);
extern s32 aa_gameFramesSinceRaceEnded asm("sdata_static+1472");
extern struct Instance *aa_gameHudC asm("sdata_static+2656");
extern struct Instance *aa_gameHudT asm(GAME_HUD_T_ASM_NAME);
extern struct Instance *aa_gameHudR asm(GAME_HUD_R_ASM_NAME);
extern struct Instance *aa_gameToken asm("sdata_static+2660");
extern char **aa_languageStrings asm("sdata_static+2316");
extern s16 aa_gameNumIconsEOR asm("sdata_static+1540");
extern s32 aa_gameAnyPlayerTap asm("sdata_static+2532");
extern s32 aa_gameMenuReady asm("sdata_static+1360");
extern u32 aa_gameAddConfig0 asm(GAME_ADD_CONFIG_0_ASM_NAME);
extern u32 aa_gameRemoveConfig0 asm("sdata_static+408");
extern u32 aa_gameAddConfig8 asm("sdata_static+412");
extern u32 aa_gameRemoveConfig8 asm("sdata_static+416");
extern struct AdvProgress aa_gameAdvProgress asm("sdata_static+11320");
extern struct UiElement2D *aa_gameHudStructs[4] asm(GAME_HUD_STRUCTS_ASM_NAME);
extern struct MetaDataCHAR aa_gameCharacterMetadata[0x10] asm(GAME_CHARACTER_METADATA_ASM_NAME);
extern s16 aa_gameCharacterIDs[8] asm("data+25828");
extern struct RectMenu aa_gameMenuRetryExit asm("data+22900");

#define gameTrackerPtr           aa_gameTrackerPtr
#define gameFramesSinceRaceEnded aa_gameFramesSinceRaceEnded
#define gameAdvProgress          aa_gameAdvProgress
#define gameHudC                 aa_gameHudC
#define gameHudT                 aa_gameHudT
#define gameHudR                 aa_gameHudR
#define gameToken                aa_gameToken
#define languageStrings          aa_languageStrings
#define gameNumIconsEOR          aa_gameNumIconsEOR
#define gameAnyPlayerTap         aa_gameAnyPlayerTap
#define gameMenuReady            aa_gameMenuReady
#define gameAddConfig0           aa_gameAddConfig0
#define gameRemoveConfig0        aa_gameRemoveConfig0
#define gameAddConfig8           aa_gameAddConfig8
#define gameRemoveConfig8        aa_gameRemoveConfig8
#define gameHudStructs           aa_gameHudStructs
#define gameCharacterMetadata    aa_gameCharacterMetadata
#define gameCharacterIDs         aa_gameCharacterIDs
#define gameMenuRetryExit        aa_gameMenuRetryExit

#endif

#ifndef CTR_MATCHING_OVERLAY_222_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_222_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

// NOTE(aalhendi): Overlay 222 addresses resident EXE state absolutely rather
// than through gp. Keep those artifact bindings out of shared layout headers.
#define GAME_HUD_T_ASM_NAME              "sdata_static+2648"
#define GAME_HUD_R_ASM_NAME              "sdata_static+2652"
#define GAME_HUD_STRUCTS_ASM_NAME        "data+22716"

extern struct Instance *aa_gameHudC asm("sdata_static+2656");
extern struct Instance *aa_gameHudT asm(GAME_HUD_T_ASM_NAME);
extern struct Instance *aa_gameHudR asm(GAME_HUD_R_ASM_NAME);
extern s16 aa_gameNumIconsEOR asm("sdata_static+1540");
extern u32 aa_gameAddConfig8 asm("sdata_static+412");
extern u32 aa_gameRemoveConfig8 asm("sdata_static+416");
extern struct UiElement2D *aa_gameHudStructs[4] asm(GAME_HUD_STRUCTS_ASM_NAME);
extern struct RectMenu aa_gameMenuRetryExit asm("data+22900");

#define gameHudC                 aa_gameHudC
#define gameHudT                 aa_gameHudT
#define gameHudR                 aa_gameHudR
#define gameNumIconsEOR          aa_gameNumIconsEOR
#define gameAddConfig8           aa_gameAddConfig8
#define gameRemoveConfig8        aa_gameRemoveConfig8
#define gameHudStructs           aa_gameHudStructs
#define gameMenuRetryExit        aa_gameMenuRetryExit

#endif

#ifndef CTR_MATCHING_OVERLAY_223_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_223_RETAIL_SYMBOLS_H

#include <common.h>

// NOTE(aalhendi): Overlay 223 addresses resident EXE state absolutely rather
// than through gp. Keep those artifact bindings out of shared layout headers.
extern struct GameTracker *rr_gameTracker asm("sdata_static+832");
extern char **rr_languageStrings asm("sdata_static+2316");
extern s32 rr_framesSinceRaceEnded asm("sdata_static+1472");
extern struct Instance *rr_relic asm("sdata_static+2632");
extern struct Instance *rr_timebox1 asm("sdata_static+2668");
extern s32 rr_menuReady asm("sdata_static+1360");
extern s32 rr_anyPlayerTap asm("sdata_static+2532");
extern struct AdvProgress rr_advProgress asm("sdata_static+11320");
extern struct GameProgress rr_gameProgress asm("sdata_static+6012");
extern s32 rr_relicTimes[18][3] asm("data+22944");
extern s32 rr_relicTime1Min asm("sdata_static+2628");
extern s32 rr_relicTime10Sec asm("sdata_static+2676");
extern s32 rr_relicTime1Sec asm("sdata_static+2684");
extern s32 rr_relicTime10Ms asm("sdata_static+2664");
extern s32 rr_relicTime1Ms asm("sdata_static+2636");
extern Color rr_menuHighlight asm("sdata_static+2528");
extern struct MetaDataCHAR rr_characterMetadata[16] asm("data+25572");

// NOTE(aalhendi): These declarations preserve the retail callers' argument
// layout without imposing it on the native renderer interfaces.
extern void rr_drawPolyGT4(struct Icon *icon, s16 posX, s32 posY, struct PrimMem *primMem, u32 *ot, Color color0, Color color1, Color color2, Color color3,
                           s8 transparency, s16 scale) asm("RECTMENU_DrawPolyGT4");
extern void rr_drawClearBox(const RECT *rect, const Color *color, s32 transparency, u32 *ot, struct PrimMem *primMem) asm("CTR_Box_DrawClearBox");
extern void rr_drawLineWideX(char *str, s32 posX, s16 posY, s16 fontType, s16 flags) asm("DecalFont_DrawLine");

#define RR_GAME_TRACKER            rr_gameTracker
#define RR_LANGUAGE_STRINGS        rr_languageStrings
#define RR_FRAMES_SINCE_RACE_ENDED rr_framesSinceRaceEnded
#define RR_RELIC                   rr_relic
#define RR_TIMEBOX1                rr_timebox1
#define RR_MENU_READY              rr_menuReady
#define RR_ANY_PLAYER_TAP          rr_anyPlayerTap
#define RR_ADV_PROGRESS            rr_advProgress
#define RR_GAME_PROGRESS           rr_gameProgress
#define RR_RELIC_TIMES             rr_relicTimes
#define RR_RELIC_TIME_1MIN         rr_relicTime1Min
#define RR_RELIC_TIME_10SEC        rr_relicTime10Sec
#define RR_RELIC_TIME_1SEC         rr_relicTime1Sec
#define RR_RELIC_TIME_10MS         rr_relicTime10Ms
#define RR_RELIC_TIME_1MS          rr_relicTime1Ms
#define RR_MENU_HIGHLIGHT          rr_menuHighlight
#define RR_CHARACTER_METADATA      rr_characterMetadata
#define RR_DRAW_POLY_GT4           rr_drawPolyGT4
#define RR_DRAW_CLEAR_BOX          rr_drawClearBox
#define RR_DRAW_LINE_WIDE_X        rr_drawLineWideX

#endif

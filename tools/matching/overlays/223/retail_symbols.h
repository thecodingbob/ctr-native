#ifndef CTR_MATCHING_OVERLAY_223_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_223_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

// NOTE(aalhendi): Overlay 223 addresses resident EXE state absolutely rather
// than through gp. Keep those artifact bindings out of shared layout headers.
extern struct Instance *rr_relic asm("sdata_static+2632");
extern struct Instance *rr_timebox1 asm("sdata_static+2668");
extern s32 rr_relicTimes[18][3] asm("data+22944");
extern s32 rr_relicTime1Min asm("sdata_static+2628");
extern s32 rr_relicTime10Sec asm("sdata_static+2676");
extern s32 rr_relicTime1Sec asm("sdata_static+2684");
extern s32 rr_relicTime10Ms asm("sdata_static+2664");
extern s32 rr_relicTime1Ms asm("sdata_static+2636");

// NOTE(aalhendi): These declarations preserve the retail callers' argument
// layout without imposing it on the native renderer interfaces.
extern void rr_drawLineWideX(char *str, s32 posX, s16 posY, s16 fontType, s16 flags) asm("DecalFont_DrawLine");

#define RR_RELIC                   rr_relic
#define RR_TIMEBOX1                rr_timebox1
#define RR_RELIC_TIMES             rr_relicTimes
#define RR_RELIC_TIME_1MIN         rr_relicTime1Min
#define RR_RELIC_TIME_10SEC        rr_relicTime10Sec
#define RR_RELIC_TIME_1SEC         rr_relicTime1Sec
#define RR_RELIC_TIME_10MS         rr_relicTime10Ms
#define RR_RELIC_TIME_1MS          rr_relicTime1Ms
#define RR_DRAW_LINE_WIDE_X        rr_drawLineWideX

#endif

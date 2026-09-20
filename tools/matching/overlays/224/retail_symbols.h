#ifndef CTR_MATCHING_OVERLAY_224_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_224_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

// NOTE(aalhendi): Overlay 224 addresses resident EXE state absolutely rather
// than through gp. Keep those artifact bindings out of shared layout headers.
extern u32 tt_flags asm("sdata_static+2592");
extern b16 tt_ghostTooBig asm("sdata_static+2008");

// NOTE(aalhendi): These declarations preserve the retail callers' argument
// layout without imposing it on the native renderer interfaces.
extern void tt_drawLineWideX(char *str, s32 posX, s16 posY, s16 fontType, s16 flags) asm("DecalFont_DrawLine");

#define TT_FLAGS                   tt_flags
#define TT_GHOST_TOO_BIG           tt_ghostTooBig
#define TT_DRAW_LINE_WIDE_X        tt_drawLineWideX

#endif

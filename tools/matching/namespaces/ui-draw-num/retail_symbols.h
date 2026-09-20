#define UI_NUM_X             uiNumX
#define UI_NUM_INT           uiNumInt
#define UI_NUM_LONG          uiNumLong
#define UI_NUM_COLORS        uiNumColors
#define UI_NUM_DRAW_POLY_GT4 uiNumDrawPolyGT4

#include "../../retail_bindings.h"

// NOTE(aalhendi): GCC -G8 keeps scalar loads symbolic; assembler -G0 expands
// them as absolute LUI/LW pairs instead of GP-relative accesses.
extern char uiNumX[] asm("sdata_static+1468");
extern char uiNumInt[] asm("sdata_static+1444");
extern char uiNumLong[] asm("sdata_static+1460");
extern u32 *uiNumColors[NUM_COLORS] asm("data+5072");

// NOTE(aalhendi): This caller copies packed colors and passes full-word scale/blending arguments.
extern void uiNumDrawPolyGT4(struct Icon *icon, s32 x, s32 y, struct PrimMem *prim, u32 *ot, Color color0, Color color1, Color color2, Color color3,
                             s32 transparency, s32 scale) asm("DecalHUD_DrawPolyGT4");

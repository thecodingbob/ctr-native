#define FONT_ICON_GROUP        fontIconGroup
#define FONT_CHAR_WIDTH        fontCharWidth
#define FONT_CHAR_HEIGHT       fontCharHeight
#define FONT_PUNCT_WIDTH       fontPunctWidth
#define FONT_CHARACTER_ICONS   fontCharacterIcons
#define FONT_BUTTON_SCALE      fontButtonScale
#define FONT_BUTTON_WIDTH      fontButtonWidth
#define FONT_BUTTON_HEIGHT     fontButtonHeight
#define FONT_INDENT_ICONS      fontIndentIcons
#define FONT_INDENT_DIMENSIONS fontIndentDimensions
#define FONT_COLORS            fontColors
#define FONT_DRAW_POLY_GT4     fontDrawPolyGT4

#include "../../retail_bindings.h"

extern s16 fontIconGroup[FONT_NUM];
extern s16 fontCharWidth[FONT_NUM];
extern s16 fontCharHeight[FONT_NUM];
extern s16 fontPunctWidth[FONT_NUM];
extern u8 fontCharacterIcons[0xE0];
extern s16 fontButtonScale[FONT_NUM];
extern s16 fontButtonWidth[FONT_NUM];
extern s16 fontButtonHeight[FONT_NUM];
extern u8 fontIndentIcons[FONT_NUM * 2];
extern s16 fontIndentDimensions[FONT_NUM * 2];
extern u32 *fontColors[NUM_COLORS];

// NOTE(aalhendi): Retail copies byte-aligned colors into the outgoing arguments.
// This call site also keeps full-word blending and scale arguments.
extern void fontDrawPolyGT4(struct Icon *icon, s32 x, s32 y, struct PrimMem *prim, u32 *ot, Color color0, Color color1, Color color2, Color color3,
                            s32 transparency, s32 scale) asm("DecalHUD_DrawPolyGT4");

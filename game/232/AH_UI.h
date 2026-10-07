#ifndef CTR_AH_UI_H
#define CTR_AH_UI_H

// NOTE(aalhendi): Native palettes can be packed u32 arrays; copy their bytes into
// real Color values for the shared HUD entry points. Retail supplies its own bindings.
#define AH_DECALHUD_ARROW_2D(icon, x, y, primMem, ot, c0, c1, c2, c3, transparency, scale, rotation)                                                \
	DecalHUD_Arrow2D((icon), (x), (y), (primMem), (ot), ColorCode_Load(&(c0)), ColorCode_Load(&(c1)), ColorCode_Load(&(c2)), ColorCode_Load(&(c3)), \
	                 (transparency), (scale), (rotation))
#define AH_DECALHUD_DRAW_POLY_GT4(icon, x, y, primMem, ot, c0, c1, c2, c3, transparency, scale)                                                         \
	DecalHUD_DrawPolyGT4((icon), (x), (y), (primMem), (ot), ColorCode_Load(&(c0)), ColorCode_Load(&(c1)), ColorCode_Load(&(c2)), ColorCode_Load(&(c3)), \
	                     (transparency), (scale))

static inline void AH_DrawMenuEdge(RECT *rect, u32 *ot)
{
	// NOTE(aalhendi): Native's typed primitive builder needs a Color object,
	// not a Color pointer into the packed UI word. PSX passes that word's address.
	Color color;
	ColorCode_SetPacked(&color, AH_MENU_EDGE_COLOR);
	RECTMENU_DrawOuterRect_Edge(rect, &color, 0x20, ot);
}

#endif

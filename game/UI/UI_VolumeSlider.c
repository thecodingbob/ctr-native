#include <common.h>

void UI_DrawVolumeSlider(const RECT *rect, int volume, u32 *ot)
{
	enum
	{
		SLIDER_MARKER_WIDTH = 5,
		SLIDER_MARKER_INSET = 1,
		SLIDER_VOLUME_SHIFT = 8,
		SLIDER_COLOR_BAR = 3,
		SLIDER_COLOR_OUTLINE = 4,
	};
	struct PrimMem *primMem = &sdata->gGT->backBuffer->primMem;
	const int markerX = rect->x + ((volume * (rect->w - SLIDER_MARKER_WIDTH)) >> SLIDER_VOLUME_SHIFT);
	s16 triangle[6] = {
		rect->x, rect->y + rect->h,
		rect->x + rect->w, rect->y,
		rect->x + rect->w, rect->y + rect->h,
	};
	RECT marker = {markerX + SLIDER_MARKER_INSET, rect->y,
	               SLIDER_MARKER_WIDTH - 2 * SLIDER_MARKER_INSET, rect->h};
	Color color = ((Color *)data.Options_VolumeSlider_Colors)[SLIDER_COLOR_BAR];
	CTR_Box_DrawSolidBox(&marker, &color, ot, primMem);

	RECT outline = {markerX, rect->y - SLIDER_MARKER_INSET,
	                SLIDER_MARKER_WIDTH, rect->h + 2 * SLIDER_MARKER_INSET};
	color = ((Color *)data.Options_VolumeSlider_Colors)[SLIDER_COLOR_OUTLINE];
	CTR_Box_DrawSolidBox(&outline, &color, ot, primMem);
	RECTMENU_DrawRwdTriangle(triangle, data.Options_VolumeSlider_Colors, ot, primMem);
}

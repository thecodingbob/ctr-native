#include <common.h>

void MM_HighScore_Text3D(char *string, int posX, int posY, s16 font, u32 flags)
{
	// draw a string
	DecalFont_DrawLine(string, posX, posY, font, flags);

	// draw the same string in a different place
	DecalFont_DrawLine(string, (posX + 3), (posY + 1), font, (flags & (JUSTIFY_CENTER | JUSTIFY_RIGHT)) | BLACK);
}

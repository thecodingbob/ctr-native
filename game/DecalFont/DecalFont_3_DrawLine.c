#include <common.h>

void DecalFont_DrawLine(char *str, int posX, int posY, s16 fontType, int flags)
{
	DecalFont_DrawLineStrlen(str, -1, posX, posY, fontType, flags);
}

#include <common.h>

int DecalFont_DrawMultiLine(char *str, int posX, int posY, int maxPixLen, s16 fontType, int flags)
{
	return DecalFont_DrawMultiLineStrlen(str, -1, posX, posY, maxPixLen, fontType, flags);
}

#include <common.h>

int DecalFont_GetLineWidth(char *str, s16 fontType)
{
	return DecalFont_GetLineWidthStrlen(str, -1, fontType);
}

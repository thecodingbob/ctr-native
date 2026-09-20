#include <common.h>

#ifndef FONT_ICON_GROUP
#define FONT_ICON_GROUP        data.font_IconGroupID
#define FONT_CHAR_WIDTH        data.font_charPixWidth
#define FONT_CHAR_HEIGHT       data.font_charPixHeight
#define FONT_PUNCT_WIDTH       data.font_puncPixWidth
#define FONT_CHARACTER_ICONS   data.font_characterIconID
#define FONT_BUTTON_SCALE      data.font_buttonScale
#define FONT_BUTTON_WIDTH      data.font_buttonPixWidth
#define FONT_BUTTON_HEIGHT     data.font_buttonPixHeight
#define FONT_INDENT_ICONS      data.font_indentIconID
#define FONT_INDENT_DIMENSIONS data.font_indentPixDimensions
#define FONT_COLORS            data.ptrColor
#endif

#ifndef FONT_DRAW_POLY_GT4
#define FONT_DRAW_POLY_GT4(icon, x, y, primMem, ot, c0, c1, c2, c3, transparency, scale)                                                                \
	DecalHUD_DrawPolyGT4((icon), (x), (y), (primMem), (ot), ColorCode_Load(&(c0)), ColorCode_Load(&(c1)), ColorCode_Load(&(c2)), ColorCode_Load(&(c3)), \
	                     (transparency), (scale))
#endif

s32 DecalFont_GetLineWidthStrlen(char *str, s16 len, s16 fontType)
{
	s16 width = 0;
	while (*str != 0 && len != 0)
	{
		u8 c = *str;
		if (c == '@' || c == '[' || c == '^' || c == '*')
		{
			// NOTE(aalhendi): @, [, ^ and * encode circle, square, triangle and cross.
			width += FONT_CHAR_WIDTH[fontType] + FONT_BUTTON_WIDTH[fontType];
		}
		else if (c == ':' || c == '.')
		{
			width += FONT_PUNCT_WIDTH[fontType];
		}
		else if (c > 2)
		{
			width += FONT_CHAR_WIDTH[fontType];
		}
		str++;
		len--;
	}
	return width;
}


s32 DecalFont_GetLineWidth(char *str, s16 fontType)
{
	return (s16)DecalFont_GetLineWidthStrlen(str, -1, fontType);
}


void DecalFont_DrawLineStrlen(char *str, s16 len, s16 posX, s16 posY, s16 fontType, s16 flags)
{
	s16 x = posX;
	u8 *character;

	if (flags & JUSTIFY_CENTER)
	{
		x = posX - ((s16)DecalFont_GetLineWidthStrlen(str, len, fontType) >> 1);
	}
	else if (flags & JUSTIFY_RIGHT)
	{
		x = posX - DecalFont_GetLineWidthStrlen(str, len, fontType);
	}

	flags &= 0xfff;
	for (character = (u8 *)str; *character != 0 && len != 0; character++, len--)
	{
		s16 extraX = 0;
		u16 iconID = 0xff;
		s16 extraY = 0;
		s16 iconGroupID = FONT_ICON_GROUP[fontType];
		Color *colors = (Color *)FONT_COLORS[flags];
		s16 charWidth = FONT_CHAR_WIDTH[fontType];
		s16 iconScale = FP(1.0);
		s32 rawCode = *character;
		// NOTE(aalhendi): Keep the raw byte separate from the classifier's retail scratch register.
		register s32 c CTR_PSX_REGISTER("$3") = rawCode;

		if (c == ':' || c == '.')
		{
			charWidth = FONT_PUNCT_WIDTH[fontType];
		}
		else if (c == '@' || c == '[' || c == '^' || c == '*')
		{
			// NOTE(aalhendi): Button glyphs have their own scale, baseline and neutral palette.
			iconScale = FONT_BUTTON_SCALE[fontType];
			extraY = FONT_BUTTON_HEIGHT[fontType];
			charWidth = FONT_CHAR_WIDTH[fontType] + FONT_BUTTON_WIDTH[fontType];
			colors = (Color *)FONT_COLORS[GRAY];
		}

		c = rawCode;
		if (c < 3)
		{
			// NOTE(aalhendi): Bytes 1 and 2 select zero-advance accent placeholders.
			s32 indentBase = fontType * 2 - 1;
			s16 *dimensions = FONT_INDENT_DIMENSIONS + fontType * 2;
			iconID = (u8)FONT_INDENT_ICONS[c + indentBase];
			extraX = dimensions[0];
			extraY = dimensions[1];
			charWidth = 0;
		}
		else if ((u32)(c - 0x21) < 0xdf && iconID == 0xff)
		{
			iconID = FONT_CHARACTER_ICONS[c - 0x21];
		}

		if (iconID != 0xff)
		{
			struct GameTracker *gGT;
			struct IconGroup *group;
			if (iconID > 0x7f)
			{
				// NOTE(aalhendi): High IDs select the reserved kana groups, not the normal font atlas.
				s16 kanaGroup;
				iconID -= 0x80;
				kanaGroup = 15;
				if (iconGroupID == 4)
				{
					kanaGroup = 14;
				}
				iconGroupID = kanaGroup;
			}
			gGT = GAME_TRACKER;
			group = gGT->iconGroup[iconGroupID];
#ifdef CTR_NATIVE
			// NOTE(aalhendi): Native can boot before every retail icon group is loaded.
			if (group != NULL)
#endif
			{
				if (iconID < group->numIcons)
				{
					struct Icon **icons = ICONGROUP_GETICONS(group);
					FONT_DRAW_POLY_GT4(icons[iconID], x + extraX, posY + extraY, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, colors[0], colors[1],
					                   colors[2], colors[3], 0, iconScale);
				}
			}
		}
		x += charWidth;
	}
}


void DecalFont_DrawLine(char *str, s16 posX, s16 posY, s16 fontType, s16 flags)
{
	DecalFont_DrawLineStrlen(str, -1, (s16)posX, (s16)posY, fontType, (s16)flags);
}


void DecalFont_DrawLineOT(char *str, s16 posX, s16 posY, s16 fontType, s16 flags, u32 *ot)
{
	struct GameTracker *gGT;
	u32 *backupOT;

	gGT = GAME_TRACKER;

	backupOT = gGT->pushBuffer_UI.ptrOT;
	gGT->pushBuffer_UI.ptrOT = ot;

	DecalFont_DrawLine(str, (s16)posX, (s16)posY, fontType, (s16)flags);

	// NOTE(aalhendi): Reload the tracker after drawing before restoring its UI table.
	GAME_TRACKER->pushBuffer_UI.ptrOT = backupOT;
}


s32 DecalFont_DrawMultiLineStrlen(char *str, s16 len, s16 posX, s16 posY, s16 maxPixLen, s16 fontType, s16 flags)
{
	s16 totalHeight = 0;
	char *cursor;
	char *lineStart;
	s16 remainingAtBreak;
	s16 lineWidth;

	do
	{
		cursor = str;
		remainingAtBreak = len;
		lineStart = str;
		// NOTE(aalhendi): CTR uses carriage returns for explicit line breaks.
		if (*str != '\r')
		{
			while (1)
			{
				if (*cursor == ' ' && len != 0)
				{
					cursor++;
					len--;
				}
				if (*cursor == 0 || len == 0)
				{
					break;
				}
				if (*cursor != ' ' && *cursor != '\r')
				{
					// NOTE(aalhendi): The word scanner keeps its own terminators and
					// short budget snapshot to retain the retail loop layout.
					s32 wordSpace = ' ';
					s32 wordReturn = '\r';
					while (1)
					{
						s16 remaining = len;
						if (remaining == 0)
						{
							break;
						}
						cursor++;
						len--;
						if (*cursor == 0 || *cursor == wordSpace || *cursor == wordReturn)
						{
							break;
						}
					}
				}
				lineWidth = DecalFont_GetLineWidthStrlen(lineStart, (s16)(cursor - lineStart), fontType);
				if (maxPixLen <= lineWidth)
				{
					break;
				}
				// NOTE(aalhendi): Accept only complete words that fit; retain their remaining byte budget.
				str = cursor;
				remainingAtBreak = len;
				if (*cursor == '\r')
				{
					break;
				}
			}
		}

		DecalFont_DrawLineStrlen(lineStart, (s16)(str - lineStart), posX, posY + totalHeight, fontType, flags);
		len = remainingAtBreak;
		totalHeight += FONT_CHAR_HEIGHT[fontType];
		if (*str == 0)
		{
			break;
		}
		if (len != 0)
		{
			str++;
			len--;
		}
		if (*str == 0 || len == 0)
		{
			break;
		}
	} while (1);
	return totalHeight;
}


s32 DecalFont_DrawMultiLine(char *str, s16 posX, s16 posY, s16 maxPixLen, s16 fontType, s16 flags)
{
	return (s16)DecalFont_DrawMultiLineStrlen(str, -1, (s16)posX, (s16)posY, (s16)maxPixLen, fontType, (s16)flags);
}

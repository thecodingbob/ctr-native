#include <common.h>

int DecalFont_GetLineWidthStrlen(char *character, int len, int fontType)
{
	s16 font_charPixWidth;
	s16 font_buttonPixWidth;
	s16 font_puncPixWidth;
	int pixLength;
	u8 c;

	font_charPixWidth = data.font_charPixWidth[fontType];
	font_buttonPixWidth = data.font_buttonPixWidth[fontType];
	font_puncPixWidth = data.font_puncPixWidth[fontType];
	pixLength = 0;

	while ((*character != 0) && (len != 0))
	{
		c = *character;

		// do not use "switch" or "else if" that increases the number of bytes, and makes the function too large

		// if the character is one of the PSX buttons
		// @ is circle, [ is square, ^ is triangle, * is cross
		if ((c == '@') || (c == '[') || (c == '^') || (c == '*'))
		{
			// character width, plus extra spacing for button
			pixLength += font_buttonPixWidth; // + font_charPixWidth
		}

		// colon or period
		if ((c == ':') || (c == '.'))
		{
			// punctuation spacing
			pixLength += font_puncPixWidth - font_charPixWidth; // + font_charPixWidth
		}


		// if normal character
		if (c > 2)
		{
			// normal character spacing
			// this will be added on top of button,
			// and colon, and period, so dont "else if"
			pixLength += font_charPixWidth;
		}

		character++;
		len--;
	}

	return pixLength;
}


int DecalFont_GetLineWidth(char *str, s16 fontType)
{
	return (s16)DecalFont_GetLineWidthStrlen(str, -1, fontType);
}


void DecalFont_DrawLineStrlen(char *str, s16 len, int posX, s16 posY, s16 fontType, int flags)
{
	struct GameTracker *gGT = sdata->gGT;

	// text is justified left by default
	if (flags & (JUSTIFY_CENTER | JUSTIFY_RIGHT))
	{
		int alignX = DecalFont_GetLineWidthStrlen(str, len, fontType);

		if (flags & JUSTIFY_CENTER)
		{
			alignX /= 2;
		}

		posX -= alignX;
	}


	flags &= 0xfff;


	for (; *str != 0 && len != 0; str++, len--)
	{
		u8 *strcopy = (u8 *)str;
		u16 iconID = 0xff;
		s16 charWidth = data.font_charPixWidth[fontType];
		s16 pixWidthExtra = 0;
		s16 pixHeightExtra = 0;
		s16 iconScale = FP(1.0);


		u32 *ptrColor = data.ptrColor[flags];


		if (*strcopy == ':' || *strcopy == '.')
		{
			charWidth = data.font_puncPixWidth[fontType];
		}

		// if the character is one of the PSX buttons
		// @ is circle, [ is square, ^ is triangle, * is X
		if ((((*strcopy == '@') || (*strcopy == '[')) || (*strcopy == '^')) || (*strcopy == '*'))
		{
			iconScale = data.font_buttonScale[fontType];
			pixHeightExtra = data.font_buttonPixHeight[fontType];
			charWidth = data.font_charPixWidth[fontType] + data.font_buttonPixWidth[fontType];

			// use neutral vertex color for button characters
			ptrColor = data.ptrColor[GRAY];
		}

		// Set character sprite (icon) IDs
		// The first 0x21 (counting 0) ASCII characters don't have icon IDs assigned to them
		// High icon IDs are reserved for the incomplete kana font path.

		// ASCII characters and reserved kana slots
		// 0xE0 characters from 0x20 to 0x100
		// TO DO: figure out why the cast to u32 is necessary --Super
		if (((u32)*strcopy - 0x21) < 0xdf)
		{
			// get iconID based on ascii character
			iconID = data.font_characterIconID[*strcopy - 0x21];
		}

		// Unused dakuten and handakuten placeholders
		if (*strcopy < 3)
		{
			charWidth = 0;
			iconID = data.font_indentIconID[fontType * 2 + *strcopy - 1];
			pixWidthExtra = data.font_indentPixDimensions[fontType * 2];
			pixHeightExtra = data.font_indentPixDimensions[(fontType * 2) + 1];
		}


		// if iconID is valid
		if (iconID != 0xff)
		{
			s16 iconGroupID = data.font_IconGroupID[fontType];


			// Incomplete kana font path; unused
			// see below for more details
			if (iconID > 0x7f)
			{
				iconID -= 0x80;
				s16 kanaIconGroupID = 15;
				if (iconGroupID == 4)
				{
					kanaIconGroupID = 14;
				}
				iconGroupID = kanaIconGroupID;
			}


// NOTE(aalhendi): Native can boot before every retail icon group is loaded.
#ifdef CTR_NATIVE
			if (gGT->iconGroup[iconGroupID] != 0)
			{
#endif

				if (iconID < gGT->iconGroup[iconGroupID]->numIcons)
				{
					struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[iconGroupID]);

					DecalHUD_DrawPolyGT4(iconPtrArray[iconID],

					                     posX + pixWidthExtra, posY + pixHeightExtra,

					                     &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,

					                     ptrColor[0], ptrColor[1], ptrColor[2], ptrColor[3],

					                     0, iconScale);
				}
			}
		}
		posX += charWidth;
	}
}


void DecalFont_DrawLine(char *str, s16 posX, s16 posY, s16 fontType, s16 flags)
{
	DecalFont_DrawLineStrlen(str, -1, (s16)posX, (s16)posY, fontType, (s16)flags);
}


void DecalFont_DrawLineOT(char *str, int posX, int posY, s16 fontType, int flags, u32 *ot)
{
	struct GameTracker *gGT;
	u32 *backupOT;

	gGT = sdata->gGT;

	// backup
	backupOT = gGT->pushBuffer_UI.ptrOT;

	// alter
	gGT->pushBuffer_UI.ptrOT = ot;

	// draw
	DecalFont_DrawLine(str, (s16)posX, (s16)posY, fontType, (s16)flags);

	// reset
	gGT->pushBuffer_UI.ptrOT = backupOT;
}


int DecalFont_DrawMultiLineStrlen(char *str, s16 len, s16 posX, s16 posY, s16 maxPixLen, s16 fontType, s16 flags)
{
	char strCharacter;
	s16 lineLen;
	char *currPointer;
	s16 lettersRemaining;
	char *strPointer;
	int totalPassageHeight;

	totalPassageHeight = 0;

	do
	{
		// pointer to string
		strPointer = str;

		// rather than using \n for new lines, CTR uses \r, which is similar if you try it with printf

		// while you've not reached the end of the line
		if (*str != '\r')
		{
			// get the first character
			strCharacter = *str;

			while (1)
			{
				// pointer to current letter
				currPointer = strPointer;

				// number of letters remaining
				lettersRemaining = len;

				// if you reached a space, and you're
				// not out of letters yet
				if ((strCharacter == ' ') && (len != 0))
				{
					// increment pointer to next letter
					currPointer = strPointer + 1;

					// one letter less
					lettersRemaining = len - 1;
				}

				// get next character
				strCharacter = *currPointer;

				// if nullptr, or out of letters, quit the loop
				if ((strCharacter == '\0') || (lettersRemaining == 0))
				{
					break;
				}

				// if this is a letter, number, or symbol
				if ((strCharacter != ' ') && (strCharacter != '\r'))
				{
					// get the length of the next word
					while (lettersRemaining != 0)
					{
						// increment pointer to next letter
						currPointer = currPointer + 1;

						// get value of next character
						strCharacter = *currPointer;

						// reduce number of remaining characters
						lettersRemaining = lettersRemaining + -1;

						// stop counting at a nullptr,
						// or a space (end of word),
						// or the end of the line '\r'
						if (((strCharacter == '\0') || (strCharacter == ' ')) || (strCharacter == '\r'))
						{
							break;
						}
					}
				}

				lineLen = DecalFont_GetLineWidthStrlen(str, (u32)currPointer - (u32)str, (int)fontType);

				if (
				    // if parameter line length is longer than string line length
				    (maxPixLen <= lineLen) || (
				                                  // get character
				                                  strCharacter = *currPointer,

				                                  // update pointer
				                                  strPointer = currPointer,

				                                  // update number of remaining characters
				                                  len = lettersRemaining,

				                                  // check if this is new line
				                                  strCharacter == '\r'))
				{
					break;
				}
			}
		}

		DecalFont_DrawLineStrlen(str, (u32)strPointer - (u32)str, (int)posX, posY + totalPassageHeight, (int)fontType, (int)flags);


		totalPassageHeight += data.font_charPixHeight[fontType];


		if (*strPointer == '\0')
		{
		EndFunction:
			return totalPassageHeight;
		}

		if (len != 0)
		{
			strPointer = strPointer + 1;
			len = len + -1;
		}
		if ((*strPointer == '\0') || (str = strPointer, len == 0))
		{
			goto EndFunction;
		}
	} while (1);
}


int DecalFont_DrawMultiLine(char *str, int posX, int posY, int maxPixLen, s16 fontType, int flags)
{
	return (s16)DecalFont_DrawMultiLineStrlen(str, -1, (s16)posX, (s16)posY, (s16)maxPixLen, fontType, (s16)flags);
}

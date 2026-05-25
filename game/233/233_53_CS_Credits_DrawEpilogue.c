#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8bd0-0x800b8dc8
void CS_Credits_DrawEpilogue(struct CreditsObj *co)
{
	if (co->epilogue_topString == 0)
		return;

	co->epilogueCount200--;

	if (co->epilogueCount200 <= 0)
	{
		co->epilogueCount200 = 200;
		co->epilogue_topString = co->epilogue_nextString;
		co->epilogue_nextString = CS_Credits_GetNextString(co->epilogue_nextString);
	}

	if (co->epilogue_topString == 0)
		return;

	s16 timeRemaining = co->epilogueCount200;
	s16 fadeAmount = 20;

	if (timeRemaining < 0xb5)
	{
		if (timeRemaining <= 0x13)
		{
			fadeAmount = timeRemaining;
		}
	}
	else
	{
		fadeAmount = 200 - timeRemaining;
	}

	s16 colorSlot = 4;

	if (fadeAmount < 20)
	{
		if (fadeAmount > 0)
		{
			colorSlot = 0x1f;

			int fade8 = (fadeAmount << 8) / 20;
			char *dst = (char *)&data.colors[31];
			char *src = (char *)data.ptrColor[4];

			for (int i = 0; i < 4; i++)
			{
				int stride = i * 4;
				dst[stride + 0] = (char)((u8)src[stride + 0] * fade8 >> 8);
				dst[stride + 1] = (char)((u8)src[stride + 1] * fade8 >> 8);
				dst[stride + 2] = (char)((u8)src[stride + 2] * fade8 >> 8);
			}
		}
		else
		{
			colorSlot = -1;
		}
	}

	if ((colorSlot >= 0) && (creditsBSS.boolAllBlue != 0))
	{
		s16 strLen = -1;

		if (co->epilogue_nextString != 0)
		{
			strLen = (s16)(co->epilogue_nextString - co->epilogue_topString) - 1;
		}

		DecalFont_DrawMultiLineStrlen(co->epilogue_topString, strLen, 0x100, 0xaf, 0x1cc, 2, colorSlot | 0x8000);
	}
}

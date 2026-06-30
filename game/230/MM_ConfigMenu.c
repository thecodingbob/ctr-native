#include <common.h>
#include <stdio.h>


// Row arrays with CONFIG entry at bottom, used by MM_MenuProc_Main
struct MenuRow s_rowsMainMenuBasicConfig[] = {
	{0x4C, 0, 1, 0, 0},
	{0x4D, 0, 2, 1, 1},
	{0x4E, 1, 3, 2, 2},
	{0x4F, 2, 4, 3, 3},
	{0x50, 3, 5, 4, 4},
	{0x51, 4, 6, 5, 5},
	{0x0E, 5, 6, 6, 6},
	{-1},
};

struct MenuRow s_rowsMainMenuWithSBConfig[] = {
	{0x4C, 0, 1, 0, 0},
	{0x4D, 0, 2, 1, 1},
	{0x4E, 1, 3, 2, 2},
	{0x4F, 2, 4, 3, 3},
	{0x50, 3, 5, 4, 4},
	{0x51, 4, 6, 5, 5},
	{0x234, 5, 7, 6, 6},
	{0x0E, 6, 7, 7, 7},
	{-1},
};

static void MM_MenuProc_Config(struct RectMenu *menu);

// Section lookup built from g_configEntries at first use
static int s_sectionToEntry[16];
static int s_sectionCount[16];
static int s_numSections = 0;

static void BuildSectionMap(void)
{
	s_numSections = 0;
	const char *curSection = NULL;
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		if (curSection == NULL || strcmp(g_configEntries[i].section, curSection) != 0)
		{
			curSection = g_configEntries[i].section;
			s_sectionToEntry[s_numSections] = i;
			s_sectionCount[s_numSections] = 0;
			s_numSections++;
		}
		s_sectionCount[s_numSections - 1]++;
	}
}

static int s_currentSection = -1; // -1 = section selector, 0+ = submenu

struct RectMenu g_configMenu = {
	.stringIndexTitle = -1,
	.state = EXECUTE_FUNCPTR | DISABLE_INPUT_ALLOW_FUNCPTRS,
	.funcPtr = MM_MenuProc_Config,
};

static void Config_UpdateSlider(const struct GamepadBuffer *pad, const int rowSelected,
                              const int localRow, int *value, const int min, const int max, const int step)
{
	if (rowSelected != localRow)
		return;
	const int held = pad->buttonsHeldCurrFrame;
	if ((held & BTN_LEFT) != 0 && (sdata->frameCounter % 3) == 0)
	{
		*value -= step;
		if (*value < min) *value = min;
	}
	if ((held & BTN_RIGHT) != 0 && (sdata->frameCounter % 3) == 0)
	{
		*value += step;
		if (*value > max) *value = max;
	}
}

static void Config_DrawValue(const ConfigEntry *e, const int valueX, int y, uint32_t *ot, char *buf)
{
	if (e->type == CFG_BOOL)
	{
		DecalFont_DrawLineOT(*(bool *)e->valuePtr ? "ON" : "OFF",
			valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
	}
	else if (e->type == CFG_ENUM)
	{
		int val = *(int *)e->valuePtr;
		const char *name = "?";
		for (int j = 0; j < e->numEnumValues; j++)
		{
			if (e->enumValues[j].value == val)
			{
				name = e->enumValues[j].name;
				break;
			}
		}
		DecalFont_DrawLineOT((char *)name, valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
	}
	else
	{
		sprintf(buf, "%d%%", *(int *)e->valuePtr);
		DecalFont_DrawLineOT(buf, valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
	}
}

static void MM_MenuProc_Config(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];
	char buf[32];

	if (s_numSections == 0)
		BuildSectionMap();

	if ((pad->buttonsTapped & (BTN_TRIANGLE | BTN_START)) != 0)
	{
		OtherFX_Play(2, 1);
		if (s_currentSection >= 0)
		{
			menu->rowSelected = s_currentSection;
			s_currentSection = -1;
		}
		else
		{
			NativeConfig_Save();
			sdata->ptrDesiredMenu = &D230.menuMainMenu;
		}
	}

	if (s_currentSection >= 0)
	{
		const int sec = s_currentSection;
		const int numRows = s_sectionCount[sec];
		const int firstEntry = s_sectionToEntry[sec];

		if ((pad->buttonsTapped & BTN_UP) != 0)
		{
			menu->rowSelected = (menu->rowSelected > 0) ? menu->rowSelected - 1 : numRows - 1;
			OtherFX_Play(0, 1);
		}
		if ((pad->buttonsTapped & BTN_DOWN) != 0)
		{
			menu->rowSelected = (menu->rowSelected < numRows - 1) ? menu->rowSelected + 1 : 0;
			OtherFX_Play(0, 1);
		}

		if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
		{
			OtherFX_Play(1, 1);
			const ConfigEntry *e = &g_configEntries[firstEntry + menu->rowSelected];
			if (e->type == CFG_BOOL)
				*(bool *)e->valuePtr ^= 1;
			else if (e->type == CFG_ENUM)
			{
				int *val = (int *)e->valuePtr;
				*val = (*val + 1) % e->numEnumValues;
			}
		}

		// slider update for int entries
		for (int j = 0; j < numRows; j++)
		{
			const ConfigEntry *e = &g_configEntries[firstEntry + j];
			if (e->type == CFG_INT)
				Config_UpdateSlider(pad, menu->rowSelected, j, (int *)e->valuePtr, e->min, e->max, e->step);
		}

		DecalFont_DrawLineOT((char *)g_configEntries[firstEntry].section,
			0x100, 0x18, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);

		int labelX = 0x38;
		int valueX = 0x1DC;
		int startY = 0x3C;
		int rowSpacing = 0x0E;

		for (int j = 0; j < numRows; j++)
		{
			const ConfigEntry *e = &g_configEntries[firstEntry + j];
			int y = startY + j * rowSpacing;

			DecalFont_DrawLineOT((char *)e->label, labelX, y, FONT_SMALL, ORANGE, ot);
			Config_DrawValue(e, valueX, y, ot, buf);

			if (j == menu->rowSelected)
			{
				RECT sel = {0x30, y - 2, 0x1B0, 0x0C};
				CTR_Box_DrawClearBox(&sel, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot);
			}
		}
	}
	else
	{
		if ((pad->buttonsTapped & BTN_UP) != 0)
		{
			menu->rowSelected = (menu->rowSelected > 0) ? menu->rowSelected - 1 : s_numSections - 1;
			OtherFX_Play(0, 1);
		}
		if ((pad->buttonsTapped & BTN_DOWN) != 0)
		{
			menu->rowSelected = (menu->rowSelected < s_numSections - 1) ? menu->rowSelected + 1 : 0;
			OtherFX_Play(0, 1);
		}

		if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
		{
			OtherFX_Play(1, 1);
			s_currentSection = menu->rowSelected;
			menu->rowSelected = 0;
		}

		DecalFont_DrawLineOT(sdata->lngStrings[LNG_OPTIONS],
			0x100, 0x18, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);

		int labelX = 0x38;
		int startY = 0x3C;
		int spacing = 0x0E;

		for (int i = 0; i < s_numSections; i++)
		{
			const ConfigEntry *e = &g_configEntries[s_sectionToEntry[i]];
			int y = startY + i * spacing;
			DecalFont_DrawLineOT((char *)e->section, labelX, y, FONT_SMALL, ORANGE, ot);
			if (i == menu->rowSelected)
			{
				RECT sel = {0x30, y - 2, 0x1B0, 0x0C};
				CTR_Box_DrawClearBox(&sel, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot);
			}
		}
	}

	{
		RECT sep = {0x20, 0x2C, 0x1C0, 2};
		Color sepColor;
		sepColor.self = sdata->battleSetup_Color_UI_1;
		RECTMENU_DrawOuterRect_Edge(&sep, sepColor, 0x20, ot);
	}

	RECT bg = {0x10, 4, 0x1E0, 0xCE};
	RECTMENU_DrawInnerRect(&bg, 4, ot);
}

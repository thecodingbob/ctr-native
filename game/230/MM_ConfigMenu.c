#include <common.h>
#include <stdio.h>

#define CONFIG_ROW_SKIP_INTROS      0
#define CONFIG_ROW_SKIP_HINTS       1
#define CONFIG_ROW_UNLOCK_GATES     2
#define CONFIG_ROW_UNLOCK_PORTALS   3
#define CONFIG_ROW_SPEED            4
#define CONFIG_ROW_GRAVITY          5
#define CONFIG_ROW_UNLOCK_CHARS     6
#define CONFIG_ROW_DRAW_DISTANCE    7

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

static void Config_UpdateSlider(struct GamepadBuffer *pad, int rowSelected, int rowIndex, int *value, int min, int max, int step)
{
	if (rowSelected != rowIndex)
		return;
	int held = pad->buttonsHeldCurrFrame;
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

static void Config_DrawSlider(char *label, int val, int labelX, int valueX, int y, uint32_t *ot, char *buf)
{
	DecalFont_DrawLineOT(label, labelX, y, FONT_SMALL, ORANGE, ot);
	sprintf(buf, "%d%%", val);
	DecalFont_DrawLineOT(buf, valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
}

static void MM_MenuProc_Config(struct RectMenu *menu);

static char *s_sectionLabels[] = {
	"General",
	"Adventure",
	"Vehicle",
	"Unlocks",
	"Graphics",
};
static int s_rowsPerSection[] = {1, 3, 2, 1, 1};
static int s_sectionRowOffset[] = {0, 1, 4, 6, 7};
static int s_currentSection = -1; // -1 = section selector, 0-4 = submenu

struct RectMenu g_configMenu = {
	.stringIndexTitle = -1,
	.state = EXECUTE_FUNCPTR | DISABLE_INPUT_ALLOW_FUNCPTRS,
	.funcPtr = MM_MenuProc_Config,
};

static void MM_MenuProc_Config(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];
	char buf[32];

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
		int sectionIndex = s_currentSection;
		int numRows = s_rowsPerSection[sectionIndex];
		int firstRow = s_sectionRowOffset[sectionIndex];

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
			int globalRow = firstRow + menu->rowSelected;
			switch (globalRow)
			{
				case CONFIG_ROW_SKIP_INTROS:    g_config.skipIntro ^= 1; break;
				case CONFIG_ROW_SKIP_HINTS:     g_config.skipHints ^= 1; break;
				case CONFIG_ROW_UNLOCK_GATES:   g_config.unlockAllGates ^= 1; break;
				case CONFIG_ROW_UNLOCK_PORTALS: g_config.unlockAllPortals ^= 1; break;
				case CONFIG_ROW_UNLOCK_CHARS:   g_config.unlockAllCharacters ^= 1; break;
				case CONFIG_ROW_DRAW_DISTANCE:  g_config.increaseDrawDistance ^= 1; break;
			}
		}

		if (sectionIndex == 2)
		{
			Config_UpdateSlider(pad, menu->rowSelected, 0, &g_config.speedMultiplier, 10, 200, 10);
			Config_UpdateSlider(pad, menu->rowSelected, 1, &g_config.gravityMultiplier, 10, 300, 10);
		}

		DecalFont_DrawLineOT(s_sectionLabels[sectionIndex],
			0x100, 0x18, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);

		int labelX = 0x38;
		int valueX = 0x1DC;
		int startY = 0x3C;
		int rowSpacing = 0x0E;

		for (int j = 0; j < numRows; j++)
		{
			int globalRow = firstRow + j;
			int y = startY + j * rowSpacing;

			switch (globalRow)
			{
				case CONFIG_ROW_SKIP_INTROS:
					DecalFont_DrawLineOT("Skip Intros", labelX, y, FONT_SMALL, ORANGE, ot);
					DecalFont_DrawLineOT(g_config.skipIntro ? "ON" : "OFF",
						valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
					break;
				case CONFIG_ROW_SKIP_HINTS:
					DecalFont_DrawLineOT("Skip Mask Hints", labelX, y, FONT_SMALL, ORANGE, ot);
					DecalFont_DrawLineOT(g_config.skipHints ? "ON" : "OFF",
						valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
					break;
				case CONFIG_ROW_UNLOCK_GATES:
					DecalFont_DrawLineOT("Open All Gates", labelX, y, FONT_SMALL, ORANGE, ot);
					DecalFont_DrawLineOT(g_config.unlockAllGates ? "ON" : "OFF",
						valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
					break;
				case CONFIG_ROW_UNLOCK_PORTALS:
					DecalFont_DrawLineOT("Open All Portals", labelX, y, FONT_SMALL, ORANGE, ot);
					DecalFont_DrawLineOT(g_config.unlockAllPortals ? "ON" : "OFF",
						valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
					break;
				case CONFIG_ROW_SPEED:
					Config_DrawSlider("Kart Speed Multiplier", g_config.speedMultiplier, labelX, valueX, y, ot, buf);
					break;
				case CONFIG_ROW_GRAVITY:
					Config_DrawSlider("Gravity Multiplier", g_config.gravityMultiplier, labelX, valueX, y, ot, buf);
					break;
				case CONFIG_ROW_UNLOCK_CHARS:
					DecalFont_DrawLineOT("Unlock All Characters", labelX, y, FONT_SMALL, ORANGE, ot);
					DecalFont_DrawLineOT(g_config.unlockAllCharacters ? "ON" : "OFF",
						valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
					break;
				case CONFIG_ROW_DRAW_DISTANCE:
					DecalFont_DrawLineOT("Increase Draw Distance", labelX, y, FONT_SMALL, ORANGE, ot);
					DecalFont_DrawLineOT(g_config.increaseDrawDistance ? "ON" : "OFF",
						valueX, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
					break;
			}

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
			menu->rowSelected = (menu->rowSelected > 0) ? menu->rowSelected - 1 : numConfigSections - 1;
			OtherFX_Play(0, 1);
		}
		if ((pad->buttonsTapped & BTN_DOWN) != 0)
		{
			menu->rowSelected = (menu->rowSelected < numConfigSections - 1) ? menu->rowSelected + 1 : 0;
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

		for (int i = 0; i < numConfigSections; i++)
		{
			int y = startY + i * spacing;
			DecalFont_DrawLineOT(s_sectionLabels[i], labelX, y, FONT_SMALL, ORANGE, ot);
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

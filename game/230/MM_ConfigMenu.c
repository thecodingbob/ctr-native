#include <common.h>

#include <platform/native_controls.h>
#include <stdio.h>


// Row arrays with CONFIG entry at bottom, used by MM_MenuProc_Main
struct MenuRow s_rowsMainMenuBasicConfig[] = {
	{0x4C, 7, 1, 0, 0},
	{0x4D, 0, 2, 1, 1},
	{0x4E, 1, 3, 2, 2},
	{0x4F, 2, 4, 3, 3},
	{0x50, 3, 5, 4, 4},
	{0x51, 4, 6, 5, 5},
	{0x0E, 5, 7, 6, 6},
	{0x003, 6, 0, 7, 7},
	{RECTMENU_STRING_NONE, 0, 0, 0, 0},
};

struct MenuRow s_rowsMainMenuWithSBConfig[] = {
	{0x4C, 8, 1, 0, 0},
	{0x4D, 0, 2, 1, 1},
	{0x4E, 1, 3, 2, 2},
	{0x4F, 2, 4, 3, 3},
	{0x50, 3, 5, 4, 4},
	{0x51, 4, 6, 5, 5},
	{0x234, 5, 7, 6, 6},
	{0x0E, 6, 8, 7, 7},
	{0x003, 7, 0, 8, 8},
	{RECTMENU_STRING_NONE, 0, 0, 0, 0},
};

static void MM_MenuProc_Config(struct RectMenu *menu);

// Owns the "Controls" section screen: player selection, device selection, and
// rebinding. Defined in MM_ControlsConfig.c.
void MM_MenuProc_Controls(struct RectMenu *menu);

// True while the device picker is up, so this file leaves the back button to it
// instead of backing out of the whole section.
b32 MM_Controls_IsPickerOpen(void);

// Union of the button presses this screen accepts: player 1's device and the selected
// player's own. Used for the back button, which this file handles on its behalf.
u32 MM_Controls_MenuTapped(void);

// Frame and separators shared by the section list and every section screen.
static void Config_DrawFrame(uint32_t *ot)
{
	{
		RECT sep = {0x20, 0x2C, 0x1C0, 2};
		Color sepColor;
		ColorCode_SetPacked(&sepColor, sdata->battleSetup_Color_UI_1);
		RECTMENU_DrawOuterRect_Edge(&sep, &sepColor, 0x20, ot);
	}

	RECT bg = {0x10, 4, 0x1E0, 0xCE};
	RECTMENU_DrawInnerRect(&bg, 4, ot);
}

static void Config_DrawRowHighlight(struct GameTracker *gGT, uint32_t *ot, int y)
{
	RECT sel = {0x30, y - 2, 0x1B0, 0x0C};
	CTR_Box_DrawClearBox(&sel, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, ot, &gGT->backBuffer->primMem);
}

// Section lookup built from g_configEntries at first use
static int s_sectionToEntry[16];
static int s_sectionCount[16];
static int s_numSections = 0;

static void BuildSectionMap(void)
{
	s_numSections = 0;
	const char *curSection = NULL;
	extern NativeConfig g_config;
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		// Only include Developer Hacks section if enabled
		if (strcmp(g_configEntries[i].section, CONFIG_SECTION_DEVELOPER_HACKS) == 0)
		{
			if (!g_config.developerHacksEnabled)
			{
				continue;
			}
		}
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
static int s_scrollOffset = 0;    // first visible row in submenu
#define CONFIG_MAX_VISIBLE_ROWS 10

// Row geometry shared with MM_ControlsConfig.c. Every screen in the options menu puts its
// title and rows in the same places, so the numbers are named once here rather than being
// restated by each screen that draws a list.
#define CONFIG_TITLE_Y     0x18
#define CONFIG_ROW_START_Y 0x3C
#define CONFIG_ROW_SPACING 0x0E
#define CONFIG_LABEL_X     0x38
#define CONFIG_VALUE_X     0x1DC

// One clamped step of an option row, and the cadence a held direction runs at. Lives here
// beside the other helpers the Controls screen borrows, so a numeric option behaves the
// same wherever it is edited rather than each screen inventing its own.

// Frames a held Left/Right waits before it steps again, so one press moves a single step
// and holding runs the value through its range at a pace that can be seen.
#define CONFIG_STEP_REPEAT_FRAMES 3

// True when a held direction should step this frame. Takes the button mask rather than a
// pad, because a screen may answer for more than one player and each has its own mask.
static bool Config_OptionStepDue(int held, int direction)
{
	const int button = (direction < 0) ? BTN_LEFT : BTN_RIGHT;

	if ((held & button) == 0)
	{
		return false;
	}

	return (sdata->frameCounter % CONFIG_STEP_REPEAT_FRAMES) == 0;
}

// Applies one clamped step. Reports whether the value moved, so a caller can commit only on
// a real change instead of once per frame while a direction is held.
static bool Config_OptionStep(int *value, int direction, int min, int max, int step)
{
	const int before = *value;
	int wanted = before + (direction * step);

	if (wanted < min)
	{
		wanted = min;
	}
	else if (wanted > max)
	{
		wanted = max;
	}

	*value = wanted;

	return *value != before;
}

// True when the section at this index is one of the screens that draws and edits
// its own rows instead of going through g_configEntries values.
static bool Config_IsCustomSection(int section)
{
	if ((section < 0) || (section >= s_numSections))
	{
		return false;
	}

	return strcmp(g_configEntries[s_sectionToEntry[section]].section, CONFIG_SECTION_CONTROLS) == 0;
}

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

	for (int direction = -1; direction <= 1; direction += 2)
	{
		if (Config_OptionStepDue(pad->buttonsHeldCurrFrame, direction))
		{
			Config_OptionStep(value, direction, min, max, step);
		}
	}
}

// A numeric option's value, in the shared value column. Every screen shows these as a
// percentage, so the sign and the column are written once here rather than per screen.
static void Config_DrawPercent(int percent, int y, uint32_t *ot)
{
	char text[8];

	snprintf(text, sizeof(text), "%d%%", percent);
	DecalFont_DrawLineOT(text, CONFIG_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
}

static void Config_DrawValue(const ConfigEntry *e, int y, uint32_t *ot)
{
	if (e->type == CFG_BOOL)
	{
		DecalFont_DrawLineOT(*(bool *)e->valuePtr ? "ON" : "OFF",
			CONFIG_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
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
		DecalFont_DrawLineOT((char *)name, CONFIG_VALUE_X, y, FONT_SMALL, JUSTIFY_RIGHT | WHITE, ot);
	}
	else
	{
		Config_DrawPercent(*(int *)e->valuePtr, y, ot);
	}
}

static void MM_MenuProc_Config(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	uint32_t *ot = gGT->backBuffer->otMem.uiOT;
	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];

	if (s_numSections == 0)
		BuildSectionMap();

	// Inside the Controls section the back button belongs to player 1's device and the
	// selected player's own, so read it through the same union that screen uses.
	const bool inControls = Config_IsCustomSection(s_currentSection);
	const u32 backTapped = inControls ? MM_Controls_MenuTapped() : (u32)pad->buttonsTapped;

	// The device picker consumes back itself, so it must close before this file acts,
	// otherwise one press would close the picker and leave the section in the same
	// frame.
	if (!MM_Controls_IsPickerOpen() && ((backTapped & (BTN_TRIANGLE | BTN_START)) != 0))
	{
		OtherFX_Play(2, 1);

		// A rebind in progress would keep eating input on the next screen.
		NativeControls_CancelCapture();

		if (s_currentSection >= 0)
		{
			menu->rowSelected = s_currentSection;
			s_currentSection = -1;
		}
		else
		{
			NativeConfig_Save();
			NativeControls_Save();
			sdata->ptrDesiredMenu = &D230.menuMainMenu;
		}
	}

	if (s_currentSection >= 0)
	{
		if (Config_IsCustomSection(s_currentSection))
		{
			// Draws and edits everything itself, including its own input.
			MM_MenuProc_Controls(menu);
			return;
		}

		const int sec = s_currentSection;
		const int numRows = s_sectionCount[sec];
		const int firstEntry = s_sectionToEntry[sec];

		if ((pad->buttonsTapped & BTN_UP) != 0)
		{
			if (menu->rowSelected > 0)
			{
				menu->rowSelected--;
				if (menu->rowSelected < s_scrollOffset)
					s_scrollOffset--;
			}
			else
			{
				menu->rowSelected = numRows - 1;
				s_scrollOffset = numRows > CONFIG_MAX_VISIBLE_ROWS ? numRows - CONFIG_MAX_VISIBLE_ROWS : 0;
			}
			OtherFX_Play(0, 1);
		}
		if ((pad->buttonsTapped & BTN_DOWN) != 0)
		{
			if (menu->rowSelected < numRows - 1)
			{
				menu->rowSelected++;
				if (menu->rowSelected >= s_scrollOffset + CONFIG_MAX_VISIBLE_ROWS)
					s_scrollOffset++;
			}
			else
			{
				menu->rowSelected = 0;
				s_scrollOffset = 0;
			}
			OtherFX_Play(0, 1);
		}

		if ((pad->buttonsTapped & (BTN_CROSS | BTN_CIRCLE)) != 0)
		{
			OtherFX_Play(1, 1);
			const ConfigEntry *e = &g_configEntries[firstEntry + menu->rowSelected];
			if (e->type == CFG_BOOL)
			{
				*(bool *)e->valuePtr ^= 1;
				NativeConfig_ApplyDependencies(e);
				// Rebuild section map if visibility changed (e.g. Developer Hacks toggle)
				BuildSectionMap();
				// Turning the gate off hides the section we are standing in, so the
				// cached index no longer names a section. Fall back to the section
				// list instead of reading past the entry table.
				if (s_currentSection >= s_numSections)
				{
					s_currentSection = -1;
					return;
				}
			}
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
			0x100, CONFIG_TITLE_Y, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);

		int visibleEnd = numRows;
		if (visibleEnd > s_scrollOffset + CONFIG_MAX_VISIBLE_ROWS)
			visibleEnd = s_scrollOffset + CONFIG_MAX_VISIBLE_ROWS;

		for (int j = s_scrollOffset; j < visibleEnd; j++)
		{
			const ConfigEntry *e = &g_configEntries[firstEntry + j];
			int y = CONFIG_ROW_START_Y + (j - s_scrollOffset) * CONFIG_ROW_SPACING;

			DecalFont_DrawLineOT((char *)e->label, CONFIG_LABEL_X, y, FONT_SMALL, ORANGE, ot);
			Config_DrawValue(e, y, ot);

			if (j == menu->rowSelected)
				Config_DrawRowHighlight(gGT, ot, y);
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
			s_scrollOffset = 0;
		}

		// If visibility changes (e.g. cheat enabled), rebuild map
		if (s_numSections == 0 || (sdata->frameCounter % 30) == 0)
		{
			BuildSectionMap();
			if (menu->rowSelected >= s_numSections && s_numSections > 0)
				menu->rowSelected = s_numSections - 1;
		}

		DecalFont_DrawLineOT(sdata->lngStrings[LNG_OPTIONS],
			0x100, CONFIG_TITLE_Y, FONT_BIG, JUSTIFY_CENTER | ORANGE, ot);

		for (int i = 0; i < s_numSections; i++)
		{
			const ConfigEntry *e = &g_configEntries[s_sectionToEntry[i]];
			int y = CONFIG_ROW_START_Y + i * CONFIG_ROW_SPACING;
			DecalFont_DrawLineOT((char *)e->section, CONFIG_LABEL_X, y, FONT_SMALL, ORANGE, ot);
			if (i == menu->rowSelected)
				Config_DrawRowHighlight(gGT, ot, y);
		}
	}

	Config_DrawFrame(ot);
}

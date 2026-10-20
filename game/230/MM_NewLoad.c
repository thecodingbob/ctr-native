#include <common.h>

extern void MM_ChaosConfig_OnEnter(void);

static void MM_Menu_Attach(struct RectMenu *parent, struct RectMenu *child)
{
	parent->ptrNextBox_InHierarchy = child;
	parent->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
	child->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	child->ptrPrevBox_InHierarchy = parent;
	child->ptrNextBox_InHierarchy = NULL;
}

static void MM_Menu_Detach(struct RectMenu *child)
{
	struct RectMenu *parent = child->ptrPrevBox_InHierarchy;
	child->ptrNextBox_InHierarchy = NULL;
	child->ptrPrevBox_InHierarchy = NULL;
	if (parent != NULL)
	{
		parent->ptrNextBox_InHierarchy = NULL;
		parent->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	}
}

static bool MM_NewLoad_YieldsToChaosSetup(void)
{
	if (!sdata->chaosSettings.armed || sdata->newLoad.chaosSetupOffered)
	{
		return false;
	}
	sdata->newLoad.chaosSetupOffered = true;
	sdata->ptrDesiredMenu = &MM_MENU_CHAOS_CONFIG;
	MM_ChaosConfig_OnEnter();
	return true;
}

static struct RectMenu *MM_NewLoad_Screen(int step)
{
	switch (step)
	{
	case NEWLOAD_STEP_CHAOS:
		return g_config.chaosMode ? &MM_MENU_CHAOS_PROMPT : NULL;
	case NEWLOAD_STEP_PLAYERS:
		return g_config.multiplayerAdventure ? &MM_MENU_PLAYERS_1P2P : NULL;
	default:
		return NULL;
	}
}

static struct RectMenu *MM_NewLoad_StepAway(int from, int walk)
{
	for (int step = from; step > NEWLOAD_STEP_NONE && step < NEWLOAD_STEP_COUNT; step += walk)
	{
		struct RectMenu *screen = MM_NewLoad_Screen(step);
		if (screen != NULL)
		{
			sdata->newLoad.step = (enum NewLoadStep)step;
			return screen;
		}
	}
	return NULL;
}

// Clear every link so a later visit cannot form a cycle through a stale child.
static void MM_NewLoad_Unwind(void)
{
	sdata->newLoad.chaosSetupOffered = false;
	for (int step = sdata->newLoad.step; step > NEWLOAD_STEP_NONE; step--)
	{
		struct RectMenu *screen = MM_NewLoad_Screen(step);
		struct RectMenu *parent = sdata->newLoad.parentOf[step];
		sdata->newLoad.parentOf[step] = NULL;
		if (parent != NULL)
		{
			parent->ptrNextBox_InHierarchy = NULL;
			parent->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		}
		if (screen != NULL)
		{
			screen->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
			screen->ptrNextBox_InHierarchy = NULL;
			screen->ptrPrevBox_InHierarchy = NULL;
		}
	}
	sdata->newLoad.step = NEWLOAD_STEP_NONE;
}

static void MM_NewLoad_Exit(void)
{
	// Settings take over as the active menu while the chain remains linked underneath.
	if (MM_NewLoad_YieldsToChaosSetup())
	{
		return;
	}
	// The garage does not use the character-select return path.
	sdata->newLoad.previousMenu = (g_config.extendedAdventureCharacterSelect && sdata->newLoad.step != NEWLOAD_STEP_NONE)
	                                  ? MM_NewLoad_Screen(sdata->newLoad.step) : NULL;
	MM_NewLoad_Unwind();
	if (sdata->ptrActiveMenu != &MM_MENU_MAIN)
	{
		sdata->ptrDesiredMenu = &MM_MENU_MAIN;
	}
	MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;
	MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_ADV_NEW;
	MM_MENU_ADVENTURE.state |= ONLY_DRAW_TITLE;
}

static bool MM_NewLoad_InChain(void)
{
	return sdata->newLoad.step != NEWLOAD_STEP_NONE;
}

static void MM_NewLoad_Begin(struct RectMenu *parent)
{
	sdata->newLoad = (struct NewLoadState){0};
	// Every new file starts Regular with fresh defaults, even when the prompt is disabled.
	GAMEPROG_ResetChaosSettings(&sdata->chaosSettings);
	struct RectMenu *screen = MM_NewLoad_StepAway(NEWLOAD_STEP_CHAOS, 1);
	if (screen == NULL)
	{
		MM_NewLoad_Exit();
		return;
	}
	sdata->newLoad.parentOf[sdata->newLoad.step] = parent;
	screen->rowSelected = 0;
	MM_Menu_Attach(parent, screen);
}

void MM_NewLoad_Advance(struct RectMenu *screen)
{
	struct RectMenu *next = MM_NewLoad_StepAway(sdata->newLoad.step + 1, 1);
	if (next == NULL)
	{
		MM_NewLoad_Exit();
		return;
	}
	sdata->newLoad.parentOf[sdata->newLoad.step] = screen;
	next->rowSelected = 0;
	MM_Menu_Attach(screen, next);
}

void MM_NewLoad_Retreat(struct RectMenu *screen)
{
	MM_Menu_Detach(screen);
	struct RectMenu *previous = MM_NewLoad_StepAway(sdata->newLoad.step - 1, -1);
	if (previous == NULL)
	{
		sdata->newLoad.step = NEWLOAD_STEP_NONE;
		return;
	}
	struct RectMenu *parent = sdata->newLoad.parentOf[sdata->newLoad.step];
	if (parent == NULL)
	{
		sdata->newLoad.step = NEWLOAD_STEP_NONE;
		return;
	}
	MM_Menu_Attach(parent, previous);
}

void MM_NewLoad_FinishChaosSetup(void)
{
	sdata->newLoad.chaosSetupOffered = true;
	MM_NewLoad_Exit();
}

void MM_NewLoad_ReturnFromChaosSetup(void)
{
	sdata->newLoad.chaosSetupOffered = false;
	sdata->ptrDesiredMenu = &MM_MENU_MAIN;
}

static void MM_NewLoad_ResumeFromCharacterSelect(struct RectMenu *previous)
{
	int lastStep = NEWLOAD_STEP_NONE;
	for (int step = NEWLOAD_STEP_CHAOS; step < NEWLOAD_STEP_COUNT; step++)
	{
		if (MM_NewLoad_Screen(step) == previous)
		{
			lastStep = step;
			break;
		}
	}
	if (lastStep == NEWLOAD_STEP_NONE)
	{
		return;
	}
	sdata->newLoad.chaosSetupOffered = false;
	// Rebuild every ancestor: retreat needs each step's original parent.
	struct RectMenu *parent = &MM_MENU_ADVENTURE;
	for (int step = NEWLOAD_STEP_CHAOS; step <= lastStep; step++)
	{
		struct RectMenu *screen = MM_NewLoad_Screen(step);
		if (screen == NULL)
		{
			continue;
		}
		sdata->newLoad.parentOf[step] = parent;
		sdata->newLoad.step = (enum NewLoadStep)step;
		// These choices survive overlay resets in the profile's live settings.
		screen->rowSelected = (step == NEWLOAD_STEP_CHAOS)
		                          ? !sdata->chaosSettings.armed
		                          : sdata->advMultiplayer.numPlayers - 1;
		MM_Menu_Attach(parent, screen);
		parent = screen;
	}
}

void MM_NewLoad_OnCharacterSelectExit(bool advance)
{
	struct RectMenu *previous = sdata->newLoad.previousMenu;
	sdata->newLoad.previousMenu = NULL;

	if (advance)
	{
		return;
	}

	MM_JumpTo_Title_Returning();
	if (previous != NULL)
	{
		MM_NewLoad_ResumeFromCharacterSelect(previous);
	}
}

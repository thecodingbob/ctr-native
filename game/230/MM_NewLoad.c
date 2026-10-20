#include <common.h>

extern struct RectMenu MM_menuChaosConfig;
extern void MM_ChaosConfig_OnEnter(void);

static void MM_Menu_Attach(struct RectMenu *parent, struct RectMenu *child)
{
	parent->ptrNextBox_InHierarchy = child;
	parent->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
	child->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	child->ptrNextBox_InHierarchy = NULL;
	child->rowSelected = 0;
}

static void MM_Menu_Detach(struct RectMenu *child)
{
	struct RectMenu *parent = child->ptrPrevBox_InHierarchy;
	child->ptrNextBox_InHierarchy = NULL;
	child->ptrPrevBox_InHierarchy = NULL;
	if (parent != NULL)
	{
		parent->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	}
}

enum NewLoadStep
{
	NEWLOAD_STEP_CHAOS,
	NEWLOAD_STEP_PLAYERS,
	NEWLOAD_STEP_COUNT,
};

static int s_newLoadStep = -1;
static bool s_chaosSetupOffered;
static struct RectMenu *s_previousMenu;
static struct RectMenu *s_newLoadParentOf[NEWLOAD_STEP_COUNT];

static bool MM_NewLoad_YieldsToChaosSetup(void)
{
	if (!sdata->chaosSettings.armed || s_chaosSetupOffered)
	{
		return false;
	}
	s_chaosSetupOffered = true;
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
	for (int step = from; (step >= 0) && (step < NEWLOAD_STEP_COUNT); step += walk)
	{
		struct RectMenu *screen = MM_NewLoad_Screen(step);
		if (screen != NULL)
		{
			s_newLoadStep = step;
			return screen;
		}
	}
	return NULL;
}

// Clear every link so a later visit cannot form a cycle through a stale child.
static void MM_NewLoad_Unwind(void)
{
	s_chaosSetupOffered = false;
	for (int step = s_newLoadStep; step >= 0; step--)
	{
		struct RectMenu *screen = MM_NewLoad_Screen(step);
		struct RectMenu *parent = s_newLoadParentOf[step];
		s_newLoadParentOf[step] = NULL;
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
	s_newLoadStep = -1;
}

static void MM_NewLoad_Exit(void)
{
	// Settings take over as the active menu while the chain remains linked underneath.
	if (MM_NewLoad_YieldsToChaosSetup())
	{
		return;
	}
	s_previousMenu = (s_newLoadStep >= 0) ? MM_NewLoad_Screen(s_newLoadStep) : NULL;
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
	return s_newLoadStep >= 0;
}

static void MM_NewLoad_Begin(struct RectMenu *parent)
{
	s_newLoadStep = -1;
	s_chaosSetupOffered = false;
	s_previousMenu = NULL;
	// Every new file starts Regular with fresh defaults, even when the prompt is disabled.
	GAMEPROG_ResetChaosSettings(&sdata->chaosSettings);
	struct RectMenu *screen = MM_NewLoad_StepAway(s_newLoadStep + 1, 1);
	if (screen == NULL)
	{
		MM_NewLoad_Exit();
		return;
	}
	s_newLoadParentOf[s_newLoadStep] = parent;
	MM_Menu_Attach(parent, screen);
}

void MM_NewLoad_Advance(struct RectMenu *screen)
{
	struct RectMenu *next = MM_NewLoad_StepAway(s_newLoadStep + 1, 1);
	if (next == NULL)
	{
		if (MM_NewLoad_YieldsToChaosSetup())
		{
			return;
		}
		MM_Menu_Detach(screen);
		MM_NewLoad_Exit();
		return;
	}
	s_newLoadParentOf[s_newLoadStep] = screen;
	MM_Menu_Attach(screen, next);
}

void MM_NewLoad_Retreat(struct RectMenu *screen)
{
	MM_Menu_Detach(screen);
	struct RectMenu *previous = MM_NewLoad_StepAway(s_newLoadStep - 1, -1);
	if (previous == NULL)
	{
		s_newLoadStep = -1;
		return;
	}
	struct RectMenu *parent = s_newLoadParentOf[s_newLoadStep];
	if (parent == NULL)
	{
		s_newLoadStep = -1;
		return;
	}
	MM_Menu_Attach(parent, previous);
}

void MM_NewLoad_FinishChaosSetup(void)
{
	s_chaosSetupOffered = true;
	MM_NewLoad_Exit();
}

void MM_NewLoad_ReturnFromChaosSetup(void)
{
	s_chaosSetupOffered = false;
	sdata->ptrDesiredMenu = &MM_MENU_MAIN;
}

struct RectMenu *MM_NewLoad_PreviousMenu(void)
{
	struct RectMenu *previous = s_previousMenu;
	s_previousMenu = NULL;
	return previous;
}

void MM_NewLoad_ResumeFromCharacterSelect(struct RectMenu *previous)
{
	MM_JumpTo_Title_Returning();
	int lastStep = -1;
	for (int step = NEWLOAD_STEP_CHAOS; step < NEWLOAD_STEP_COUNT; step++)
	{
		if (MM_NewLoad_Screen(step) == previous)
		{
			lastStep = step;
			break;
		}
	}
	if (lastStep < 0)
	{
		return;
	}
	s_chaosSetupOffered = false;
	// Rebuild every ancestor: retreat needs each step's original parent.
	struct RectMenu *parent = &MM_MENU_ADVENTURE;
	for (int step = NEWLOAD_STEP_CHAOS; step <= lastStep; step++)
	{
		struct RectMenu *screen = MM_NewLoad_Screen(step);
		if (screen == NULL)
		{
			continue;
		}
		s_newLoadParentOf[step] = parent;
		s_newLoadStep = step;
		MM_Menu_Attach(parent, screen);
		parent = screen;
	}
}

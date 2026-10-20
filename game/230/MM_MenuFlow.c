#include <common.h>
#include <stdlib.h>

extern struct MenuRow s_rowsMainMenuBasicConfig[];
extern struct MenuRow s_rowsMainMenuWithSBConfig[];
extern struct RectMenu g_configMenu;

// Defined in MM_ChaosConfig.c, which 230.c includes after this file.
extern struct RectMenu MM_menuChaosConfig;
extern void MM_ChaosConfig_OnEnter(void);

// ================= cascading menu navigation =================

// The only two places a menu box is linked to another or unlinked.

static void MM_Menu_Attach(struct RectMenu *parent, struct RectMenu *child)
{
	parent->ptrNextBox_InHierarchy = child;
	parent->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;

	// A screen that has just been linked is by definition the deepest one, so it
	// must not be left collapsed from a time when it had a child of its own. Only
	// RECTMENU_ProcessState clears this, and only on the frame it promotes a menu.
	child->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	child->ptrNextBox_InHierarchy = NULL;

	// Re-entering a screen starts it at its first row.
	child->rowSelected = 0;
}

// Unlinks child from whatever it hangs off, leaving its parent holding whatever it
// had before. The child's own links go either way: a stale ptrNextBox_InHierarchy
// is what lets two menus end up pointing at each other, and both RECTMENU_DrawSelf
// and RECTMENU_ProcessInput follow it for as long as it is set.
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

// The screens between the Adventure new/load menu and a new race, in order. Each
// is optional and each is gated on its own option, so "New" walks this instead of
// each screen hardcoding what comes after it. That is what lets the player back
// out of the racer-count screen onto the Chaos prompt rather than over it.
enum NewLoadStep
{
	NEWLOAD_STEP_CHAOS,
	NEWLOAD_STEP_PLAYERS,
	NEWLOAD_STEP_COUNT,
};

// Which step is in hand. -1 means the new/load menu itself, which is where the
// chain starts and where it lands when there is nothing behind it.
static int s_newLoadStep = -1;

// Whether the Chaos setup screens have already been offered for the file being created.
//
// The chain hands over to them on its way out, so without this a file created with Chaos
// would be offered the settings again every time the chain closed, and finishing the
// settings would hand back to itself forever.
static bool s_chaosSetupOffered;

// The menu the chain was on when it closed, which is where the player came from and so
// where backing out of the next screen has to land. NULL when no chain was involved.
static struct RectMenu *s_previousMenu;

// A file being created with Chaos still has its settings to pick, and those screens are not
// steps of the chain. They are promoted to be the active menu instead, and the chain is left
// linked up underneath, for two reasons: the screens draw themselves over everything only
// while nothing else is the active menu, and coming back from them has to land on the step
// the player last answered rather than on the title.
//
// Recorded once per visit, which is what keeps MM_NewLoad_FinishChaosSetup from handing back
// to itself, and which is cleared when the chain is rewound or the settings are left.
static bool MM_NewLoad_YieldsToChaosSetup(void)
{
	if (!sdata->chaosArmed || s_chaosSetupOffered)
	{
		return false;
	}

	s_chaosSetupOffered = true;

	sdata->ptrDesiredMenu = &MM_MENU_CHAOS_CONFIG;
	MM_ChaosConfig_OnEnter();

	return true;
}

// What each step hangs off. Backing out has to put a step back where *it* was, not
// where the step being left hangs off, which is a different menu once there is more
// than one step: the racer count hangs off the Chaos prompt, not off the new/load
// menu.
static struct RectMenu *s_newLoadParentOf[NEWLOAD_STEP_COUNT];

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

// The first available step walking away from `from`, which is inclusive, or NULL
// when the chain has run out in that direction. Steps whose option is off are
// skipped rather than stopping the walk, so a missing step needs no special case
// anywhere. Moves the chain onto whatever it finds.
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

// Undoes every link the chain made and leaves each step as it was found.
//
// Complete, not a flag. MM_ResetAllMenus runs only on the first entry to the title
// (MM_JumpTo_Title_FirstTime) and never on a return, so a link left behind survives
// into the next visit of the menu, where attaching a step to a step that still had
// the old one as its child leaves the two pointing at each other — an infinite
// recursion in RECTMENU_DrawSelf (`:656`) and RECTMENU_ProcessInput (`:877`), and
// an immediate crash.
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

// The chain is finished: leave the title the way "New" always did.
static void MM_NewLoad_Exit(void)
{
	// Handing over is not finishing: the settings screens take over as the active menu and
	// the chain stays as it is underneath them.
	if (MM_NewLoad_YieldsToChaosSetup())
	{
		return;
	}

	// Character select is the next screen, and backing out of it lands back on whichever
	// step this chain ended on. Read before the unwind, which is what clears the step.
	s_previousMenu = (s_newLoadStep >= 0) ? MM_NewLoad_Screen(s_newLoadStep) : NULL;

	MM_NewLoad_Unwind();

	// The Chaos screens and the racer count both ran as menus in their own right, so the
	// title menu may not be the active one any more. Nothing drives the fade-out below
	// until it is, so hand it back before asking to leave.
	if (sdata->ptrActiveMenu != &MM_MENU_MAIN)
	{
		sdata->ptrDesiredMenu = &MM_MENU_MAIN;
	}

	MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;
	MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_ADV_NEW;

	// The new/load menu keeps its title bar, as it did when New left the title
	// directly, so its rows do not redraw under the fade out.
	MM_MENU_ADVENTURE.state |= ONLY_DRAW_TITLE;
}

static bool MM_NewLoad_InChain(void)
{
	return s_newLoadStep >= 0;
}

// Entering the chain, from "New" on the new/load menu.
static void MM_NewLoad_Begin(struct RectMenu *parent)
{
	s_newLoadStep = -1;
	s_chaosSetupOffered = false;
	s_previousMenu = NULL;

	// The answer for a file being created starts as Regular, and only answering Chaos on
	// the prompt turns it on. Without this the answer survives from whatever the last file
	// in the session left behind: with the prompt turned off nothing ever writes it, so a
	// file created with Chaos earlier would still be offered the settings, and its card
	// slot would still be marked.
	sdata->chaosArmed = false;

	struct RectMenu *screen = MM_NewLoad_StepAway(s_newLoadStep + 1, 1);

	if (screen == NULL)
	{
		MM_NewLoad_Exit();
		return;
	}

	s_newLoadParentOf[s_newLoadStep] = parent;
	MM_Menu_Attach(parent, screen);
}

// A step in the chain was answered: show the next one below it, or leave the title
// when there is none.
void MM_NewLoad_Advance(struct RectMenu *screen)
{
	struct RectMenu *next = MM_NewLoad_StepAway(s_newLoadStep + 1, 1);

	if (next == NULL)
	{
		// Handing over leaves the chain alone: the screen being left stays attached, which
		// is what keeps the step the player just answered as the one they come back to.
		// Only a real exit unlinks it.
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

// A step in the chain was backed out of: put the step before it back where it was,
// or leave the new/load menu alone when it is already behind it.
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

	// A step with no parent recorded cannot be put back anywhere, and handing a NULL
	// parent to MM_Menu_Attach would be a write through it. Treat it as leaving the chain,
	// which is where the screen behind it was already going.
	if (parent == NULL)
	{
		s_newLoadStep = -1;
		return;
	}

	MM_Menu_Attach(parent, previous);
}

// The Chaos settings have been picked: let the chain close for real.
void MM_NewLoad_FinishChaosSetup(void)
{
	// Recorded before the close, because the close is what would otherwise hand back here.
	s_chaosSetupOffered = true;

	MM_NewLoad_Exit();
}

// The Chaos settings were left before they were finished, so the player can still change
// their mind.
//
// The title menu is put back rather than one of the chain's steps promoted. The chain was
// handed over before MM_NewLoad_Unwind ran, so it is still linked up, and restoring the
// title brings the whole hierarchy with it: the step that was last answered ends up in
// front, and the title transitions put it where it belongs. Promoting a step on its own
// would draw it in the top left, because the chain's boxes are positioned by the title
// update and never meant to stand alone.
void MM_NewLoad_ReturnFromChaosSetup(void)
{
	// Cleared so that answering the step again offers the settings again, which is the
	// point of coming back: the player is here because they want to change something.
	s_chaosSetupOffered = false;

	sdata->ptrDesiredMenu = &MM_MENU_MAIN;
}

// Which screen the player came from before character select, or NULL when no new-file
// chain was involved. Read once and consumed, so a character select reached any other way
// later in the session cannot inherit the answer.
struct RectMenu *MM_NewLoad_PreviousMenu(void)
{
	struct RectMenu *previous = s_previousMenu;

	s_previousMenu = NULL;

	return previous;
}

// Coming back from character select after the new-file chain.
//
// Retail returns to the title from here, which used to land on the new/load menu because
// the chain's last step did not exist yet. With the chain, that last step is a screen the
// player was just on, and skipping it throws it away, so the chain is rebuilt up to it.
//
// The chain is closed by the time this runs, so it is rebuilt here by walking forward the
// way MM_NewLoad_Begin does. Every step has to be rebuilt, not just the last one:
// MM_NewLoad_Unwind cleared s_newLoadParentOf, and rebuilding only the last step would
// leave the step before it with no parent to hang off, which is a NULL write the first
// time the player backs out of it.
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
		// Not a step of the chain, or the option that put it there is off now. The title is
		// where the return belongs, exactly as it was before the chain existed.
		return;
	}

	s_chaosSetupOffered = false;

	// Same walk as MM_NewLoad_Begin: each step hangs off the one before it, and only the
	// steps the chain would have walked are rebuilt, so a config with the prompt turned
	// off does not get one back.
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

s32 MM_TransitionInOut(struct TransitionMeta *meta, s32 framesPassed, s32 numFrames)
{
	s16 negativeTransitionState;
	s16 allTransitionsDone;
	u16 transitionIndex;
	s32 transitionFrame;
	s16 start;
	s16 framesLeft;

	allTransitionsDone = 1;
	negativeTransitionState = 0;
	transitionIndex = 0;
	start = meta->headStart;
	transitionFrame = framesPassed;

	// last member of array is null-terminated with 0xFFFF
	for (/**/; start > -1; meta++, transitionIndex++, start = meta->headStart)
	{
		framesLeft = (s16)transitionFrame - start;

		if ((framesLeft == MM_TRANSITION_SWISH_FRAME) && (transitionIndex == 0))
		{
			// Play "swoosh" sound for menu transition
			OtherFX_Play(MM_TRANSITION_SWISH_SFX, 0);
		}

		if (framesLeft < 1)
		{
			allTransitionsDone = 0;
			meta->currX = 0;
			meta->currY = 0;
		}
		else if (framesLeft >= (s16)numFrames)
		{
			negativeTransitionState = 0;
			meta->currX = meta->distX;
			meta->currY = meta->distY;
		}
		else
		{
			allTransitionsDone = 0;
			negativeTransitionState = allTransitionsDone;
			meta->currX = framesLeft * meta->distX / (s16)numFrames;
			meta->currY = framesLeft * meta->distY / (s16)numFrames;
		}
	}
	return allTransitionsDone ? 1 : (negativeTransitionState ? -1 : 0);
}

void MM_MenuProc_Main(struct RectMenu *menu)
{
	register u32 timeTrialSetupTracker CTR_PSX_REGISTER("$3");
	register struct RectMenu *mainMenu CTR_PSX_REGISTER("$16");
	struct Title *titleObj;
	s16 choose;

	mainMenu = menu;

	// if scrapbook is unlocked, change "rows" to extended array
	if ((MM_GAME_UNLOCKS[MEMCARD_BIT_WORD(GAME_UNLOCK_BIT_SCRAPBOOK)] & MEMCARD_BIT_MASK(GAME_UNLOCK_BIT_SCRAPBOOK)) != 0)
	{
	  mainMenu->rows = &s_rowsMainMenuWithSBConfig[0];
	} else
	{
	  mainMenu->rows = &s_rowsMainMenuBasicConfig[0];
	}

	MM_ParseCheatCodes();
	MM_QuickRace_RequestStart(mainMenu);
	MM_ToggleRows_Difficulty();
	MM_ToggleRows_PlayerCount();

	// If you are at the highest hierarchy level of main menu
	if (mainMenu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
		MM_Title_MenuUpdate();

		if (
		    // main menu, "title" exists, and timer >= 230
		    (MM_TITLE_MENU_STATE == TITLE_MENU_STATE_IN_MENU) && (MM_TITLE_OBJECT != NULL) && (TITLE_INTRO_TM_DRAW_MIN_FRAME < (s16)MM_TITLE_INTRO_FRAME))
		{
			DecalFont_DrawLineOT(GAME_LANGUAGE_STRINGS[LNG_TM], MM_TITLE_TM_X, MM_TITLE_TM_Y, FONT_SMALL, ORANGE,
			                     &GAME_TRACKER->backBuffer->otMem.uiOT[MM_TITLE_TM_OT_INDEX]);
		}

		if ((MM_MENU_MAIN.state & DRAW_NEXT_MENU_IN_HIERARCHY) == 0)
		{
			GAME_TRACKER->numPlyrNextGame = 1;

			// if button pressed, reset timer
			if (GAMEPADS->anyoneHeldCurr != 0)
			{
				GAME_TRACKER->demoCountdownTimer = TITLE_DEMO_IDLE_FRAMES;
			}

			// if no buttons pressed, check demo mode
			else
			{
				GAME_TRACKER->demoCountdownTimer--;

				// If time runs out
				if (GAME_TRACKER->demoCountdownTimer < 1)
				{
					// Transition out of main menu
					MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;

					// Go to a cutscene of some kind, either the Oxide intro
					// or a demo-mode race.
					MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_DEMO;
				}
			}
		}
	}

	MM_Title_Init();

	// if drawing ptrNextBox_InHierarchy
	if ((mainMenu->state & DRAW_NEXT_MENU_IN_HIERARCHY) != 0)
	{
		MM_TITLE_INTRO_FRAME = TITLE_INTRO_SKIP_FRAME;
	}

	// if funcPtr is null
	if ((mainMenu->state & EXECUTE_FUNCPTR) == 0)
	{
		return;
	}

	titleObj = MM_TITLE_OBJECT;

	// if "title" object exists
	if (titleObj != NULL)
	{
		// CameraPosOffset X
		titleObj->cameraPosOffset.x = 0;
	}

	// if you are at highest level of menu hierarchy
	if (mainMenu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		// leave the function
		return;
	}

	// If you are here, then you must not be
	// at the highest level of menu hierarchy

	// if row is negative, do nothing
	if (mainMenu->rowSelected < 0)
	{
		return;
	}
	// clear flags from game mode
	GAME_TRACKER->gameMode1 &= ~(BATTLE_MODE | ADVENTURE_MODE | TIME_TRIAL | ADVENTURE_ARENA | ARCADE_MODE | ADVENTURE_CUP);

	// clear more game mode flags
	GAME_TRACKER->gameMode2 &= ~(CUP_ANY_KIND);

	mainMenu->state |= ONLY_DRAW_TITLE;

	// Default to 3,
	// this intentionally disables the 1-lap cheat
	// in Time Trial and Adventure, DONT change it
	GAME_TRACKER->numLaps = MM_DEFAULT_LAP_COUNT;

	// get LNG index of row selected
	choose = mainMenu->rows[mainMenu->rowSelected].stringIndex;

	switch (choose)
	{
	// Adventure Mode
	case LNG_ADVENTURE:
	{
		struct GameTracker *adventureTracker;

		adventureTracker = GAME_TRACKER;

		// Turn on Adventure Mode
		adventureTracker->gameMode1 |= ADVENTURE_MODE;

		// menu for new/load
		mainMenu->ptrNextBox_InHierarchy = &MM_MENU_ADVENTURE;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;

		// Turn off item cheats
		adventureTracker->gameMode2 &= ~CHEAT_WUMPA;
		adventureTracker->gameMode2 &= ~CHEAT_TURBO;
		adventureTracker->gameMode2 &= ~CHEAT_MASK;
		adventureTracker->gameMode2 &= ~CHEAT_ENGINE;
		adventureTracker->gameMode2 &= ~CHEAT_BOMBS;
		return;
	}

	// Time Trial
	case LNG_TIME_TRIAL:
		timeTrialSetupTracker = (u32)GAME_TRACKER;
		{
			// Leave main menu hierarchy
			MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;

			// Leave through the normal character-select flow.
			MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_CHARACTER_SELECT;

			// set game mode to Time Trial Mode
			((struct GameTracker *)timeTrialSetupTracker)->numPlyrNextGame = 1;
			((struct GameTracker *)timeTrialSetupTracker)->gameMode1 |= TIME_TRIAL;
			GAME_TRACKER->gameMode2 &= ~CHEAT_WUMPA;
			GAME_TRACKER->gameMode2 &= ~CHEAT_TURBO;
			GAME_TRACKER->gameMode2 &= ~CHEAT_MASK;
			GAME_TRACKER->gameMode2 &= ~CHEAT_ENGINE;
			GAME_TRACKER->gameMode2 &= ~CHEAT_BOMBS;

			return;
		}

	// Arcade Mode
	case LNG_ARCADE:
	{
		// DONT change, should only work in Arcade, and VS
		if ((GAME_TRACKER->gameMode2 & CHEAT_ONELAP) != 0)
		{
			GAME_TRACKER->numLaps = MM_ONE_LAP_CHEAT_COUNT;
		}

		// set game mode to Arcade Mode
		GAME_TRACKER->gameMode1 |= ARCADE_MODE;

		// set next menu
		mainMenu->ptrNextBox_InHierarchy = &MM_MENU_RACE_TYPE;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// Versus
	case LNG_VS:
	{
		// DONT change, should only work in Arcade, and VS
		if ((GAME_TRACKER->gameMode2 & CHEAT_ONELAP) != 0)
		{
			GAME_TRACKER->numLaps = MM_ONE_LAP_CHEAT_COUNT;
		}

		// next menu is choosing single+cup
		mainMenu->ptrNextBox_InHierarchy = &MM_MENU_RACE_TYPE;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}

	// Battle
	case LNG_BATTLE:
	{
		MM_CHARACTER_SELECT_TRANSITION_STATE = EXITING_MENU;

		// set game mode to Battle Mode
		GAME_TRACKER->gameMode1 |= BATTLE_MODE;

		// set next menu to 2P,3P,4P
		mainMenu->ptrNextBox_InHierarchy = &MM_MENU_PLAYERS_2P3P4P;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;
	}
		CTR_PSX_CLOBBER("$7");

	// High Score
	case LNG_HIGH_SCORE:
		// Leave main menu hierarchy
		MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;

		// Set next stage to high score menu
		MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_HIGH_SCORE;
		return;

	// Scrapbook
	case LNG_SCRAPBOOK:
		// Leave main menu hierarchy
		MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;

		// Set next stage to Scrapbook
		MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_SCRAPBOOK;
		return;

	case LNG_OPTIONS:
		sdata->ptrDesiredMenu = &g_configMenu;
		return;

	case LNG_QUIT:
		mainMenu->ptrNextBox_InHierarchy = &D230.menuQuitConfirm;
		mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
		return;

	default:
		return;
	}
}

void MM_MenuProc_QuitConfirm(struct RectMenu *menu)
{
	if (menu->rowSelected < 0)
	{
		MM_Menu_Detach(menu);
		return;
	}
	// Called every frame from RECTMENU_ProcessState with funcState=1;
	// only act on actual button presses (funcState=0 from ProcessInput).
	if (menu->funcState != 0)
		return;

	if (menu->rowSelected == 0) // YES
		exit(0);
	// NO - go back
	MM_Menu_Detach(menu);
}

void MM_ToggleRows_PlayerCount(void)
{
	struct MenuRow *row;
	s16 rowIndex;
	s16 selectablePlayerCountRows = MM_PLAYER_1P2P_SELECTABLE_ROWS;
	const b32 adventurePlayerCountSelection = (GAME_TRACKER->gameMode1 & ADVENTURE_MODE) != 0 && g_config.multiplayerAdventure;
	struct MenuRow *playerCountRows = MM_ROWS_PLAYERS_1P2P;

	if (g_config.extendedMultiplayer && !adventurePlayerCountSelection)
	{
		playerCountRows = MM_ROWS_PLAYERS_1P2P3P4P;
		selectablePlayerCountRows = MM_PLAYER_1P2P3P4P_SELECTABLE_ROWS;
	}

	MM_MENU_PLAYERS_1P2P.rows = playerCountRows;

	for (rowIndex = 0; rowIndex < selectablePlayerCountRows; rowIndex++)
	{
		row = &playerCountRows[rowIndex];

		// unlock row
		row->stringIndex &= MENU_ROW_LNG_MASK;

		if ((s16)MainFrame_HaveAllPads(rowIndex + 1) == 0)
		{
			// lock row
			row->stringIndex |= MENU_ROW_LOCKED;
		}
	}

	for (rowIndex = 0; rowIndex < MM_PLAYER_2P3P4P_SELECTABLE_ROWS; rowIndex++)
	{
		row = &MM_ROWS_PLAYERS_2P3P4P[rowIndex];

		// unlock row
		row->stringIndex &= MENU_ROW_LNG_MASK;

		if ((s16)MainFrame_HaveAllPads(rowIndex + 2) == 0)
		{
			// lock row
			row->stringIndex |= MENU_ROW_LOCKED;
		}
	}
}

void MM_MenuProc_1p2p(struct RectMenu *menu)
{
	s16 selectableRowCount = 0;

	const s16 row = menu->rowSelected;

	// if uninitialized
	if (row == -1)
	{
		goto UNINITIALIZED;
	}

	if (row < -1)
	{
		return;
	}

	while (menu->rows[selectableRowCount].stringIndex != RECTMENU_STRING_NONE)
	{
		selectableRowCount++;
	}

	if (row >= selectableRowCount)
	{
		return;
	}

	// row N is (N+1)P
	GAME_TRACKER->numPlyrNextGame = row + 1;

	// The same screen is reached from the main menu, where it leads to the
	// difficulty box, and from the new-file chain, where answering it just moves
	// to the next step.
	if (MM_NewLoad_InChain())
	{
		MM_NewLoad_Advance(menu);
		return;
	}

	// go to difficulty box
	menu->ptrNextBox_InHierarchy = &MM_MENU_DIFFICULTY;

	menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;
	return;

UNINITIALIZED:
	if (MM_NewLoad_InChain())
	{
		// "Previous" from a step of the new-file chain, which is the screen before
		// it,  not the new/load menu it came off.
		MM_NewLoad_Retreat(menu);
	}
	else
	{
		MM_Menu_Detach(menu);
	}

	GAME_TRACKER->numPlyrNextGame = 1;

	MM_CHARACTER_SELECT_TRANSITION_STATE = ENTERING_MENU;
	return;
}

void MM_MenuProc_2p3p4p(struct RectMenu *menu)
{
	register struct RectMenu *menuOwner CTR_PSX_REGISTER("$4") = menu;
	struct RectMenu *selectedMenu;
	s16 row;

	selectedMenu = menu;
	row = selectedMenu->rowSelected;

	// if uninitialized
	if (row == -1)
	{
		goto UNINITIALIZED;
	}

	if (row < -1)
	{
		return;
	}

	// row is 0, 1, 2
	if (row >= MM_PLAYER_2P3P4P_SELECTABLE_ROWS)
	{
		return;
	}

	// row 0 is 2P, row 1 is 3P, row 2 is 4P
	GAME_TRACKER->numPlyrNextGame = selectedMenu->rowSelected + 2;

	MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;
	MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_CHARACTER_SELECT;

	selectedMenu->state |= ONLY_DRAW_TITLE;
	return;

UNINITIALIZED:
	menuOwner->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);

	GAME_TRACKER->numPlyrNextGame = 1;

	MM_CHARACTER_SELECT_TRANSITION_STATE = ENTERING_MENU;
	return;
}

void MM_ToggleRows_Difficulty(void)
{
	struct RectMenu *difficultyMenu;
	s16 *firstUnlockBits;
	struct GameProgress *progress;
	struct GameTracker *gGT;
	b16 shouldCheckNextTrack;
	b16 finalUnlockState;
	s16 difficultyIndex;
	s16 trackIndex;
	s16 *firstUnlockBit;
	s16 *innerFirstUnlockBit;
	s16 *firstUnlockBitCopy;
	struct MenuRow *difficultyRow;
	u32 *unlockWord;
	s32 unlockBit;
	u32 isUnlocked;
	s32 lngIndex;
	s32 signedDifficultyIndex;
	s32 stringIndexOffset;

	difficultyMenu = &MM_MENU_DIFFICULTY;
	difficultyIndex = 0;
	firstUnlockBits = MM_CUP_DIFFICULTY_FIRST_UNLOCK_BIT;
	progress = &GAME_PROGRESS;
	gGT = GAME_TRACKER;

	// check 3 modes (easy, medium, hard)
	for (/**/; difficultyIndex < MM_DIFFICULTY_COUNT; difficultyIndex++)
	{
		firstUnlockBit = (s16 *)((u32)(difficultyIndex * sizeof(*firstUnlockBits)) + (u32)firstUnlockBits);
		firstUnlockBitCopy = firstUnlockBit;

		// if negative (for EASY row), skip
		if (*firstUnlockBit < 0)
		{
			continue;
		}

		// assume unlocked
		shouldCheckNextTrack = 1;
		trackIndex = 0;
		innerFirstUnlockBit = firstUnlockBitCopy;

		// check 4 bits starting at bitIndex,
		// one for each track in cup
		for (/**/; trackIndex < MM_CUP_TRACK_COUNT; trackIndex++, shouldCheckNextTrack = (b16)isUnlocked)
		{
			isUnlocked = 0;

			// if not determined locked
			if (shouldCheckNextTrack)
			{
				unlockBit = (s32)*innerFirstUnlockBit + trackIndex;

				// check what is unlocked
				unlockWord = (u32 *)((u32)((unlockBit >> 5) * sizeof(*unlockWord)) + (u32)progress);
				isUnlocked = (unlockWord[1] >> (unlockBit & 0x1f)) & 1;
			}
		}

		// get current value of lng index,
		// for easy, medium, hard
		signedDifficultyIndex = (s16)difficultyIndex;
		CTR_PSX_KEEP_VALUE(signedDifficultyIndex);
		stringIndexOffset = signedDifficultyIndex * sizeof(s16);
		{
			register s16 *stringIndices CTR_PSX_REGISTER("$2");

			CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(stringIndices, MM_CUP_DIFFICULTY_STRING_INDEX_ASM_NAME, stringIndexOffset);
			CTR_PSX_ADD_SYMBOL_LOW(stringIndices, stringIndices, MM_CUP_DIFFICULTY_STRING_INDEX_ASM_NAME, MM_CUP_DIFFICULTY_STRING_INDEX);
			lngIndex = *(s16 *)((u32)stringIndexOffset + (u32)stringIndices);
		}
		difficultyRow = (struct MenuRow *)((u32)(signedDifficultyIndex * sizeof(*difficultyRow)) + (u32)difficultyMenu->rows);
		finalUnlockState = shouldCheckNextTrack;
		isUnlocked = 0;

		if (
		// if locked
		    ((u32)(u16)finalUnlockState == isUnlocked) &&

		    // If you're in Arcade mode
		    ((gGT->gameMode1 & ARCADE_MODE) != isUnlocked) &&

		    // if you are in Arcade or VS cup
		    ((gGT->gameMode2 & CUP_ANY_KIND) != isUnlocked))
		{
			// use high bits for "LOCKED"
			lngIndex |= MENU_ROW_LOCKED;
		}

		// save new value
		difficultyRow->stringIndex = lngIndex;
	}
}
void MM_MenuProc_Difficulty(struct RectMenu *menu)
{
	register struct RectMenu *menuOwner CTR_PSX_REGISTER("$4") = menu;
	struct RectMenu *selectedMenu;
	s16 row;

	selectedMenu = menu;
	row = selectedMenu->rowSelected;

	// if uninitialized
	if (row == -1)
	{
		goto UNINITIALIZED;
	}

	if (row < -1)
	{
		return;
	}

	// if you are on a valid row
	if (row >= MM_DIFFICULTY_COUNT)
	{
		return;
	}

	// set difficulty to value, from array of fixed difficulty values
	GAME_TRACKER->arcadeDifficulty = MM_CUP_DIFFICULTY_SPEED[row];

	MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;
	MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_CHARACTER_SELECT;

	selectedMenu->state |= ONLY_DRAW_TITLE;
	return;

UNINITIALIZED:
	menuOwner->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
	return;
}

void MM_MenuProc_SingleCup(struct RectMenu *menu)
{
	s16 row;

	row = menu->rowSelected;

	if (row == -1)
	{
		goto UNINITIALIZED;
	}

	if (row < -1)
	{
		return;
	}

	if (row >= MM_RACE_TYPE_SELECTABLE_ROWS)
	{
		return;
	}

	// disable Cup mode
	GAME_TRACKER->gameMode2 &= ~(CUP_ANY_KIND);

	// if you choose cup mode
	if (menu->rowSelected != 0)
	{
		// enable cup mode
		GAME_TRACKER->gameMode2 |= CUP_ANY_KIND;
	}

	menu->state |= ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY;

	// if mode is Arcade
	if ((GAME_TRACKER->gameMode1 & ARCADE_MODE) != 0)
	{
		// set next menu to 1P+2P select
		menu->ptrNextBox_InHierarchy = &MM_MENU_PLAYERS_1P2P;
		MM_CHARACTER_SELECT_TRANSITION_STATE = IN_MENU;
		return;
	}

	// if mode is VS

	// set next menu to 2P+3P+4P (vs or battle)
	menu->ptrNextBox_InHierarchy = &MM_MENU_PLAYERS_2P3P4P;
	MM_CHARACTER_SELECT_TRANSITION_STATE = EXITING_MENU;
	return;

UNINITIALIZED:
	MM_Menu_Detach(menu);
}

void MM_MenuProc_NewLoad(struct RectMenu *menu)
{
	// row number
	s16 row;

	row = menu->rowSelected;

	switch (row)
	{
	case 0:
		// "New" enters the chain; the chain decides which screens are actually
		// in it, from the options.
		MM_NewLoad_Begin(menu);
		return;

	case 1:
		// MM_Title transitioning out
		MM_TITLE_MENU_STATE = TITLE_MENU_STATE_EXITING;

		// Load was chosen
		MM_DESIRED_MENU_INDEX = MM_EXIT_ROUTE_ADV_LOAD;

		// Loading a save is not the new-file chain, so nothing must send character
		// select back to a step of it.
		s_previousMenu = NULL;

		menu->state |= ONLY_DRAW_TITLE;
		return;

	case -1:
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		return;

	default:
		return;
	}
}

struct RectMenu *MM_AdvNewLoad_GetMenuPtr(void)
{
	// menu for new/load
	return &MM_MENU_ADVENTURE;
}

void MM_ResetAllMenus(void)
{
	u16 menuIndex;
	for (menuIndex = 0; menuIndex < MM_MENU_RESET_COUNT; menuIndex++)
	{
		struct RectMenu *menu = MM_MENU_POINTERS[(s16)menuIndex];

		// NOTE(aalhendi): Retail resets one menu per array slot; native walks
		// chained menus because overlay 230 data is not reloaded.
		do
		{
			struct RectMenu *next = menu->ptrNextBox_InHierarchy;

			menu->state |= RECTMENU_CLOSE_TRANSIENT;
			menu->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
			menu->ptrNextBox_InHierarchy = 0;
			menu->ptrPrevBox_InHierarchy = 0;

			menu = next;
		} while (menu != 0);
	}

	// unused
	sdata_static.framesRemainingInMenu = MM_MENU_RESET_DONE_FRAMES;
}

void MM_JumpTo_Title_Returning(void)
{
	// return to main menu from another menu
	MM_TITLE_MENU_STATE = TITLE_MENU_STATE_RETURNING;

	// return to main menu
	MM_DESIRED_MENU = &MM_MENU_MAIN;

	MM_TITLE_MENU_TRANSITION_FRAME = MM_TITLE_TRANSITION_DURATION;
}

void MM_JumpTo_Title_FirstTime(void)
{
	register struct GameTracker *gGT CTR_PSX_REGISTER("$4");
	register struct RectMenu *mainMenu CTR_PSX_REGISTER("$5");

	MM_ResetAllMenus();

	MainStats_ClearBattleVS();

	// open Main Menu for the first time
	mainMenu = &MM_MENU_MAIN;
	MM_ACTIVE_MENU = mainMenu;

	MM_TITLE_INTRO_FRAME = 0;
	gGT = GAME_TRACKER;

	// first time in main menu
	// (play crash trophy anim)
	MM_TITLE_MENU_STATE = TITLE_MENU_STATE_INTRO;

	// reset countdown clock for battle or crystal challenge
	gGT->originalEventTime = TITLE_INITIAL_EVENT_TIME;

	mainMenu->state &= ~(EXECUTE_FUNCPTR | ONLY_DRAW_TITLE);
	mainMenu->state |= DISABLE_INPUT_ALLOW_FUNCPTRS;

	// distance to screen (perspective)
	gGT->pushBuffer[0].distanceToScreen_PREV = TITLE_DEFAULT_DISTANCE_TO_SCREEN;
	gGT->pushBuffer[0].distanceToScreen_CURR = TITLE_DEFAULT_DISTANCE_TO_SCREEN;
	gGT->gameMode1 &= ~(TIME_TRIAL);
}

void MM_JumpTo_BattleSetup(void)
{
	register struct RectMenu *menu CTR_PSX_REGISTER("$4");

	// Go to battle setup
	menu = &MM_MENU_BATTLE_WEAPONS;
	MM_ACTIVE_MENU = menu;

	menu->state &= ~(ONLY_DRAW_TITLE);

	MM_Battle_Init();
}

void MM_JumpTo_TrackSelect(void)
{
	register struct RectMenu *menu CTR_PSX_REGISTER("$4");

	// return to track selection
	menu = &MM_MENU_TRACK_SELECT;
	MM_ACTIVE_MENU = menu;

	menu->state &= ~(ONLY_DRAW_TITLE);

	MM_TrackSelect_Init();
}

void MM_JumpTo_Characters(void)
{
	register struct RectMenu *menu CTR_PSX_REGISTER("$4");

	// return to character selection
	menu = &MM_MENU_CHARACTER_SELECT;
	MM_ACTIVE_MENU = menu;

	menu->state &= ~(ONLY_DRAW_TITLE);

	MM_Characters_RestoreIDs();
}

void MM_JumpTo_Scrapbook(void)
{
	register struct RectMenu *menu CTR_PSX_REGISTER("$4");

	// go to scrapbook
	menu = &MM_MENU_SCRAPBOOK;
	MM_ACTIVE_MENU = menu;

	menu->state &= ~(ONLY_DRAW_TITLE);

	MM_Scrapbook_Init();
}

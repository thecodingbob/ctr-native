#include <common.h>

extern struct RectMenu menu224NoSave;

static void RefreshCard_InsertSavedGhostProfile(void)
{
	int index = sdata->ghostProfile_indexSave;

	if (index < 0)
		index = 0;

	if (index > 6)
		index = 6;

	sdata->ghostProfile_memcard[index] = sdata->ghostProfile_current;

	if (sdata->ghostProfile_rowSelect < 0)
	{
		if (sdata->numGhostProfilesSaved < 7)
			sdata->numGhostProfilesSaved++;
	}

	if (sdata->numGhostProfilesSaved <= index)
		sdata->numGhostProfilesSaved = index + 1;
}

static void RefreshCard_FinishGhostSave(int result)
{
	if (result == MC_RETURN_IOE)
	{
		RefreshCard_InsertSavedGhostProfile();
		sdata->gGT->gameModeEnd |= PLAYER_GHOST_BEAT;
		sdata->ptrDesiredMenu = &menu224NoSave;

		if (sdata->ptrActiveMenu == &data.menuGhostSelection)
			RECTMENU_Hide(sdata->ptrActiveMenu);
	}
	else
	{
		sdata->boolError = 1;
		sdata->mcScreenText = MC_SCREEN_ERROR_TIMEOUT;
	}

	sdata->mcStart = 2;
	sdata->unk8008d964 = 1;
}

// TODO(aalhendi): Replace this ghost-save-only bridge with the full retail RefreshCard_Entry state machine.
void RefreshCard_Entry(void)
{
	int result;

	if (sdata->mcStart != 6)
		return;

	if (sdata->memcard_stage == MC_STAGE_IDLE)
	{
		if (sdata->GhostRecording.ptrGhost == NULL)
		{
			RefreshCard_FinishGhostSave(MC_RETURN_TIMEOUT);
			return;
		}

		sdata->mcScreenText = MC_SCREEN_SAVING;
		result = MEMCARD_Save(0, sdata->ghostProfile_current.profile_name, (char *)&sdata->memcardIcon_HeaderGHOST[0], (u8 *)sdata->GhostRecording.ptrGhost,
		                      0x3e00, 0);

		if (result != MC_RETURN_PENDING)
		{
			RefreshCard_FinishGhostSave(result);
			return;
		}
	}

	result = MEMCARD_HandleEvent();
	if (result != MC_RETURN_PENDING)
		RefreshCard_FinishGhostSave(result);
}

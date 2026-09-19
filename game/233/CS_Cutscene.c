#include <common.h>

// for oxide intro and ND box
void CS_Cutscene_Start(void)
{
	struct CsThreadInitData initData;
	// NOTE(aalhendi): Retail reserves 32 unused bytes in this call frame.
	u8 unusedWorkspace[0x20];

	struct GameTracker *gGT;
	(void)unusedWorkspace;

	CS_Thread_Init(0, CS_INTRO_NAME(s_introcam), 0, 0, 0);

	gGT = GAME_TRACKER;
	if ((gGT->gameMode2 & CREDITS) != 0)
	{
		CS_FINISHED = 0;

		CS_Credits_Init();

		CS_Instance_InitMatrix();
		return;
	}

	if (gGT->levelID != NAUGHTY_DOG_CRATE)
	{
		return;
	}

	CS_Instance_InitMatrix();

	initData.podiumPos.x = 0;
	initData.podiumPos.y = 0;
	initData.podiumPos.z = 0;
	initData.rot.x = 0;
	initData.rot.y = 0;
	initData.rot.z = 0;
	initData.characterPos.x = 0;
	initData.characterPos.y = 0;
	initData.characterPos.z = 0;

	CS_Thread_Init(NDI_BOX_BOX_01, CS_INTRO_NAME(s_box1), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02, CS_INTRO_NAME(s_box2), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02_BOTTOM, CS_INTRO_NAME(s_box2_bottom), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02_FRONT, CS_INTRO_NAME(s_box2_front), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02A, CS_INTRO_NAME(s_box2_A), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_03, CS_INTRO_NAME(s_box3), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_CODE, CS_INTRO_NAME(s_code), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_GLOW, CS_INTRO_NAME(s_glow), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LID, CS_INTRO_NAME(s_lid), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LIDB, CS_INTRO_NAME(s_lidb), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LIDC, CS_INTRO_NAME(s_lidc), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LIDD, CS_INTRO_NAME(s_lidd), &initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LID2, CS_INTRO_NAME(s_lid2), &initData, 0, 0);
	CS_Thread_Init(NDI_KART0, CS_INTRO_NAME(s_kart0), &initData, 0, 0);
	CS_Thread_Init(NDI_KART1, CS_INTRO_NAME(s_kart1), &initData, 0, 0);
	CS_Thread_Init(NDI_KART2, CS_INTRO_NAME(s_kart2), &initData, 0, 0);
	CS_Thread_Init(NDI_KART3, CS_INTRO_NAME(s_kart3), &initData, 0, 0);
	CS_Thread_Init(NDI_KART6, CS_INTRO_NAME(s_kart6), &initData, 0, 0);
	CS_Thread_Init(NDI_KART7, CS_INTRO_NAME(s_kart7), &initData, 0, 0);
}

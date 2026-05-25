#include <common.h>

// for oxide intro and ND box
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b087c-0x800b0b38
void CS_Cutscene_Start(void)
{
	struct CsThreadInitData initData;

	struct GameTracker *gGT = sdata->gGT;

	CS_Thread_Init(0, OVR_233.s_introcam, 0, 0, 0);

	if ((gGT->gameMode2 & CREDITS) != 0)
	{
		OVR_233.isCutsceneOver = 0;

		CS_Credits_Init();

		CS_Instance_InitMatrix();
		return;
	}

	if (gGT->levelID != NAUGHTY_DOG_CRATE)
		return;

	CS_Instance_InitMatrix();

	initData.podiumPos[0] = 0;
	initData.podiumPos[1] = 0;
	initData.podiumPos[2] = 0;
	initData.characterPos[0] = 0;
	initData.characterPos[1] = 0;
	initData.characterPos[2] = 0;
	initData.rot[0] = 0;
	initData.rot[1] = 0;
	initData.rot[2] = 0;

	CS_Thread_Init(NDI_BOX_BOX_01, OVR_233.s_box1, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02, OVR_233.s_box2, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02_BOTTOM, OVR_233.s_box2_bottom, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02_FRONT, OVR_233.s_box2_front, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_02A, OVR_233.s_box2_A, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_BOX_03, OVR_233.s_box3, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_CODE, OVR_233.s_code, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_GLOW, OVR_233.s_glow, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LID, OVR_233.s_lid, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LIDB, OVR_233.s_lidb, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LIDC, OVR_233.s_lidc, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LIDD, OVR_233.s_lidd, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_BOX_LID2, OVR_233.s_lid2, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_KART0, OVR_233.s_kart0, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_KART1, OVR_233.s_kart1, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_KART2, OVR_233.s_kart2, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_KART3, OVR_233.s_kart3, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_KART6, OVR_233.s_kart6, (s16 *)&initData, 0, 0);
	CS_Thread_Init(NDI_KART7, OVR_233.s_kart7, (s16 *)&initData, 0, 0);
}

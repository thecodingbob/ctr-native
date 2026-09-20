OUTPUT_ARCH(mips)

SECTIONS
{
	/*
	 * The retail linker laid each source file's compiler-generated switch table
	 * over the corresponding words already present in R230.  Keep those input
	 * sections as address-only views so code relocations resolve to the retail
	 * table addresses without emitting duplicate bytes.
	 */
	.title_jump_table __overlay_load_address + 0x4 (NOLOAD) : SUBALIGN(4)
	{
		*MM_Title.o(.rodata)
	}
	.characters_jump_table __overlay_load_address + 0x3c (NOLOAD) : SUBALIGN(4)
	{
		*MM_Characters.o(.rodata)
	}
	.battle_jump_table __overlay_load_address + 0x6c (NOLOAD) : SUBALIGN(4)
	{
		*MM_Battle.o(.rodata)
	}
	.scrapbook_jump_table __overlay_load_address + 0xa4 (NOLOAD) : SUBALIGN(4)
	{
		*MM_Scrapbook.o(.rodata)
	}

	.overlay __overlay_load_address :
	{
		*(.R230)
		*(.MM_Battle_DrawIcon_Character)
		*(.MM_TransitionInOut)
		*(.MM_Title_MenuUpdate)
		*(.MM_Title_SetTrophyDPP)
		*(.MM_Title_CameraMove)
		*(.MM_Title_ThTick)
		*(.MM_Title_Init)
		*(.MM_Title_CameraReset)
		*(.MM_Title_KillThread)
		*(.MM_Cheat_MaxWumpa)
		*(.MM_Cheat_UnlockRoo)
		*(.MM_Cheat_UnlockPapu)
		*(.MM_Cheat_UnlockJoe)
		*(.MM_Cheat_UnlockPinstripe)
		*(.MM_Cheat_UnlockFakeCrash)
		*(.MM_Cheat_UnlockPenta)
		*(.MM_Cheat_UnlockTropy)
		*(.MM_Cheat_UnlockScrapbook)
		*(.MM_Cheat_UnlockTracks)
		*(.MM_Cheat_InfiniteMasks)
		*(.MM_Cheat_MaxTurbos)
		*(.MM_Cheat_MaxInvisibility)
		*(.MM_Cheat_MaxEngine)
		*(.MM_Cheat_MaxBombs)
		*(.MM_Cheat_AdvDifficulty)
		*(.MM_Cheat_SuperHard)
		*(.MM_Cheat_IcyTracks)
		*(.MM_Cheat_SuperTurboPads)
		*(.MM_Cheat_OneLap)
		*(.MM_Cheat_TurboCounter)
		*(.MM_ParseCheatCodes)
		*(.MM_MenuProc_Main)
		*(.MM_ToggleRows_PlayerCount)
		*(.MM_MenuProc_1p2p)
		*(.MM_MenuProc_2p3p4p)
		*(.MM_ToggleRows_Difficulty)
		*(.MM_MenuProc_Difficulty)
		*(.MM_MenuProc_SingleCup)
		*(.MM_MenuProc_NewLoad)
		*(.MM_AdvNewLoad_GetMenuPtr)
		*(.MM_Characters_AnimateColors)
		*(.MM_Characters_GetNextDriver)
		*(.MM_Characters_boolIsInvalid)
		*(.MM_Characters_GetModelByName)
		*(.MM_Characters_DrawWindows)
		*(.MM_Characters_SetMenuLayout)
		*(.MM_Characters_BackupIDs)
		*(.MM_Characters_PreventOverlap)
		*(.MM_Characters_RestoreIDs)
		*(.MM_Characters_HideDrivers)
		*(.MM_Characters_MenuProc)
		*(.MM_TrackSelect_Video_SetDefaults)
		*(.MM_TrackSelect_Video_State)
		*(.MM_TrackSelect_Video_Draw)
		*(.MM_TrackSelect_boolTrackOpen)
		*(.MM_TrackSelect_Init)
		*(.MM_TrackSelect_MenuProc)
		*(.MM_TrackSelect_GetMenuPtr)
		*(.MM_CupSelect_Init)
		*(.MM_CupSelect_MenuProc)
		*(.MM_Battle_CloseSubMenu)
		*(.MM_Battle_DrawIcon_Weapon)
		*(.MM_Battle_Init)
		*(.MM_Battle_MenuProc)
		*(.MM_HighScore_Text3D)
		*(.MM_HighScore_Draw)
		*(.MM_HighScore_Init)
		*(.MM_HighScore_MenuProc)
		*(.MM_Scrapbook_Init)
		*(.MM_Scrapbook_PlayMovie)
		*(.MM_ResetAllMenus)
		*(.MM_JumpTo_Title_Returning)
		*(.MM_JumpTo_Title_FirstTime)
		*(.MM_JumpTo_BattleSetup)
		*(.MM_JumpTo_TrackSelect)
		*(.MM_JumpTo_Characters)
		*(.MM_JumpTo_Scrapbook)
		*(.D230)
		*(.MM_Video_DecDCToutCallbackFunc)
		*(.MM_Video_KickCD)
		*(.MM_Video_VLC_Decode)
		*(.MM_Video_StartStream)
		*(.MM_Video_StopStream)
		*(.MM_Video_AllocMem)
		*(.MM_Video_ClearMem)
		*(.MM_Video_DecodeFrame)
		*(.MM_Video_CheckIfFinished)
		*(.text)
		*(.data)
		*(.sdata)
		. = . + 0x88;
	}

	/DISCARD/ :
	{
		*(.bss)
		*(.sbss)
		*(.reginfo)
		*(.MIPS.abiflags)
		*(.pdr)
		*(.gnu.attributes)
		*(.comment)
	}
}

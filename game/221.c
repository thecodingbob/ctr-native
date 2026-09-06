#include <common.h>

#if defined(CTR_NATIVE)
#include <platform/native_log.h>
#endif

enum CrystalChallengeEndMenuConstants
{
	CC_FLY_IN_FRAMES = 0x14,
	CC_SCREEN_DEPTH = 0x200,
	CC_TOKEN_GROW_LIMIT = 0x2001,
	CC_TOKEN_GROW_STEP = 0x200,
	CC_CONFIRM_BUTTON_MASK = BTN_CROSS_one | BTN_CIRCLE,
	// NOTE(aalhendi): Native sentinel for retail stack residue that has not
	// been modeled yet.
	CC_UNMODELED_REWARD_OFFSET = -0x400000,
};

#define CC_BATTLE_TRACK_PURPLE_TOKEN_OFFSETS 3, 1, -1, 0, -1, 2, -1

#if defined(CTR_NATIVE)
global_variable const s16 s_battleTrackPurpleTokenOffset[LAB_BASEMENT - NITRO_COURT + 1] = {
    CC_BATTLE_TRACK_PURPLE_TOKEN_OFFSETS,
};
#endif

extern struct RectMenu menu221;

// NOTE(aalhendi): The overlay-private matching ABI overrides these defaults
// before compiling the retail artifact.
#ifndef CC_GAME_TRACKER
#define CC_GAME_TRACKER            (sdata->gGT)
#define CC_FRAMES_SINCE_RACE_ENDED (sdata->framesSinceRaceEnded)
#define CC_HUD_CRYSTAL             (sdata->ptrHudCrystal)
#define CC_MENU_CRYSTAL            (sdata->ptrMenuCrystal)
#define CC_TOKEN                   (sdata->ptrToken)
#define CC_LANGUAGE_STRINGS        (sdata->lngStrings)
#define CC_MENU_READY              (sdata->menuReadyToPass)
#define CC_ANY_PLAYER_TAP          (sdata->AnyPlayerTap)
#define CC_ADV_PROGRESS            (sdata->advProgress)
#define CC_ADD_CONFIG_0            (sdata->Loading.OnBegin.AddBitsConfig0)
#define CC_REMOVE_CONFIG_0         (sdata->Loading.OnBegin.RemBitsConfig0)
#endif

#ifndef CC_READ_GAME_TRACKER
#define CC_READ_GAME_TRACKER() CC_GAME_TRACKER
#endif

#if defined(CTR_NATIVE)
static void CC_EndEvent_LogUnmodeledRewardOffset(const s32 levelID)
{
	static u64 s_loggedLevelMask[2];
	static b32 s_loggedOutOfRangeLevel;

	if (levelID >= 0 && levelID < 128)
	{
		const u64 levelBit = (u64)1 << (levelID & 0x3f);
		u64 *levelMask = &s_loggedLevelMask[levelID >> 6];

		if ((*levelMask & levelBit) != 0)
		{
			return;
		}

		*levelMask |= levelBit;
	}
	else
	{
		if (s_loggedOutOfRangeLevel)
		{
			return;
		}

		s_loggedOutOfRangeLevel = true;
	}

	Platform_LogWarn("[CTR 221] unmodeled CC reward residue: levelID=%d\n", levelID);
}
#endif

#if defined(CTR_NATIVE)
static s32 CC_EndEvent_GetRewardOffset(struct GameTracker *gGT)
{
	s32 levelID = gGT->levelID;
	s32 frustumSavedCameraZ = PushBuffer_GetFrustumSavedCameraZ();

	if (levelID == DINGO_CANYON)
	{
		// NOTE(aalhendi): Retail does not read pushBuffer[0].pos.z directly
		// here. This underreads the battle-token stack table and aliases the
		// earlier PushBuffer_SetFrustumPlane stack save of
		// PushBuffer_UpdateFrustum's s5 camera-Z value.
		return (s16)frustumSavedCameraZ;
	}

	if (levelID == DRAGON_MINES)
	{
		// Same saved s5 word as Dingo, read from its high halfword.
		return (s16)CTR_MipsSra(frustumSavedCameraZ, 16);
	}

	if (levelID == BLIZZARD_BLUFF)
	{
		// Low halfword of saved s6 from the same producer; retail s6 is the
		// scratchpad base 0x1f800000, so this reads as zero.
		return 0;
	}

	if (levelID < NITRO_COURT || levelID > LAB_BASEMENT)
	{
		// NOTE(aalhendi): Retail would keep reading the adjacent stack halfwords.
		// Native only models audited residue producers. We just log unmodeled UB
		CC_EndEvent_LogUnmodeledRewardOffset(levelID);
		return CC_UNMODELED_REWARD_OFFSET;
	}

	return s_battleTrackPurpleTokenOffset[levelID - NITRO_COURT];
}

static u32 CC_EndEvent_GetRewardBitMask(s32 rewardBit)
{
	return 1u << ((u32)rewardBit & 0x1f);
}

CTR_STATIC_ASSERT(OFFSETOF(struct sData, gameOptions) + sizeof(struct GameOptions) == OFFSETOF(struct sData, advProgress));

static u8 *CC_EndEvent_GetNativeRewardWordBytes(s32 rewardBit)
{
	s32 wordIndex = CTR_MipsSra(rewardBit, 5);
	s64 rewardByteOffset = (s64)OFFSETOF(struct sData, advProgress.rewards) + (s64)wordIndex * (s64)sizeof(u32);
	s64 windowStart = (s64)OFFSETOF(struct sData, gameOptions);
	s64 windowEnd = (s64)OFFSETOF(struct sData, advProgress) + (s64)sizeof(struct AdvProgress);

	// NOTE(aalhendi): Retail applies the unchecked residue index to
	// advProgress.rewards, so Dingo Bingo can touch adjacent gameOptions words.
	// Native bounds that retail window without doing host out-of-bounds access.
	if ((rewardByteOffset < windowStart) || (rewardByteOffset > windowEnd - (s32)sizeof(u32)))
	{
		return NULL;
	}

	return (u8 *)sdata + (s32)rewardByteOffset;
}

static b32 CC_EndEvent_HasRewardBit(struct AdvProgress *adv, s32 rewardBit)
{
	(void)adv;
	u8 *wordBytes = CC_EndEvent_GetNativeRewardWordBytes(rewardBit);
	if (wordBytes == NULL)
	{
		return true;
	}

	u32 rewardWord;
	memcpy(&rewardWord, wordBytes, sizeof(rewardWord));
	return (rewardWord & CC_EndEvent_GetRewardBitMask(rewardBit)) != 0;
}

static void CC_EndEvent_UnlockRewardBit(struct AdvProgress *adv, s32 rewardBit)
{
	(void)adv;
	u8 *wordBytes = CC_EndEvent_GetNativeRewardWordBytes(rewardBit);
	if (wordBytes == NULL)
	{
		return;
	}

	u32 rewardWord;
	memcpy(&rewardWord, wordBytes, sizeof(rewardWord));
	rewardWord |= CC_EndEvent_GetRewardBitMask(rewardBit);
	memcpy(wordBytes, &rewardWord, sizeof(rewardWord));
}
#endif

void CC_EndEvent_DrawMenu()
{
	struct GameTracker *comparisonGT;
	struct Driver *driver;
	s16 pos[2];
	s32 resultStringIndex;
	b32 didWin;
#if defined(CTR_NATIVE)
	register b32 canAward;
	register s32 currentFrames;
#else
	// NOTE(aalhendi): Retail-path register bindings preserve two GCC allocation
	// and scheduling decisions without embedding instructions.
	register b32 canAward asm("$2");
	u32 *rewardWords;
	register s32 currentFrames asm("$3");
#endif
	s32 tokenRewardBit;

	driver = CC_READ_GAME_TRACKER()->drivers[0];
	comparisonGT = CC_READ_GAME_TRACKER();

#if !defined(CTR_NATIVE)
	// NOTE(aalhendi): This block lets GCC 2.8.1 declare the retail stack table
	// after the entry loads while keeping it live through the reward paths.
	{
		s16 battleTrackPurpleTokenOffset[LAB_BASEMENT - NITRO_COURT + 1] = {CC_BATTLE_TRACK_PURPLE_TOKEN_OFFSETS};
#endif

		if (driver->numCrystals >= comparisonGT->numCrystalsInLEV)
		{
			didWin = true;
			resultStringIndex = LNG_YOU_WIN;
		}
		else
		{
			didWin = false;
			resultStringIndex = LNG_TRY_AGAIN;
		}

		if (CC_FRAMES_SINCE_RACE_ENDED < CTR_SECONDS_TO_FRAMES(30))
		{
			CC_FRAMES_SINCE_RACE_ENDED++;
		}

#if defined(CTR_NATIVE)
		// NOTE(aalhendi): Menu-storage can enter this flow in tracks without
		// crystal HUD instances; keep reward/menu logic and skip missing models.
		if (CC_HUD_CRYSTAL != NULL)
#endif
		{
			CC_HUD_CRYSTAL->flags = CC_HUD_CRYSTAL->flags | HIDE_MODEL;
		}

		currentFrames = CC_FRAMES_SINCE_RACE_ENDED;

		// fly in from left
		UI_Lerp2D_Linear(&pos[0], -0x64, 0x18, 0x100, 0x18, currentFrames, CC_FLY_IN_FRAMES);
		DecalFont_DrawLine(CC_LANGUAGE_STRINGS[LNG_TIME_REMAINING], pos[0], pos[1], FONT_BIG, (JUSTIFY_CENTER | ORANGE));
		UI_DrawLimitClock(pos[0] - 0x33, pos[1] + 0x11, FONT_BIG);

		// fly in from right
		UI_Lerp2D_Linear(&pos[0], 0x264, 0x56, 0xcd, 0x56, CC_FRAMES_SINCE_RACE_ENDED, CC_FLY_IN_FRAMES);

		// Crystal count
#if defined(CTR_NATIVE)
		if (CC_MENU_CRYSTAL != NULL)
#endif
		{
			CC_MENU_CRYSTAL->matrix.t[0] = UI_ConvertX_2(pos[0], CC_SCREEN_DEPTH);
			CC_MENU_CRYSTAL->matrix.t[1] = UI_ConvertY_2(pos[1], CC_SCREEN_DEPTH);
		}
		UI_DrawNumCrystal(pos[0] + 0xf, pos[1] - 0x10, driver);

		// YOU WIN, or TRY AGAIN
		DecalFont_DrawLine(CC_LANGUAGE_STRINGS[resultStringIndex], pos[0] + 0x33, pos[1] + 8, FONT_BIG, (JUSTIFY_CENTER | ORANGE));

		canAward = didWin;
		if (!canAward)
		{
			goto DrawAlreadyAwardedMenu;
		}

#if defined(CTR_NATIVE)
		tokenRewardBit = CC_EndEvent_GetRewardOffset(CC_GAME_TRACKER) + ADV_REWARD_FIRST_PURPLE_TOKEN;
		if (CC_EndEvent_HasRewardBit(&CC_ADV_PROGRESS, tokenRewardBit))
#else
	// NOTE(aalhendi): Retail intentionally indexes this stack table without
	// bounds checks.
	rewardWords = CC_ADV_PROGRESS.rewards;
	tokenRewardBit = battleTrackPurpleTokenOffset[CC_GAME_TRACKER->levelID - NITRO_COURT] + ADV_REWARD_FIRST_PURPLE_TOKEN;
	if (CHECK_ADV_BIT(rewardWords, tokenRewardBit))
#endif
		{
			goto DrawAlreadyAwardedMenu;
		}

		{
			UI_Lerp2D_Linear(&pos[0], -0x64, 0xA2, 0x100, 0xA2, CC_FRAMES_SINCE_RACE_ENDED, CC_FLY_IN_FRAMES);

			DecalFont_DrawLine(CC_LANGUAGE_STRINGS[LNG_CTR_TOKEN_AWARDED], pos[0], pos[1], FONT_BIG,
			                   (CC_GAME_TRACKER->timer != 0) ? (s16)(JUSTIFY_CENTER | ORANGE) : (s16)(JUSTIFY_CENTER | WHITE));
#if defined(CTR_NATIVE)
			if (CC_TOKEN != NULL)
#endif
			{
				CC_TOKEN->flags &= ~(HIDE_MODEL);
				CC_TOKEN->matrix.t[0] = UI_ConvertX_2(pos[0], CC_SCREEN_DEPTH);
				CC_TOKEN->matrix.t[1] = UI_ConvertY_2(pos[1] - 0x18, CC_SCREEN_DEPTH);
			}

			if (CC_FRAMES_SINCE_RACE_ENDED == CTR_SECONDS_TO_FRAMES(1))
			{
				OtherFX_Play(0x67, 1);
			}

			if (CC_FRAMES_SINCE_RACE_ENDED > CTR_SECONDS_TO_FRAMES(1))
			{
				struct Instance *token = CC_TOKEN;

#if defined(CTR_NATIVE)
				if (token != NULL)
#endif
				{
					if (token->scale.x < CC_TOKEN_GROW_LIMIT)
					{
						token->scale.x += CC_TOKEN_GROW_STEP;
						token->scale.y += CC_TOKEN_GROW_STEP;
						token->scale.z += CC_TOKEN_GROW_STEP;
					}
				}
			}

			DecalFont_DrawLine(CC_LANGUAGE_STRINGS[LNG_PRESS_TO_CONTINUE], 0x100, 0xbe, FONT_BIG, (JUSTIFY_CENTER | ORANGE));

			// if still waiting to press X/O, quit function
			if ((CC_ANY_PLAYER_TAP & CC_CONFIRM_BUTTON_MASK) == 0)
			{
				return;
			}

			// if pressed X/O,
			// unlock token and leave level

			RECTMENU_ClearInput();
			CC_FRAMES_SINCE_RACE_ENDED = 0;

			{
				struct GameTracker *returnGT;
				u32 *addConfig;

				CC_REMOVE_CONFIG_0 |= CRYSTAL_CHALLENGE;
				returnGT = CC_GAME_TRACKER;
				addConfig = &CC_ADD_CONFIG_0;
#if defined(CTR_NATIVE)
				CC_EndEvent_UnlockRewardBit(&CC_ADV_PROGRESS, tokenRewardBit);
#else
			UNLOCK_ADV_BIT(CC_ADV_PROGRESS.rewards, battleTrackPurpleTokenOffset[returnGT->levelID - NITRO_COURT] + ADV_REWARD_FIRST_PURPLE_TOKEN);
#endif
				*addConfig |= ADVENTURE_ARENA;
				MainRaceTrack_RequestLoad(returnGT->prevLEV); // Adventure hub.
			}

			return;
		}

	DrawAlreadyAwardedMenu:
	{
		if ((CC_MENU_READY & 1) != 0)
		{
			return;
		}

		DecalFont_DrawLine(CC_LANGUAGE_STRINGS[LNG_PRESS_TO_CONTINUE], 0x100, 0xbe, FONT_BIG, (JUSTIFY_CENTER | ORANGE));

		if ((CC_ANY_PLAYER_TAP & CC_CONFIRM_BUTTON_MASK) == 0)
		{
			return;
		}

		RECTMENU_ClearInput();
		RECTMENU_Show(&menu221); // Retry / Exit To Map menu
		CC_MENU_READY |= 1;
	}
#if !defined(CTR_NATIVE)
	}
#endif
}

struct MenuRow rows221[3] = {
    // Retry
    {
        .stringIndex = LNG_RETRY,
        .rowOnPressUp = 0,
        .rowOnPressDown = 1,
        .rowOnPressLeft = 0,
        .rowOnPressRight = 0,
    },

    // Exit to map
    {
        .stringIndex = LNG_EXIT_TO_MAP,
        .rowOnPressUp = 0,
        .rowOnPressDown = 1,
        .rowOnPressLeft = 1,
        .rowOnPressRight = 1,
    },

    // NULL, end of menu
    {
        .stringIndex = RECTMENU_STRING_NONE,
        .rowOnPressUp = 0,
        .rowOnPressDown = 0,
        .rowOnPressLeft = 0,
        .rowOnPressRight = 0,
    }};

struct RectMenu menu221 = {
    .stringIndexTitle = RECTMENU_STRING_NONE,
    .posX_curr = 0x100,
    .posY_curr = 0xB4,

    .unk1 = 0,

    .state = RECTMENU_STATE_CENTERED,
    .rows = rows221,
    .funcPtr = UI_RaceEnd_MenuProc,
    .drawStyle = 4,

    // rest of variables all default zero
};

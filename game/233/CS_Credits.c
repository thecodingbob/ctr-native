#include <common.h>

enum CreditsRuntimeConstants
{
	CS_CREDITS_RELIC_TRACK_COUNT = ADV_REWARD_RELIC_TRACK_COUNT,
	CS_CREDITS_THREAD_FLAGS = SMALL | OTHER,
	CS_CREDITS_CONFETTI_PARTICLE_COUNT = 250,
	CS_CREDITS_COUNTDOWN_FRAMES = 360,
	CS_CREDITS_TEXT_START_STRING_INDEX = 0x14,
	CS_CREDITS_LINE_HEIGHT = 20,
	CS_CREDITS_NAME_START_Y = 340,
	CS_CREDITS_NAME_END_Y = 0x114,
	CS_CREDITS_NAME_FADE_END_START_Y = 0x83,
	CS_CREDITS_NAME_FADE_BOTTOM_Y = 0x96,
	CS_CREDITS_NAME_DIRECTIVE_STRIDE = 3,
	CS_CREDITS_NAME_DIRECTIVE_DIGIT_BASE = '0',
	CS_CREDITS_NAME_DIRECTIVE_COLOR_LIMIT = 50,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_52 = 52,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_53 = 53,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_52_FLAG = 0x2000,
	CS_CREDITS_NAME_DIRECTIVE_STYLE_53_FLAG = 0x1000,
	CS_CREDITS_EPILOGUE_DURATION_FRAMES = 200,
	CS_CREDITS_EPILOGUE_FADE_OUT_START_FRAMES = 0xb5,
	CS_CREDITS_EPILOGUE_CENTER_X = 0x100,
	CS_CREDITS_EPILOGUE_Y = 0xaf,
	CS_CREDITS_EPILOGUE_WIDTH = 0x1cc,
	CS_CREDITS_EPILOGUE_UNUSED_POS_X = 0x200,
	CS_CREDITS_GHOST_BASE_SCALE = FP_ONE,
	CS_CREDITS_GHOST_SCALE_STEP = 300,
	CS_CREDITS_GHOST_ALPHA_STEP = 630,
	CS_CREDITS_GHOST_TRAIL_SCALE_STEP = 0x4b,
	CS_CREDITS_GHOST_TRAIL_ALPHA_STEP = 0x9d,
};


char *CS_Credits_GetNextString(char *str)
{
#if defined(CTR_NATIVE)
	if (str == NULL)
	{
		// NOTE(aalhendi): Retail blindly reads the input pointer. Native
		// returns the same "no next string" result when credits epilogue text
		// reaches the end and the next pointer is PS1 null-space.
		return NULL;
	}
#endif

	while (*str != '\0')
	{
		if (*str == '\r')
		{
			break;
		}
		str++;
	}
	if (*str == '\r')
	{
		return str + 1;
	}
	return NULL;
}

void CS_Credits_DestroyCreditGhost(void)
{
	s16 i;
	struct CreditsObj *co = &creditsBSS.creditsObj;

	for (i = 0; i < CS_CREDITS_GHOST_COUNT; i++)
	{
		INSTANCE_Death(co->creditGhostInst[i]);
	}

	MEMPACK_ClearHighMem();
}

void CS_Credits_AnimateCreditGhost(struct Instance *dst, struct Instance *src, s16 index)
{
	s16 scale;
	s16 i;
	s16 count;

	dst->animFrame = src->animFrame;
	dst->animIndex = src->animIndex;
	dst->matrix = src->matrix;
	scale = CS_CREDITS_GHOST_BASE_SCALE + (index + 1) * CS_CREDITS_GHOST_SCALE_STEP;
	dst->scale.z = scale;
	dst->scale.y = scale;
	dst->scale.x = scale;
	dst->flags &= ~HIDE_MODEL;
	if (dst->model == NULL)
		dst->flags |= HIDE_MODEL;
	dst->alphaScale = (index + 1) * CS_CREDITS_GHOST_ALPHA_STEP;
	dst->model = &creditsBSS.creditsObj.creditGhostModelCopies[index];
	*dst->model = *src->model;
	dst->model->headers = creditsBSS.creditsObj.creditGhostHeaders[index];
	count = src->model->numHeaders;
	if (count > 0)
	{
		i = 0;
		do
		{
			dst->model->headers[i] = src->model->headers[i];
			i++;
		} while (i < count);
	}
}

const struct CsCreditsNames csCreditsNames CTR_PSX_MATCH_SECTION(".CS_credits_names") = {"credits", "creditghost", "credit strings"};

void CS_Credits_ThDestroy(struct Thread *self)
{
	(void)self;
}

void CS_Credits_Init(void)
{
	b16 boolAllGold = 1;
	s16 i = 0;
	struct CreditsObj *creditsObj;
	struct Thread *creditThread;
	struct CreditsLevHeader *creditsDst;
	u8 *creditsData;
	struct GameTracker *gGT;
	u32 *rewards = sdata->advProgress.rewards;
	CS_CREDITS_DANCER_THREAD = NULL;
	CS_CREDITS_ALL_BLUE = 1;
	for (; i < CS_CREDITS_RELIC_TRACK_COUNT; i++)
	{
		boolAllGold = boolAllGold && CHECK_ADV_BIT(rewards, i + ADV_REWARD_FIRST_GOLD_RELIC);
		CS_CREDITS_ALL_BLUE = CS_CREDITS_ALL_BLUE && CHECK_ADV_BIT(rewards, i + ADV_REWARD_FIRST_SAPPHIRE_RELIC);
	}
	if (boolAllGold)
	{
		gGT = GAME_TRACKER;
		gGT->numWinners = 1;
		gGT->winnerIndex[0] = 0;
		gGT->confetti.numParticles_max = CS_CREDITS_CONFETTI_PARTICLE_COUNT;
		gGT->confetti.vanishRate = CS_CREDITS_CONFETTI_PARTICLE_COUNT;
		gGT->renderFlags |= RENDER_FLAG_CONFETTI;
	}
	creditThread = PROC_BirthWithObject(CS_CREDITS_THREAD_FLAGS, CS_Credits_ThTick, csCreditsNames.credits, NULL);
	creditsObj = &creditsBSS.creditsObj;
	creditThread->funcThDestroy = CS_Credits_ThDestroy;
	CS_CREDITS_THREAD = creditThread;
	memset(creditsObj, 0, sizeof(*creditsObj));
	creditsObj->countdown = CS_CREDITS_COUNTDOWN_FRAMES;
	// Credit ghosts use a placeholder model until the dancer supplies its mesh.
	for (i = 0; i < CS_CREDITS_GHOST_COUNT; i++)
	{
		struct Instance *inst;
		s16 j;
		creditsObj->creditGhostInst[CS_CREDITS_GHOST_COUNT - 1 - i] = inst =
		    INSTANCE_Birth3D(GAME_TRACKER->modelPtr[STATIC_AKUAKU], csCreditsNames.creditGhost, creditThread);
		CTR_WriteU32AlignedLE(&inst->matrix.m[0][0], FP_ONE);
		CTR_WriteU32AlignedLE(&inst->matrix.m[0][2], 0);
		CTR_WriteU32AlignedLE(&inst->matrix.m[1][1], FP_ONE);
		CTR_WriteU32AlignedLE(&inst->matrix.m[2][0], 0);
		inst->matrix.m[2][2] = FP_ONE;
		inst->flags |= SCREENSPACE_INSTANCE;
		inst->idpp[0].pushBuffer = &GAME_TRACKER->pushBuffer_UI;
		for (j = 1; j < GAME_TRACKER->numPlyrCurrGame; j++)
			inst->idpp[j].pushBuffer = NULL;
	}
	// NOTE(aalhendi): Walk the serialized size/count words before relocating
	// the string offsets. Byte access keeps the shared path alias-safe.
	creditsData = (u8 *)(ST1_GETPOINTERS(GAME_TRACKER->level1->ptrSpawnType1))[ST1_CREDITS];
	CS_CREDITS_DANCER = NULL;
	creditsDst = MEMPACK_AllocHighMem(CTR_ReadU32AlignedLE(creditsData), csCreditsNames.creditStrings);
	memcpy(creditsDst, creditsData, ((struct CreditsLevHeader *)creditsData)->size);
	creditsData = (u8 *)creditsDst;
	creditsData += sizeof(u32);
	CS_CREDITS_STRING_COUNT = CTR_ReadU16AlignedLE(creditsData);
	creditsData += sizeof(u32);
	CS_CREDITS_STRINGS = (char **)creditsData;
	for (i = 0; i < CS_CREDITS_STRING_COUNT; i++)
	{
		CTR_WriteU32AlignedLE(creditsData, (u32)creditsDst + CTR_ReadU32AlignedLE(creditsData));
		creditsData += sizeof(u32);
	}
	creditsObj->creditsPosY = CS_CREDITS_NAME_START_Y;
	creditsObj->creditsTopString = CS_CREDITS_STRINGS[CS_CREDITS_TEXT_START_STRING_INDEX];
}

b32 CS_Credits_IsTextValid(void)
{
	struct CreditsObj *creditsObj = &creditsBSS.creditsObj;

	if (creditsObj->epilogueTopString == 0)
	{
		creditsObj->countdown = CS_CREDITS_COUNTDOWN_FRAMES;
		return 1;
	}

	return 0;
}

void CS_Credits_NewDancer(struct Thread *dancerTh, s16 dancerModelID)
{
	struct CreditsObj *creditsObj = &creditsBSS.creditsObj;

	// kill any living thread
	struct Thread *oldDancerThread = CS_CREDITS_DANCER_THREAD;
	if (oldDancerThread != 0)
	{
		CS_CREDITS_DANCER_THREAD = 0;
		oldDancerThread->flags |= THREAD_FLAG_DEAD;
	}

	// store globally, make instance invisible
	CS_CREDITS_DANCER_THREAD = dancerTh;
	CS_CREDITS_DANCER = dancerTh->inst;
	CS_CREDITS_DANCER->flags |= HIDE_MODEL;

	creditsObj->countdown = CS_CREDITS_COUNTDOWN_FRAMES;

	if (dancerModelID >= STATIC_TAWNA1)
	{
		creditsObj->epilogueTopString = CS_CREDITS_STRINGS[dancerModelID - STATIC_CRASHDANCE - 1];
	}
	else
	{
		creditsObj->epilogueTopString = CS_CREDITS_STRINGS[dancerModelID - STATIC_CRASHDANCE];
	}

	creditsObj->epilogueFramesLeft = CS_CREDITS_EPILOGUE_DURATION_FRAMES;

	creditsObj->epilogueNextString = CS_Credits_GetNextString(creditsObj->epilogueTopString);

	creditsObj->epiloguePosX_unused = CS_CREDITS_EPILOGUE_UNUSED_POS_X;
}

int CS_Credits_NewCreditGhosts(void)
{
	s16 i;
	struct CreditsObj *co = &creditsBSS.creditsObj;
	struct Model *model = CS_CREDITS_DANCER->model;
	for (i = 0; i < CS_CREDITS_GHOST_COUNT; i++)
	{
		if (co->creditGhostModel[i] != model)
			return 0;
	}
	return 1;
}

#if defined(CTR_NATIVE)
static void CS_Credits_RestorePodiumAudioForNativeHandoff(void)
{
	if ((CS_CONTROLS_AUDIO == 0) || (howl_VolumeGet(HOWL_VOLUME_TYPE_FX) != 0) || (CS_FX_VOLUME_BACKUP == 0))
	{
		return;
	}

	// NOTE(aalhendi): Boss scripts can mute HOWL with opcode 0x30; normal
	// podium exits restore it in CS_DestroyPodium_StartDriving. Credits do not
	// return to driving, and their scripts do not apply this mute. Native can
	// still reach this handoff with a leaked FX mute, so restore the podium
	// backup before loading the hub/Scrapbook. A zero backup preserves
	// user-muted SFX.
	howl_VolumeSet(HOWL_VOLUME_TYPE_FX, (u8)CS_FX_VOLUME_BACKUP);
	howl_VolumeSet(HOWL_VOLUME_TYPE_MUSIC, (u8)CS_MUSIC_VOLUME_BACKUP);
	howl_VolumeSet(HOWL_VOLUME_TYPE_VOICE, (u8)CS_VOICE_VOLUME_BACKUP);
}
#endif

void CS_Credits_End(void)
{
	int levID;
	CS_Credits_DestroyCreditGhost();
	CS_CREDITS_THREAD->flags |= THREAD_FLAG_DEAD;
#ifdef CTR_NATIVE
	CS_Credits_RestorePodiumAudioForNativeHandoff();
#endif
	if (CS_CREDITS_ALL_BLUE != 0)
	{
		sdata->mainMenuState = MAIN_MENU_SCRAPBOOK;
		levID = SCRAPBOOK;
		MainRaceTrack_RequestLoad(levID);
	}
	else
	{
		levID = GEM_STONE_VALLEY;
		GAME_TRACKER->gameMode1 |= ADVENTURE_MODE;
		MainRaceTrack_RequestLoad(levID);
	}
	GAME_TRACKER->renderFlags &= ~RENDER_FLAG_CONFETTI;
}

// Both credits streams fade over one line at either end of their window.
static inline s32 CS_Credits_FadeAmount(s32 position, s32 width, u16 end)
{
	if (position >= end - width + 1)
		return end - position;
	if (position < width)
		return position;
	return width;
}

void CS_Credits_DrawNames(struct CreditsObj *co)
{
	s16 posY;
	char *str;
	s16 charId = 0;
	if (co->creditsTopString == NULL)
		return;
	co->creditsPosY--;
	if (co->creditsPosY < -CS_CREDITS_LINE_HEIGHT)
	{
		co->creditsTopString = CS_Credits_GetNextString(co->creditsTopString);
		co->creditsPosY += CS_CREDITS_LINE_HEIGHT;
	}
	posY = co->creditsPosY;
	str = co->creditsTopString;
#ifdef CTR_NATIVE
	// NOTE(aalhendi): The last line can scroll away above. Native cannot
	// follow retail's subsequent read from address zero.
	if (str == NULL)
		return;
#endif
	while (posY < CS_CREDITS_NAME_END_Y)
	{
		char *nextStr;
		s32 strLen;
		s32 fadeAmount;
		s16 colorSlot;
		u8 marker = (u8)*str;
		u16 textFlags = 0;
		if (marker == '~')
		{
			// A line may begin with several ~NN color/style directives.
			s32 delimiter = marker;
			char *p = str + 2;
			do
			{
				s16 value;
				s32 digit1 = (u8)p[-1] - CS_CREDITS_NAME_DIRECTIVE_DIGIT_BASE;
				s32 digit2 = (u8)p[0];
				p += CS_CREDITS_NAME_DIRECTIVE_STRIDE;
				value = digit2 + digit1 * 10 - CS_CREDITS_NAME_DIRECTIVE_DIGIT_BASE;
				str += CS_CREDITS_NAME_DIRECTIVE_STRIDE;
				if (value < CS_CREDITS_NAME_DIRECTIVE_COLOR_LIMIT)
					charId = value;
				else if (value >= CS_CREDITS_NAME_DIRECTIVE_STYLE_52)
					switch (value)
					{
					case CS_CREDITS_NAME_DIRECTIVE_STYLE_52:
						textFlags |= CS_CREDITS_NAME_DIRECTIVE_STYLE_52_FLAG;
						break;
					case CS_CREDITS_NAME_DIRECTIVE_STYLE_53:
						textFlags |= CS_CREDITS_NAME_DIRECTIVE_STYLE_53_FLAG;
						break;
					}
			} while ((u8)*str == delimiter);
		}
		nextStr = CS_Credits_GetNextString(str);
		if (nextStr != NULL)
			strLen = nextStr - str - 1;
		else
			strLen = strlen(str);
		fadeAmount = CS_Credits_FadeAmount(posY, CS_CREDITS_LINE_HEIGHT, CS_CREDITS_NAME_FADE_BOTTOM_Y);
		if (fadeAmount >= CS_CREDITS_LINE_HEIGHT || !CS_CREDITS_ALL_BLUE)
			colorSlot = charId;
		else
		{
			if (fadeAmount <= 0)
				colorSlot = -1;
			else
			{
				s16 i;
				fadeAmount = (fadeAmount << 8) / CS_CREDITS_LINE_HEIGHT;
				colorSlot = CREDITS_FADE;
				for (i = 0; i < 4; i++)
				{
					u8 *dst = (u8 *)&CS_CREDITS_FADE_PALETTE[i];
					u8 *src;
					src = (u8 *)CS_COLOR_POINTERS[charId];
					src += i * 4;
					dst[0] = src[0] * fadeAmount >> 8;
					src = (u8 *)CS_COLOR_POINTERS[charId];
					src += i * 4;
					dst[1] = src[1] * fadeAmount >> 8;
					src = (u8 *)CS_COLOR_POINTERS[charId];
					src += i * 4;
					dst[2] = src[2] * fadeAmount >> 8;
				}
			}
		}

		if (colorSlot >= 0)
			DecalFont_DrawLineStrlen(str, strLen, (s16)CS_CREDITS_TEXT_X, posY, FONT_CREDITS, (s16)(colorSlot | textFlags));
		if (nextStr == NULL)
			return;
		posY += CS_CREDITS_LINE_HEIGHT;
		str = nextStr;
	}
}

void CS_Credits_DrawEpilogue(struct CreditsObj *co)
{
	s32 fadeAmount;
	s16 colorSlot;
	if (co->epilogueTopString == NULL)
		return;
	co->epilogueFramesLeft--;
	if (co->epilogueFramesLeft <= 0)
	{
		co->epilogueFramesLeft = CS_CREDITS_EPILOGUE_DURATION_FRAMES;
		co->epilogueTopString = co->epilogueNextString;
		co->epilogueNextString = CS_Credits_GetNextString(co->epilogueTopString);
	}
	if (co->epilogueTopString == NULL)
		return;
	fadeAmount = CS_Credits_FadeAmount(co->epilogueFramesLeft, CS_CREDITS_LINE_HEIGHT, CS_CREDITS_EPILOGUE_DURATION_FRAMES);
	colorSlot = WHITE;
	if (fadeAmount < CS_CREDITS_LINE_HEIGHT)
	{
		if (fadeAmount <= 0)
			colorSlot = -1;
		else
		{
			s16 i;
			fadeAmount = (fadeAmount << 8) / CS_CREDITS_LINE_HEIGHT;
			colorSlot = CREDITS_FADE;

			for (i = 0; i < 4; i++)
			{
				u8 *dst = (u8 *)&CS_CREDITS_FADE_PALETTE[i];
				u8 *src;
				src = (u8 *)CS_COLOR_POINTERS[WHITE];
				src += i * 4;
				dst[0] = src[0] * fadeAmount >> 8;
				src = (u8 *)CS_COLOR_POINTERS[WHITE];
				src += i * 4;
				dst[1] = src[1] * fadeAmount >> 8;
				src = (u8 *)CS_COLOR_POINTERS[WHITE];
				src += i * 4;
				dst[2] = src[2] * fadeAmount >> 8;
			}
		}
	}
	if ((colorSlot >= 0) && CS_CREDITS_ALL_BLUE)
	{
		s32 strLen;
		if (co->epilogueNextString == NULL)
			strLen = -1;
		else
			strLen = (s16)((s16)(u32)co->epilogueNextString - (s16)(u32)co->epilogueTopString - 1);
		DecalFont_DrawMultiLineStrlen(co->epilogueTopString, strLen, CS_CREDITS_EPILOGUE_CENTER_X, CS_CREDITS_EPILOGUE_Y, CS_CREDITS_EPILOGUE_WIDTH, FONT_SMALL,
		                              colorSlot | JUSTIFY_CENTER);
	}
}

void CS_Credits_ThTick(void)
{
	s16 i;
	struct CreditsObj *co = &creditsBSS.creditsObj;
	struct Instance *danceInst;
	struct GameTracker *gGT;

	co->creditDanceInst = CS_CREDITS_DANCER;

	if (co->creditDanceInst != NULL)
	{
		danceInst = co->creditDanceInst;
		danceInst->flags |= HIDE_MODEL;

		danceInst->matrix.t[0] = (int)CS_CREDITS_GHOST_POS.x;
		danceInst->matrix.t[1] = (int)CS_CREDITS_GHOST_POS.y;
		danceInst->matrix.t[2] = (int)CS_CREDITS_GHOST_POS.z;

		gGT = GAME_TRACKER;

		i = 0;
		if ((gGT->timer & 3) == 0)
		{
			// Copy the trail backwards so each ghost receives the previous frame's model.
			for (i = CS_CREDITS_GHOST_COUNT - 1; i > 0; i--)
			{
				CS_Credits_AnimateCreditGhost(co->creditGhostInst[i], co->creditGhostInst[i - 1], i);
				co->creditGhostModel[i] = co->creditGhostModel[i - 1];
			}

			CS_Credits_AnimateCreditGhost(co->creditGhostInst[0], co->creditDanceInst, i);
			co->creditGhostModel[0] = co->creditDanceInst->model;
		}
		else
		{
			CS_Credits_AnimateCreditGhost(co->creditGhostInst[0], co->creditDanceInst, i);

			// Between trail snapshots, grow and fade the older ghosts in place.
			// NOTE(aalhendi): Keep the per-component slot reads to preserve retail's
			// memory access order across the short writes.
			for (i = 1; i < CS_CREDITS_GHOST_COUNT; i++)
			{
				danceInst = co->creditGhostInst[i];
				danceInst->scale.x += CS_CREDITS_GHOST_TRAIL_SCALE_STEP;
				danceInst = co->creditGhostInst[i];
				danceInst->scale.y += CS_CREDITS_GHOST_TRAIL_SCALE_STEP;
				danceInst = co->creditGhostInst[i];
				danceInst->scale.z += CS_CREDITS_GHOST_TRAIL_SCALE_STEP;
				danceInst = co->creditGhostInst[i];
				danceInst->alphaScale += CS_CREDITS_GHOST_TRAIL_ALPHA_STEP;
			}
		}
	}

	if (co->countdown > 0)
	{
		co->countdown--;
	}

	CS_Credits_DrawNames(co);
	CS_Credits_DrawEpilogue(co);
}

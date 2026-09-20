#ifndef CTR_NATIVE_OVR_233_H
#define CTR_NATIVE_OVR_233_H

enum
{
	CS_CAMERA_PAN = 0,
	CS_WAIT_INPUT = 1,
	CS_FADE_OUT = 2,
	CS_LOADING = 3,
	CS_FADE_IN = 4,
	CS_WAIT_END = 5,
};
typedef s32 CutscenePhase;

CTR_STATIC_ASSERT(sizeof(CutscenePhase) == 0x4);

struct CsThreadInitData
{
	SVec3Slot podiumPos;

	SVec3Slot characterPos;

	SVec3Slot rot;

	SVec3Slot derivedRot;
};

CTR_STATIC_ASSERT(sizeof(struct CsThreadInitData) == 0x20);
CTR_STATIC_ASSERT(offsetof(struct CsThreadInitData, podiumPos) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct CsThreadInitData, characterPos) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct CsThreadInitData, rot) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct CsThreadInitData, derivedRot) == 0x18);

struct CsPodiumCameraThreadObj
{
	s16 pathFrame32;
	u16 pad_02;
};

CTR_STATIC_ASSERT(offsetof(struct CsPodiumCameraThreadObj, pathFrame32) == 0x0);
CTR_STATIC_ASSERT(sizeof(struct CsPodiumCameraThreadObj) == 0x4);

struct Prize
{
	// 0x0
	SVec3 posStart;
	s16 _pad_posStart;

	// 0x8
	SVec3 rot;
	s16 _pad_rot;

	// 0x10
	SVec3 targetScreenPos;
	s16 _pad_targetScreenPos;

	// 0x18
	s16 specLightVerticalStart;
	s16 specLightHorizontalStart;
	s16 specLightVerticalEnd;
	s16 specLightHorizontalEnd;

	// 0x20
	s16 specLightPhase;
	s16 orbitRadius;
	s16 heightOffset;

	// 0x26
	s16 flyToHudFramesLeft;

	// 0x28
	s16 flyToHudFramesTotal;

	// 0x2a
	s16 bounceFrameIndex;

	// 0x2c -- size of struct
};

CTR_STATIC_ASSERT(offsetof(struct Prize, posStart) == 0x0);
CTR_STATIC_ASSERT(offsetof(struct Prize, rot) == 0x8);
CTR_STATIC_ASSERT(offsetof(struct Prize, targetScreenPos) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct Prize, specLightVerticalStart) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct Prize, specLightPhase) == 0x20);
CTR_STATIC_ASSERT(offsetof(struct Prize, flyToHudFramesLeft) == 0x26);
CTR_STATIC_ASSERT(offsetof(struct Prize, bounceFrameIndex) == 0x2a);
CTR_STATIC_ASSERT(sizeof(struct Prize) == 0x2c);

struct CsParticleConfigMeta
{
	u8 iconGroupIndex;
	u8 frameOffset;
	u8 count;
	u8 flags;
};

struct CsParticleConfigSpawn
{
	s8 modelDelta;
	u8 pad[3];
};

struct CsParticleConfig
{
	struct ParticleEmitter *emitter;
	struct CsParticleConfigMeta meta;
	struct CsParticleConfigSpawn spawn;
};

CTR_STATIC_ASSERT(sizeof(struct CsParticleConfigMeta) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct CsParticleConfigSpawn) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct CsParticleConfig) == 0xc);

union CsOpcodeArg
{
	int i;
	u32 u;
	char *ptr;
};

struct CsOpcodeMeta
{
	s16 opcode;
	s16 animIndex;
	s16 frameStart;
	s16 frameEnd;
	union CsOpcodeArg arg0; // shorts 4/5
	union CsOpcodeArg arg1; // shorts 6/7
	s16 rotStart;
	s16 rotEnd;
};

typedef int CsOpcodeWord CTR_MAY_ALIAS;
typedef s16 CsOpcodeHalf CTR_MAY_ALIAS;
typedef s16 CsInitMatrixHalf CTR_MAY_ALIAS;

static inline CsOpcodeWord *CsOpcodeMeta_Words(struct CsOpcodeMeta *meta)
{
	return (CsOpcodeWord *)meta;
}

static inline const CsOpcodeWord *CsOpcodeMeta_ConstWords(const struct CsOpcodeMeta *meta)
{
	return (const CsOpcodeWord *)meta;
}

static inline CsOpcodeHalf *CsOpcodeMeta_Halves(struct CsOpcodeMeta *meta)
{
	return (CsOpcodeHalf *)meta;
}

enum CutsceneOpcode
{
	CS_OPCODE_ANIM_ROT_RANGE = 0x00,
	CS_OPCODE_GOTO = 0x01,
	CS_OPCODE_HIDE_INSTANCE_AND_END_THREAD = 0x02,
	CS_OPCODE_SPAWN_CHILD = 0x03,
	CS_OPCODE_BRANCH_IF_RANDOM_LE = 0x04,
	CS_OPCODE_PLAY_CONTEXT_FX = 0x05,
	CS_OPCODE_STOP_FX = 0x06,
	CS_OPCODE_START_MUSIC = 0x07,
	CS_OPCODE_RESTART_MUSIC = 0x08,
	CS_OPCODE_SET_VISIBLE_LOD = 0x09,
	CS_OPCODE_SET_PATH_MOTION = 0x0a,
	CS_OPCODE_SCALE_TO = 0x0b,
	CS_OPCODE_FADE_FROM_BLACK = 0x0c,
	CS_OPCODE_SET_CUTSCENE_FLAGS = 0x0d,
	CS_OPCODE_CLEAR_CUTSCENE_FLAGS = 0x0e,
	CS_OPCODE_RESELECT_LEVEL_SCRIPT = 0x0f,
	CS_OPCODE_REQUEST_LEVEL = 0x10,
	CS_OPCODE_WAIT_SWAP_QUEUE = 0x11,
	CS_OPCODE_PLAY_XA = 0x12,
	CS_OPCODE_WAIT_XA_DONE = 0x13,
	CS_OPCODE_YIELD = 0x14,
	CS_OPCODE_LOAD_LEVEL_STARS = 0x15,
	CS_OPCODE_RACEFLAG_TRANSITION1_IF_OFFSCREEN = 0x16,
	CS_OPCODE_ADVANCE_IF_RACEFLAG_ONSCREEN = 0x17,
	CS_OPCODE_RACEFLAG_TRANSITION2_IF_ONSCREEN = 0x18,
	CS_OPCODE_SET_PARTICLE_ID = 0x19,
	CS_OPCODE_HIDE_INSTANCE = 0x1a,
	CS_OPCODE_SHOW_INSTANCE = 0x1b,
	CS_OPCODE_ADD_INSTANCE_DEPTH_BIAS = 0x1c,
	CS_OPCODE_SET_INSTANCE_FLAGS = 0x1d,
	CS_OPCODE_CLEAR_INSTANCE_FLAGS = 0x1e,
	CS_OPCODE_SET_UNK4_1333 = 0x1f,
	CS_OPCODE_END_BOSS_CUTSCENE = 0x20,
	CS_OPCODE_SET_BOSS_CUTSCENE_INDEX = 0x21,
	CS_OPCODE_SET_CAMERA_DISTANCE = 0x22,
	CS_OPCODE_ADVANCE_IF_CREDITS_TEXT_VALID = 0x23,
	CS_OPCODE_CREDITS_DANCER = 0x24,
	CS_OPCODE_ADVANCE_IF_CREDITS_GHOSTS_READY = 0x25,
	CS_OPCODE_BRANCH_ADV_CHAR_SELECT = 0x26,
	CS_OPCODE_ADVANCE_IF_LEVEL_TIME_REACHED = 0x27,
	CS_OPCODE_SYNC_ANIM_FRAME = 0x28,
	CS_OPCODE_END_CREDITS = 0x29,
	CS_OPCODE_ANIM_SYNC_MARKER = 0x2a,
	CS_OPCODE_ANIM_RANGE = 0x2b,
	CS_OPCODE_SET_GAME_MODE_FLAGS = 0x2c,
	CS_OPCODE_SET_SUBTITLE = 0x2d,
	CS_OPCODE_UI_FADE_TO_BLACK = 0x2e,
	CS_OPCODE_WAIT_UI_FADE = 0x2f,
	CS_OPCODE_SET_AUDIO_VOLUME = 0x30,
};

enum CutsceneGameModeTarget
{
	CS_GAME_MODE_TARGET_GAME_MODE1 = 0,
	CS_GAME_MODE_TARGET_GAME_MODE2 = 1,
	CS_GAME_MODE_TARGET_RENDER_FLAGS_SET = 2,
	CS_GAME_MODE_TARGET_RENDER_FLAGS_CLEAR = 3,
};

enum CsOpcodeMetaFlags
{
	CS_OPCODE_META_HAS_ANIM_INDEX = 0x01,
	CS_OPCODE_META_HAS_FRAME_START = 0x02,
	CS_OPCODE_META_HAS_FRAME_END = 0x04,
	CS_OPCODE_META_HAS_ARG0 = 0x08,
	CS_OPCODE_META_HAS_ARG1 = 0x10,
	CS_OPCODE_META_HAS_ALIGNED_ARG1 = 0x20,
	CS_OPCODE_META_HAS_ROT_START = 0x40,
	CS_OPCODE_META_HAS_ROT_END = 0x80,
};


enum CutsceneObjFlags
{
	CS_FLAG_PATH_MOTION_DISABLED = 0x0001,
	CS_FLAG_RANDOM_ALPHA_SCALE = 0x0002,
	CS_FLAG_SKIP_PARENT_FRAME_TRANSFORM = 0x0004,
	CS_FLAG_WRITE_VERT_SPLIT_LINE = 0x0008,
	CS_FLAG_SKIP_PARENT_ROTATION = 0x0010,
	CS_FLAG_CAMERA_DISTANCE_OVERRIDE = 0x0020,
	CS_FLAG_INTERPOLATE_FRAMES_MS = 0x0040,
	CS_FLAG_ADV_CHAR_SELECT_LOGIC = 0x0080,
	CS_FLAG_ADV_CHAR_SELECT_SELECTED = 0x0100,
	CS_FLAG_XA_SYNC_ANIMATION = 0x0200,
	CS_FLAG_XA_PLAYBACK_STARTED = 0x0400,
	CS_FLAG_SOUND_ONSCREEN_ONLY = 0x1000,
};


struct Ovr233InitMatrixTableEntry;

struct CutsceneObj
{
	// 0x0
	struct IconGroup *ptrIcons;
	// 0x4
	s16 unk4;
	// 0x6
	s16 unk6;

	/* 0x8
	jittery number? color?
	usually has value of 0x2e808080 */
	int unk8;

	// 0xC
	s16 unk_C;
	// 0xE
	s16 unk_E;

	// 0x10
	struct CsOpcodeMeta *metadataMeta;

	// 0x14
	s16 opcodeDuration;

	// 0x16
	// determines time or frame animation
	u16 flags;

	// 1/32-frame animation or path time.
	int animFrame32;

	// Base yaw added after opcode rotation interpolation.
	s16 baseRotY;
	s16 lodIndex;
	SVec3 rot;
	s16 rotPad;
	u16 pathProgress32;

	// 0x2a
	s16 scaleSpeed;
	// 0x2c
	s16 desiredScale;

	struct
	{
		// 0x2e
		SVec2 textPos;

		// 0x32
		// index to subtitle text in LNG
		// -1 to disable
		s16 lngIndex;

		// 0x34
		s16 font;

		// 0x36
		s16 colors;

	} Subtitles;


	// 0x38
	char *currOpcode[2];
	// 0x40
	char *prevOpcode;

	// 0x44
	s8 particleID;
	u8 pad_45[2];
	// 0x47
	u8 animIndex;

	// 0x48
	struct Ovr233InitMatrixTableEntry *frameOverrideRoot;

	// 0x4c
	struct CsOpcodeMeta decodedOpcode;
};

#ifndef CTR_NATIVE
CTR_STATIC_ASSERT(sizeof(struct CsOpcodeMeta) == 0x14);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, rotPad) == 0x26);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, pathProgress32) == 0x28);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, particleID) == 0x44);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, pad_45) == 0x45);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, animIndex) == 0x47);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, frameOverrideRoot) == 0x48);
CTR_STATIC_ASSERT(OFFSETOF(struct CutsceneObj, decodedOpcode) == 0x4c);
CTR_STATIC_ASSERT(sizeof(struct CutsceneObj) == 0x60);
#endif

enum BOSS_CUTSCENE_ORDER
{
	// Gemstone
	OXIDE_TROPHIES,
	PINSTRIPE_BEAT,

	// Beach
	ROO_START,
	ROO_BEAT,

	// ruins
	PAPU_START,
	PAPU_BEAT,

	// glacier
	KJOE_START,
	KJOE_BEAT,

	// citadel
	PINSTRIPE_START,

	// 0x9
	OXIDE_RELICS_GEMSTONE,
	OXIDE_RELICS_BEACH,
	OXIDE_RELICS_RUINS,
	OXIDE_RELICS_GLACIER,
	OXIDE_RELICS_CITADEL,

	// 0xE
	BOSS_CUTSCENE_COUNT,
};

enum CsBossModelPart
{
	CS_BOSS_MODEL_HEAD,
	CS_BOSS_MODEL_BODY,
	CS_BOSS_MODEL_COUNT,
};

struct BossCutsceneData
{
	// 0x0
	// Optional texture, then head and body model files.
	s32 fileIDs[1 + CS_BOSS_MODEL_COUNT];

	// Unused, cause it does model->id
	// to get the model index anyway
	int modelIndex_unused;

	// 0x10
	char *opcode;

	// 0x14
	SVec3 camPos;
	s16 _pad_camPos;
	SVec3 camRot;
	s16 _pad_camRot;

	// 0x24
	SVec3 bossPos;
	s16 _pad_bossPos;
	SVec3 bossRot;
	s16 _pad_bossRot;

	// 0x34
};

struct CsInitMatrixEntry
{
	s16 offset[4];
	union
	{
		s16 raw[10];
		u32 words[5];
		struct
		{
			SVec3 rot;
			s16 rotPad;
			SVec3 scale;
			s16 scalePad;
			s16 matrixTail[2];
		} fields;
	} matrix;
	s16 pad[2];
};

static inline const CsInitMatrixHalf *CsInitMatrixEntry_ConstHalves(const struct CsInitMatrixEntry *entry)
{
	return entry->matrix.raw;
}

CTR_STATIC_ASSERT(sizeof(struct CsInitMatrixEntry) == 0x20);

struct Ovr233InitMatrixTableEntry
{
	struct CsInitMatrixEntry *data;
	int count;
};

CTR_STATIC_ASSERT(sizeof(struct Ovr233InitMatrixTableEntry) == 0x8);

struct CsThreadNames
{
	char s_spawn[8];
	char s_g_dancer[16];
};

extern const struct CsThreadNames csThreadNames;

struct CsPodiumNames
{
	char s_podium[8];
	char s_third[8];
	char s_second[8];
	char s_first[8];
	char s_tawna[8];
	char s_prize[8];
	char s_victorycam[16];
};

extern const struct CsPodiumNames csPodiumNames;

struct CsIntroNames
{
	char s_introguy[12];
	char s_introcam[12];
	char s_box1[8];
	char s_box2[8];
	char s_box2_bottom[16];
	char s_box2_front[16];
	char s_box2_A[8];
	char s_box3[8];
	char s_code[8];
	char s_glow[8];
	char s_lid[4];
	char s_lidb[8];
	char s_lidc[8];
	char s_lidd[8];
	char s_lid2[8];
	char s_kart0[8];
	char s_kart1[8];
	char s_kart2[8];
	char s_kart3[8];
	char s_kart6[8];
	char s_kart7[8];
};

extern const struct CsIntroNames csIntroNames;

#ifndef CS_INTRO_NAME
#define CS_INTRO_NAME(field) (csIntroNames.field)
#endif

struct CsCreditsNames
{
	char credits[8];
	char creditGhost[12];
	char creditStrings[16];
};

extern const struct CsCreditsNames csCreditsNames;

struct OverlayDATA_233
{
	// 800b0b7c
	int VertSplitLine;

	// 800b0b80
	int boolLoadNextSwap;

	// 800b0b84
	int boolStartToSkip;

	// 800b0b88
	int bossCutsceneIndex;

	// 800b0b8c
	b16 CutsceneManipulatesAudio;
	u16 audioControlPadding;

	// 800b0b90
	struct ParticleEmitter particleEmitterData[63];

	// 800b146c
	struct CsParticleConfig particleConfigs[8];

	// 800b14cc
	// Opcode format flags, followed immediately by the first cutscene script.
	char csOpcodeMetaPrefix[0x34];

	// 800b1500
	char bossOpcodeData[0x2b4];

	// 800b17b4
	char script_tawnaNormal[0x28];

	// 800b17dc
	char script_tawnaCredits[0x164c];

	// 800b2e28
	char script_default[0x18];

	// 800b2e40
	char script_dingofire[0x38];

	// 800b2e78
	char *danceFirstScripts[0x10];

	// 800b2eb8
	char danceOtherOpcodeData[0x12d4];

	// 800b418c
	char *danceOtherScripts[0x10];

	// 800b41cc
	char introModelOpcodeData[0x3b0];

	// 800b457c
	char *introModelScripts[0x10];

	// 800b45bc
	char introCutsceneOpcodeData[0x11c];

	// 800b46d8
	// NOTE(aalhendi): Retail PTR_DAT_800b45bc_800b46d8.
	char *introCutsceneOpcodes[9];

	// 800b46fc
	char introEndingOpcodeData[0x50];

	// 800b474c
	char creditsCutsceneOpcodeData[0x1dc];

	// 800b4928
	// NOTE(aalhendi): Retail PTR_DAT_800b474c_800b4928.
	char *creditsCutsceneOpcodes[20];

	// 800b4978
	// ND crate intro script starts at offset 0x18.
	char creditsOpcodeData[0x40];

	// 800b49b8
	char boxAndAdvCharSelectOpcodeData[0x10c4];

	// 800b5a7c
	// NOTE(aalhendi): Retail PTR_DAT_800b49b8_800b5a7c.
	char *boxModelScripts[0x2b];

	// 800b5b28
	// NOTE(aalhendi): Retail PTR_DAT_800b4fe4_800b5b28.
	char *advCharSelectSelectOpcodes[8];

	// 800b5b48
	// NOTE(aalhendi): Retail PTR_DAT_800b5024_800b5b48.
	char *advCharSelectDeselectOpcodes[8];

	char *boxAndAdvCharSelectExtraOpcodes[2];

	// 800b5b70
	struct CsInitMatrixEntry cs_initMatrixData[190];

	// 800b7330
	// NOTE(aalhendi): Retail PTR_DAT_800b5b70_800b7330.
	struct Ovr233InitMatrixTableEntry cs_initMatrixTable[4];

	// 800b7350
	char cs_initMatrixBool;

	// 800b7351
	u8 pad_afterInitMatrix[3];
	struct ParticleEmitter unusedIntroEmitter[8];

	// 800b7474
	Color introClearBoxColor;

	// 800b7478
	RECT introClearBoxRect;

	// 800b7480
	SVec3 creditsDancerRotOffset;
	s16 _pad_creditsDancerRotOffset;


	// 800B7488
	struct BossCutsceneData bossCS[0xE];

	// 800b7760
	int isCutsceneOver;
	// 800b7764
	int podiumCameraFrame;
	// 800b7768
	s16 volumeBackup[3];
	// 800b776e
	s16 audioVolumeBackupPad;

	// 800b7770
	int podiumPrizeDropReady;
	// 800b7774
	CutscenePhase cutsceneState;

	// 800b7778
	struct Model *bossModels[CS_BOSS_MODEL_COUNT];

	// 800b7780
};


extern struct OverlayDATA_233 D233;

#ifndef CS_VERT_SPLIT
#define CS_VERT_SPLIT          (D233.VertSplitLine)
#define CS_LOAD_NEXT_SWAP      (D233.boolLoadNextSwap)
#define CS_CAN_SKIP            (D233.boolStartToSkip)
#define CS_BOSS_INDEX          (D233.bossCutsceneIndex)
#define CS_CONTROLS_AUDIO      (D233.CutsceneManipulatesAudio)
#define CS_MATRIX_TABLE        (D233.cs_initMatrixTable)
#define CS_MATRIX_INITIALIZED  (D233.cs_initMatrixBool)
#define CS_FINISHED            (D233.isCutsceneOver)
#define CS_PODIUM_CAMERA_FRAME (D233.podiumCameraFrame)
#define CS_VOLUME_BACKUP       (D233.volumeBackup)
#define CS_PRIZE_DROP_READY    (D233.podiumPrizeDropReady)
#define CS_PHASE               (D233.cutsceneState)
#define CS_BOSS_MODELS         (D233.bossModels)
#define CS_BIGFILE_HEADER      (sdata->ptrBigfileCdPos_2)
#define CS_LOAD_IN_PROGRESS    (sdata->load_inProgress)
#define CS_MAIN_MENU_STATE     (sdata->mainMenuState)
#define CS_XA_STATE            (sdata->XA_State)
#define CS_XA_OFFSET           (sdata->XA_CurrOffset)
#define CS_QUEUE_READY         (sdata->queueReady)
#define CS_QUEUE_LENGTH        (sdata->queueLength)
#define CS_GARAGE_CURRENT      (sdata->advCharSelectIndex_curr)
#define CS_GARAGE_PREVIOUS     (sdata->advCharSelectIndex_prev)
#define CS_FRAME_COUNTER_LOW   ((u16)sdata->frameCounter)
#define CS_BUTTONS_HOLD        (sdata->AnyPlayerHold)
#define CS_DESIRED_MENU        (sdata->ptrDesiredMenu)
#endif

#define CS_BOSS_HEAD_MODEL     (CS_BOSS_MODELS[CS_BOSS_MODEL_HEAD])
#define CS_BOSS_BODY_MODEL     (CS_BOSS_MODELS[CS_BOSS_MODEL_BODY])

#define CS_FX_VOLUME_BACKUP    (CS_VOLUME_BACKUP[HOWL_VOLUME_TYPE_FX])
#define CS_MUSIC_VOLUME_BACKUP (CS_VOLUME_BACKUP[HOWL_VOLUME_TYPE_MUSIC])
#define CS_VOICE_VOLUME_BACKUP (CS_VOLUME_BACKUP[HOWL_VOLUME_TYPE_VOICE])

#ifndef CS_PODIUM_FIRST_NAME
#define CS_PODIUM_FIRST_NAME      (csPodiumNames.s_first)
#define CS_PODIUM_TAWNA_NAME      (csPodiumNames.s_tawna)
#define CS_PODIUM_PRIZE_NAME      (csPodiumNames.s_prize)
#define CS_PODIUM_VICTORYCAM_NAME (csPodiumNames.s_victorycam)
#endif

#ifndef CS_INTRO_MODEL_SCRIPTS
#define CS_INTRO_MODEL_SCRIPTS (D233.introModelScripts)
#define CS_BOX_MODEL_SCRIPTS   (D233.boxModelScripts)
#endif

#ifndef CS_SCRIPT
#define CS_SCRIPT(field) (D233.field)
#endif

// Cutscene data begins at retail 0x800b0b7c; code is linked separately.
#define OVR233_LAYOUT_ASSERT(ELEMENT, OFFSET, SIZE)                                    \
	CTR_STATIC_ASSERT(OFFSETOF(struct OverlayDATA_233, ELEMENT) == (OFFSET) - 0x518c); \
	CTR_STATIC_ASSERT(sizeof(((struct OverlayDATA_233 *)0)->ELEMENT) == (SIZE))

CTR_STATIC_ASSERT(sizeof(void *) == 4);
OVR233_LAYOUT_ASSERT(VertSplitLine, 0x518c, 0x4);
OVR233_LAYOUT_ASSERT(boolLoadNextSwap, 0x5190, 0x4);
OVR233_LAYOUT_ASSERT(boolStartToSkip, 0x5194, 0x4);
OVR233_LAYOUT_ASSERT(bossCutsceneIndex, 0x5198, 0x4);
OVR233_LAYOUT_ASSERT(CutsceneManipulatesAudio, 0x519c, 0x2);
OVR233_LAYOUT_ASSERT(audioControlPadding, 0x519e, 0x2);
OVR233_LAYOUT_ASSERT(particleEmitterData, 0x51a0, 0x8dc);
OVR233_LAYOUT_ASSERT(particleConfigs, 0x5a7c, 0x60);
OVR233_LAYOUT_ASSERT(csOpcodeMetaPrefix, 0x5adc, 0x34);
OVR233_LAYOUT_ASSERT(bossOpcodeData, 0x5b10, 0x2b4);
OVR233_LAYOUT_ASSERT(script_tawnaNormal, 0x5dc4, 0x28);
OVR233_LAYOUT_ASSERT(script_tawnaCredits, 0x5dec, 0x164c);
OVR233_LAYOUT_ASSERT(script_default, 0x7438, 0x18);
OVR233_LAYOUT_ASSERT(script_dingofire, 0x7450, 0x38);
OVR233_LAYOUT_ASSERT(danceFirstScripts, 0x7488, 0x40);
OVR233_LAYOUT_ASSERT(danceOtherOpcodeData, 0x74c8, 0x12d4);
OVR233_LAYOUT_ASSERT(danceOtherScripts, 0x879c, 0x40);
OVR233_LAYOUT_ASSERT(introModelOpcodeData, 0x87dc, 0x3b0);
OVR233_LAYOUT_ASSERT(introModelScripts, 0x8b8c, 0x40);
OVR233_LAYOUT_ASSERT(introCutsceneOpcodeData, 0x8bcc, 0x11c);
OVR233_LAYOUT_ASSERT(introCutsceneOpcodes, 0x8ce8, 0x24);
OVR233_LAYOUT_ASSERT(introEndingOpcodeData, 0x8d0c, 0x50);
OVR233_LAYOUT_ASSERT(creditsCutsceneOpcodeData, 0x8d5c, 0x1dc);
OVR233_LAYOUT_ASSERT(creditsCutsceneOpcodes, 0x8f38, 0x50);
OVR233_LAYOUT_ASSERT(creditsOpcodeData, 0x8f88, 0x40);
OVR233_LAYOUT_ASSERT(boxAndAdvCharSelectOpcodeData, 0x8fc8, 0x10c4);
OVR233_LAYOUT_ASSERT(boxModelScripts, 0xa08c, 0xac);
OVR233_LAYOUT_ASSERT(advCharSelectSelectOpcodes, 0xa138, 0x20);
OVR233_LAYOUT_ASSERT(advCharSelectDeselectOpcodes, 0xa158, 0x20);
OVR233_LAYOUT_ASSERT(boxAndAdvCharSelectExtraOpcodes, 0xa178, 0x8);
OVR233_LAYOUT_ASSERT(cs_initMatrixData, 0xa180, 0x17c0);
OVR233_LAYOUT_ASSERT(cs_initMatrixTable, 0xb940, 0x20);
OVR233_LAYOUT_ASSERT(cs_initMatrixBool, 0xb960, 0x1);
OVR233_LAYOUT_ASSERT(introClearBoxColor, 0xba84, 0x4);
OVR233_LAYOUT_ASSERT(introClearBoxRect, 0xba88, 0x8);
OVR233_LAYOUT_ASSERT(creditsDancerRotOffset, 0xba90, 0x6);
OVR233_LAYOUT_ASSERT(_pad_creditsDancerRotOffset, 0xba96, 0x2);
OVR233_LAYOUT_ASSERT(bossCS, 0xba98, 0x2d8);
OVR233_LAYOUT_ASSERT(isCutsceneOver, 0xbd70, 0x4);
OVR233_LAYOUT_ASSERT(podiumCameraFrame, 0xbd74, 0x4);
OVR233_LAYOUT_ASSERT(volumeBackup, 0xbd78, 0x6);
OVR233_LAYOUT_ASSERT(audioVolumeBackupPad, 0xbd7e, 0x2);
OVR233_LAYOUT_ASSERT(podiumPrizeDropReady, 0xbd80, 0x4);
OVR233_LAYOUT_ASSERT(cutsceneState, 0xbd84, 0x4);
OVR233_LAYOUT_ASSERT(bossModels, 0xbd88, 0x8);
CTR_STATIC_ASSERT(sizeof(struct OverlayDATA_233) == 0x6c04);

#undef OVR233_LAYOUT_ASSERT

// starts at 800b8598,
// last byte of Garage Functions
struct OVR233_Garage
{
	// 800b8598
	struct RectMenu menuGarage;

	// 800b85c4
	int numFramesMax_GarageMove;
	int padding1;

	// 800b85cc
	int numFramesMax_Zoom;

	// 800b85d0
	int fovMin;

	// 800b85d4
	int fovMax;

	// 800b85d8
	s16 garageCharacterIDs[8];

	// 800b85e8
	s16 statBarLengths[3];

	// 800b85ee
	s16 unusedFrameCount;

	// 800b85f0
	s16 classStringIDs[4];

	// 800b85f8
	s16 statBarTargetLengths[3 * 4];

	// 800b8610
	int unusedArr_Colors[3];

	// 800b861c
	u32 statBarSegmentColors[7];

	// 800b8638
	s16 numFramesCurr_GarageMove;

	// 800b863a
	s16 numFramesCurr_ZoomIn;

	// 800b863c
	s16 numFramesCurr_ZoomOut;

	// 800b863e
	s16 delayOneSecond;

	// 800b8640
	s16 boolSelected;
	s16 padding2;

	// === End of Garage Data ===
	// Credits RDATA starts next byte
};

// Garage data starts at 0x800b8598.
#define OVR233_GARAGE_ASSERT(ELEMENT, OFFSET, SIZE)                         \
	CTR_STATIC_ASSERT(OFFSETOF(struct OVR233_Garage, ELEMENT) == (OFFSET)); \
	CTR_STATIC_ASSERT(sizeof(((struct OVR233_Garage *)0)->ELEMENT) == (SIZE))

OVR233_GARAGE_ASSERT(menuGarage, 0x0, 0x2c);
OVR233_GARAGE_ASSERT(numFramesMax_GarageMove, 0x2c, 0x4);
OVR233_GARAGE_ASSERT(padding1, 0x30, 0x4);
OVR233_GARAGE_ASSERT(numFramesMax_Zoom, 0x34, 0x4);
OVR233_GARAGE_ASSERT(fovMin, 0x38, 0x4);
OVR233_GARAGE_ASSERT(fovMax, 0x3c, 0x4);
OVR233_GARAGE_ASSERT(garageCharacterIDs, 0x40, 0x10);
OVR233_GARAGE_ASSERT(statBarLengths, 0x50, 0x6);
OVR233_GARAGE_ASSERT(unusedFrameCount, 0x56, 0x2);
OVR233_GARAGE_ASSERT(classStringIDs, 0x58, 0x8);
OVR233_GARAGE_ASSERT(statBarTargetLengths, 0x60, 0x18);
OVR233_GARAGE_ASSERT(unusedArr_Colors, 0x78, 0xc);
OVR233_GARAGE_ASSERT(statBarSegmentColors, 0x84, 0x1c);
OVR233_GARAGE_ASSERT(numFramesCurr_GarageMove, 0xa0, 0x2);
OVR233_GARAGE_ASSERT(numFramesCurr_ZoomIn, 0xa2, 0x2);
OVR233_GARAGE_ASSERT(numFramesCurr_ZoomOut, 0xa4, 0x2);
OVR233_GARAGE_ASSERT(delayOneSecond, 0xa6, 0x2);
OVR233_GARAGE_ASSERT(boolSelected, 0xa8, 0x2);
CTR_STATIC_ASSERT(sizeof(struct OVR233_Garage) == 0xac);

#undef OVR233_GARAGE_ASSERT

extern struct OVR233_Garage gGarage;

#if 0
800b8644 - CREDITS RDATA
800b8668 - first byte of credits func (CREDITS TEXT)
#endif

// === Credits Functions Here ===

// 800b9488 - first byte after last func (CREDITS BSS)

struct CreditsLevHeader
{
	int size;
	s16 numStrings;
	s16 unused_06;

	// char* ptrStrings[0];
};

#define CREDITSHEADER_GETSTRINGS(x) ((u32)x + sizeof(struct CreditsLevHeader))

#ifndef CTR_NATIVE
CTR_STATIC_ASSERT(OFFSETOF(struct CreditsLevHeader, numStrings) == 0x4);
CTR_STATIC_ASSERT(sizeof(struct CreditsLevHeader) == 0x8);
#endif

enum CreditsConstants
{
	CS_CREDITS_GHOST_COUNT = 5,
};

struct CreditsObj
{
	// 800b94bc (000)
	struct Model *creditGhostModel[CS_CREDITS_GHOST_COUNT]; // duplicates

	// 800b94d0 (014)
	struct Instance *creditGhostInst[CS_CREDITS_GHOST_COUNT];

	// 800b94e4 (028)
	struct ModelHeader creditGhostHeaders[CS_CREDITS_GHOST_COUNT][2];

	// 800b9764 (2a8)
	struct Model creditGhostModelCopies[CS_CREDITS_GHOST_COUNT];

	// 800b97dc (320)
	s16 countdown;

	// 800b97de (322)
	s16 unused_322;

	// 800b97e0 (324)
	int unused_324;

	// 800b97e4 (328)
	struct Instance *creditDanceInst; // base for copies

	// 800b97e8 (32c)
	s16 creditsPosY;

	// 800b97ea (32e)
	s16 unused_32e;

	// 800b97ec (330)
	char *creditsTopString;

	// 800b97f0 (334)
	char *epilogueTopString;

	// 800b97f4 (338)
	char *epilogueNextString;

	// 800b97f8 (33c)
	s16 epilogueFramesLeft;

	// 800b97fa (33e)
	s16 epiloguePosX_unused; // unused
};

#ifndef CTR_NATIVE
CTR_STATIC_ASSERT(OFFSETOF(struct CreditsObj, countdown) == 0x320);
CTR_STATIC_ASSERT(OFFSETOF(struct CreditsObj, creditsPosY) == 0x32c);
CTR_STATIC_ASSERT(OFFSETOF(struct CreditsObj, creditsTopString) == 0x330);
CTR_STATIC_ASSERT(sizeof(struct CreditsObj) == 0x340);
#endif

struct Ovr233_Credits_BSS
{
	// 800b9488
	SVec3 creditGhostPos;
	s16 _pad_creditGhostPos;

	// 800b9490
	SVec3 unused_Pos;
	s16 _pad_unused_Pos;

	// 800b9498
	int creditTextPosX;

	// 800b949c
	struct Thread *creditThread;

	// 800b94a0
	struct Thread *dancerThread;

	// 800b94a4
	struct Instance *dancerInst_invisible;

	// 800b94a8
	s16 numStrings;

	// 800b94aa
	s16 unused_94aa;

	// 800b94ac
	char **ptrStrings;

	// 800b94b0
	s16 boolAllBlue;

	// 800b94b2
	s16 unused_94b2;

	// 800b94b4
	int unused[2];

	// 800b94bc - CreditsObj (0x340 large)
	struct CreditsObj creditsObj;
};

// Credits BSS starts at 0x800b9488.
#define OVR233_CREDITS_OBJ_ASSERT(ELEMENT, OFFSET, SIZE)                 \
	CTR_STATIC_ASSERT(OFFSETOF(struct CreditsObj, ELEMENT) == (OFFSET)); \
	CTR_STATIC_ASSERT(sizeof(((struct CreditsObj *)0)->ELEMENT) == (SIZE))

OVR233_CREDITS_OBJ_ASSERT(creditGhostModel, 0x0, 0x14);
OVR233_CREDITS_OBJ_ASSERT(creditGhostInst, 0x14, 0x14);
OVR233_CREDITS_OBJ_ASSERT(creditGhostHeaders, 0x28, 0x280);
OVR233_CREDITS_OBJ_ASSERT(creditGhostModelCopies, 0x2a8, 0x78);
OVR233_CREDITS_OBJ_ASSERT(countdown, 0x320, 0x2);
OVR233_CREDITS_OBJ_ASSERT(unused_324, 0x324, 0x4);
OVR233_CREDITS_OBJ_ASSERT(creditDanceInst, 0x328, 0x4);
OVR233_CREDITS_OBJ_ASSERT(creditsPosY, 0x32c, 0x2);
OVR233_CREDITS_OBJ_ASSERT(creditsTopString, 0x330, 0x4);
OVR233_CREDITS_OBJ_ASSERT(epilogueTopString, 0x334, 0x4);
OVR233_CREDITS_OBJ_ASSERT(epilogueNextString, 0x338, 0x4);
OVR233_CREDITS_OBJ_ASSERT(epilogueFramesLeft, 0x33c, 0x2);
OVR233_CREDITS_OBJ_ASSERT(epiloguePosX_unused, 0x33e, 0x2);
CTR_STATIC_ASSERT(sizeof(struct CreditsObj) == 0x340);

#undef OVR233_CREDITS_OBJ_ASSERT

#define OVR233_CREDITS_BSS_ASSERT(ELEMENT, OFFSET, SIZE)                         \
	CTR_STATIC_ASSERT(OFFSETOF(struct Ovr233_Credits_BSS, ELEMENT) == (OFFSET)); \
	CTR_STATIC_ASSERT(sizeof(((struct Ovr233_Credits_BSS *)0)->ELEMENT) == (SIZE))

OVR233_CREDITS_BSS_ASSERT(creditGhostPos, 0x0, 0x6);
OVR233_CREDITS_BSS_ASSERT(_pad_creditGhostPos, 0x6, 0x2);
OVR233_CREDITS_BSS_ASSERT(unused_Pos, 0x8, 0x6);
OVR233_CREDITS_BSS_ASSERT(_pad_unused_Pos, 0xe, 0x2);
OVR233_CREDITS_BSS_ASSERT(creditTextPosX, 0x10, 0x4);
OVR233_CREDITS_BSS_ASSERT(creditThread, 0x14, 0x4);
OVR233_CREDITS_BSS_ASSERT(dancerThread, 0x18, 0x4);
OVR233_CREDITS_BSS_ASSERT(dancerInst_invisible, 0x1c, 0x4);
OVR233_CREDITS_BSS_ASSERT(numStrings, 0x20, 0x2);
OVR233_CREDITS_BSS_ASSERT(ptrStrings, 0x24, 0x4);
OVR233_CREDITS_BSS_ASSERT(boolAllBlue, 0x28, 0x2);
OVR233_CREDITS_BSS_ASSERT(unused, 0x2c, 0x8);
OVR233_CREDITS_BSS_ASSERT(creditsObj, 0x34, 0x340);
CTR_STATIC_ASSERT(sizeof(struct Ovr233_Credits_BSS) == 0x374);

#undef OVR233_CREDITS_BSS_ASSERT

extern struct Ovr233_Credits_BSS creditsBSS;

#ifndef CS_CREDITS_THREAD
#define CS_CREDITS_THREAD        (creditsBSS.creditThread)
#define CS_CREDITS_DANCER_THREAD (creditsBSS.dancerThread)
#define CS_CREDITS_DANCER        (creditsBSS.dancerInst_invisible)
#define CS_CREDITS_STRING_COUNT  (creditsBSS.numStrings)
#define CS_CREDITS_STRINGS       (creditsBSS.ptrStrings)
#define CS_CREDITS_ALL_BLUE      (creditsBSS.boolAllBlue)
#define CS_CREDITS_GHOST_POS     (creditsBSS.creditGhostPos)
#define CS_CREDITS_TEXT_X        (creditsBSS.creditTextPosX)
#endif

#ifndef CS_COLOR_POINTERS
#define CS_COLOR_POINTERS       (data.ptrColor)
#define CS_CREDITS_FADE_PALETTE (data.colors[CREDITS_FADE])
#endif

#ifndef CS_GARAGE_ZOOM_FRAMES
#define CS_GARAGE_ZOOM_FRAMES   (gGarage.numFramesMax_Zoom)
#define CS_GARAGE_MOVE_FRAME    (gGarage.numFramesCurr_GarageMove)
#define CS_GARAGE_ZOOM_IN       (gGarage.numFramesCurr_ZoomIn)
#define CS_GARAGE_ZOOM_OUT      (gGarage.numFramesCurr_ZoomOut)
#define CS_GARAGE_SELECTED      (gGarage.boolSelected)
#define CS_GARAGE_DELAY         (gGarage.delayOneSecond)
#define CS_GARAGE_CHARACTERS    (gGarage.garageCharacterIDs)
#define CS_GARAGE_STAT_LENGTHS  (gGarage.statBarLengths)
#define CS_GARAGE_UNUSED_FRAMES (gGarage.unusedFrameCount)
#define CS_GARAGE_CLASS_STRINGS (gGarage.classStringIDs)
#define CS_GARAGE_STAT_TARGETS  (gGarage.statBarTargetLengths)
#define CS_GARAGE_STAT_COLORS   (gGarage.statBarSegmentColors)
#define CS_GARAGE_MOVE_FRAMES   (gGarage.numFramesMax_GarageMove)
#define CS_GARAGE_FOV_MIN       (gGarage.fovMin)
#define CS_GARAGE_FOV_MAX       (gGarage.fovMax)
#endif

extern const Color csGarageWhite;

#ifndef CS_DECALHUD_ARROW_2D
#define CS_DECALHUD_ARROW_2D(icon, x, y, prim, ot, c0, c1, c2, c3, transparency, scale, rotation)                                                \
	DecalHUD_Arrow2D((icon), (x), (y), (prim), (ot), ColorCode_Load(&(c0)), ColorCode_Load(&(c1)), ColorCode_Load(&(c2)), ColorCode_Load(&(c3)), \
	                 (transparency), (scale), (rotation))
#endif

#endif

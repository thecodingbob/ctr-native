#ifndef CTR_TEST_CHAOS_FIXTURE_H
#define CTR_TEST_CHAOS_FIXTURE_H

// Shared asset-free world and integration doubles for the Chaos suites.
// Include the real modules in unity order so both suites exercise the same paths.
#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
NativeConfig g_config;
// VehBirth.c precedes Chaos in the game unity build; this fixture doubles birth.
enum { VEH_BIRTH_WHEEL_SIZE = 0xccc };
static struct GameTracker s_tracker;
static struct GamepadSystem s_gamepads;
static struct Driver s_defaultDrivers[2];
static bool s_allHumansFinished;
static int s_activationSounds;
static int s_clockSounds;
static int s_identitySounds;
static int s_squishSounds;
static int s_crateExplosions;
static int s_crateSounds;
static struct Instance *s_lastExplodingCrate;
static u32 s_lastSound;
static u32 s_lastSoundFlags;
static int s_noticeDraws;
static const char *s_noticeText;
static s16 s_noticeX;
static s16 s_noticeY;
static s16 s_noticeFont;
static u16 s_noticeFlags;
static u32 *s_noticeOT;
static const char *s_drawText[CHAOS_EFFECT_COUNT];
static s16 s_drawX[CHAOS_EFFECT_COUNT];
static s16 s_drawY[CHAOS_EFFECT_COUNT];
static s16 s_drawFont[CHAOS_EFFECT_COUNT];
static s32 s_drawScale[CHAOS_EFFECT_COUNT];
static u32 *s_drawOT[CHAOS_EFFECT_COUNT];
static int s_maskGrants;
static int s_shieldGrants;
static int s_itemSounds;
static u32 s_itemSound;
static int s_juicedSounds;
static u32 s_engineStarted[4];
static u32 s_engineStopped[4];
static int s_engineStarts[4];
static u8 s_chaosScratchpad[1024];
u8 *gCTRNativeScratchpadBase = s_chaosScratchpad;
static struct MaskHeadWeapon s_restartMasks[ADV_MULTIPLAYER_NUM_PLAYERS_MAX];
static int s_restartInitializations;
static int s_restartFallPoses;
static int s_startLightDraws;
static int s_startCountdownSounds;
static int s_startGreenSounds;
static u32 s_wrongWayDraws;
static int s_wrongWayVoices;
static int s_wrongWayVoiceID;
static bool s_voicePlayFails;
static struct Driver *s_voiceDriver;
static struct
{
	u32 header;
	struct Model model;
} s_characterModelFiles[GAME_CHARACTER_COUNT];
static int s_characterModelReads;
static struct Model *s_residentCharacterModels[GAME_CHARACTER_COUNT];

struct Model *VehBirth_GetModelByName(char *name)
{
	for (int character = 0; character < GAME_CHARACTER_COUNT; character++)
	{
		if (name == GAME_CHARACTER_METADATA[character].name_Debug)
		{
			return s_residentCharacterModels[character];
		}
	}
	return NULL;
}

void *LOAD_DramFile(void *bigfile, int fileIndex, void *destination, u32 *size, int callbackOrFlags)
{
	(void)bigfile;
	CTR_TEST_CHECK(destination == NULL && callbackOrFlags == -1, "character models are prepared with the synchronous relocatable loader");
	const int character = fileIndex - BI_RACERMODELHI;
	CTR_TEST_CHECK(character >= 0 && character < GAME_CHARACTER_COUNT, "preloading requests a playable character model");
	s_characterModelReads++;
	*size = sizeof(s_characterModelFiles[character]);
	return &s_characterModelFiles[character];
}

void UI_DrawWrongWay(const struct Driver *driver)
{
	s_wrongWayDraws |= 1u << driver->driverID;
}

b32 VehPickupItem_MaskBoolGoodGuy(struct Driver *driver)
{
	s_voiceDriver = driver;
	return driver->driverID == 0;
}

int CDSYS_XAPlay(int category, int voice)
{
	CTR_TEST_CHECK_EQ(category, CDSYS_XA_TYPE_EXTRA, "wrong-way voices use the normal mask speech bank");
	s_wrongWayVoiceID = voice;
	s_wrongWayVoices++;
	if (s_voicePlayFails)
	{
		return 0;
	}
	sdata->XA_State = XA_PLAYING;
	return 1;
}

int CDSYS_XAGetTrackLength(int category, int voice)
{
	(void)category; (void)voice;
	return 300;
}

void Bank_ResetAllocator(void) {}
void Audio_SetDefaults(void) {}
void Music_SetDefaults(void) {}
unsigned int SpuSetReverbVoice(int enabled, unsigned int voices)
{
	(void)enabled; (void)voices;
	return 0;
}
void SpuSetVoiceADSRAttr(int voice, unsigned short ar, unsigned short dr, unsigned short sr,
                       unsigned short rr, unsigned short sl, int arMode, int srMode, int rrMode)
{
	(void)voice; (void)ar; (void)dr; (void)sr; (void)rr; (void)sl;
	(void)arMode; (void)srMode; (void)rrMode;
}

void VehStuckProc_RevEngine_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	driver->kartState = KS_ENGINE_REVVING;
	driver->KartStates.RevEngine.boolMaskGrab = false;
	s_restartInitializations++;
}

struct MaskHeadWeapon *VehPickupItem_MaskCreateRescue(struct Driver *driver)
{
	return &s_restartMasks[driver->driverID];
}

void VehStuckProc_MaskGrab_SetFallPose(struct Driver *driver)
{
	(void)driver;
	s_restartFallPoses++;
}

u32 VehFrameInst_GetNumAnimFrames(struct Instance *instance, int animation)
{
	(void)instance; (void)animation;
	return 41;
}

int VehFrameInst_GetStartFrame(int animation, int frames)
{
	return animation == 0 ? frames / 2 : 0;
}

void DecalHUD_DrawPolyFT4(struct Icon *icon, s32 x, s32 y, struct PrimMem *memory, u32 *ot, u8 color, s16 scale)
{
	(void)icon; (void)x; (void)y; (void)memory; (void)ot; (void)color; (void)scale;
	s_startLightDraws++;
}

b32 EngineAudio_InitOnce(u32 soundID, u32 flags)
{
	CTR_TEST_CHECK_EQ(flags, HOWL_SFX_CENTER_NO_DISTORTION, "class changes use normal engine audio flags");
	s_engineStarted[soundID % 4] = soundID;
	s_engineStarts[soundID % 4]++;
	return true;
}

void EngineAudio_Stop(u32 soundID)
{
	s_engineStopped[soundID % 4] = soundID;
}

void VehStuckProc_RIP_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
	CtrTest_Bail("Wumpa crediting must not eliminate a driver");
}

void MainGameEnd_Initialize(void)
{
	CtrTest_Bail("Wumpa crediting must not end the race");
}

s32 MixRNG_Scramble(void)
{
	CtrTest_Bail("Chaos item selection must use its dedicated RNG");
	return 0;
}

b32 VehPickupItem_ApplyMaskMode(struct Driver *driver)
{
	(void)driver;
	return true;
}

struct MaskHeadWeapon *VehPickupItem_MaskUseWeapon(struct Driver *driver, b32 playSound)
{
	CTR_TEST_CHECK_EQ(driver->maskIsAku, -1, "direct mask activation ignores the held item's mask choice");
	CTR_TEST_CHECK(playSound, "direct mask activation requests the normal power-up sound");
	s_maskGrants++;
	return NULL;
}

void VehPickupItem_ShootNow(struct Driver *driver, s32 weapon, s32 flags)
{
	(void)driver;
	CTR_TEST_CHECK_EQ(flags, 0, "Chaos power-ups use the normal activation flags");
	if (weapon == HELD_ITEM_MASK)
	{
		s_maskGrants++;
	}
	else if (weapon == HELD_ITEM_SHIELD)
	{
		s_shieldGrants++;
	}
	else
	{
		CtrTest_Bail("Chaos direct grants must dispatch a mask or shield");
	}
}

static struct Driver *s_mailSource;
static struct Driver *s_mailTarget;
static int s_mailLaunches;

void VehPickupItem_ShootWarpball(struct Driver *driver, struct Driver *target)
{
	s_mailSource = driver;
	s_mailTarget = target;
	s_mailLaunches++;
}

// VehPickupItem.c precedes Chaos in the game unity build.
enum { SOUND_CLOCK = 0x44, SOUND_INVISIBILITY = 0x61 };

#include <game/LIST.c>
#include <game/HOWL/HOWL_Voiceline.c>
#include <game/DotLights.c>
#include <game/Vehicle/VehPhysCrash.c>
#include <game/Vehicle/VehBirth_Consts.c>
#include <game/Vehicle/VehCalc.c>
#include <game/Vehicle/VehPhysForce.c>
#include <game/Vehicle/VehPickState.c>
#include <game/Vehicle/VehPhysGeneral.c>
#include <game/Vehicle/VehPhysProc_Items.c>
// Keep the damage-path guard double while exercising real Wumpa crediting.
#define RB_Player_KillPlayer RB_Player_KillPlayer_Unused
#include <game/231/RB_Player.c>
#undef RB_Player_KillPlayer
static int s_launchConversions[8];
static Vec3 s_launchVelocity[8];
static bool s_convertLaunchSpeed;

// Ordinary lifecycle checks capture the boundary; overflow checks also commit it.
static void CaptureLaunchVelocity(struct Driver *driver, Vec3 *velocity)
{
	s_launchConversions[driver->driverID]++;
	s_launchVelocity[driver->driverID] = *velocity;
	if (s_convertLaunchSpeed)
	{
		VehPhysCrash_ConvertVecToSpeed(driver, velocity);
	}
}

#define VehPhysCrash_ConvertVecToSpeed CaptureLaunchVelocity
#include <game/231/RB_ChaosEffects.c>
#undef VehPhysCrash_ConvertVecToSpeed
#include <game/231/RB_Chaos.c>

static const char *s_externalCommand;
static int s_externalReads;
static int s_externalErrors;
static char s_externalCatalogue[NATIVE_CHAOS_CATALOGUE_SIZE];
static int s_cataloguePublications;

int NativeChaosIPC_ReadCommand(char *command, size_t capacity)
{
	s_externalReads++;
	if (s_externalCommand == NULL)
	{
		return NATIVE_CHAOS_NO_COMMAND;
	}
	snprintf(command, capacity, "%s", s_externalCommand);
	s_externalCommand = NULL;
	return NATIVE_CHAOS_COMMAND;
}

void NativeChaosIPC_WriteEffectsCatalogue(const char *catalogue)
{
	s_cataloguePublications++;
	snprintf(s_externalCatalogue, sizeof(s_externalCatalogue), "%s", catalogue);
}

void Platform_LogError(const char *format, ...)
{
	(void)format;
	s_externalErrors++;
}
#include <game/Vehicle/VehFire.c>
#include <game/Vehicle/VehAfterColl.c>
#include <game/UI/UI_Lerp2D.c>
#include <game/UI/UI_Chaos.c>
#include <game/MATH/MATH_0_Sin.c>

s32 DecalFont_GetLineWidth(char *text, s16 font)
{
	return (s32)strlen(text) * (font == FONT_SMALL ? 4 : 8);
}

void DecalFont_DrawLineOT(char *text, s16 x, s16 y, s16 font, s16 flags, u32 *ot)
{
	if (s_noticeDraws >= CHAOS_EFFECT_COUNT)
	{
		CtrTest_Bail("the Chaos HUD must draw at most one label per catalogue effect");
	}
	s_drawText[s_noticeDraws] = text;
	s_drawX[s_noticeDraws] = x;
	s_drawY[s_noticeDraws] = y;
	s_drawFont[s_noticeDraws] = font;
	s_drawOT[s_noticeDraws] = ot;
	s_drawScale[s_noticeDraws] = FP(1.0);
	s_noticeDraws++;
	s_noticeText = text;
	s_noticeX = x;
	s_noticeY = y;
	s_noticeFont = font;
	s_noticeFlags = (u16)flags;
	s_noticeOT = ot;
}

void DecalFont_DrawLineScaledOT(char *text, s16 x, s16 y, s16 font, s16 flags, s32 scale, u32 *ot)
{
	DecalFont_DrawLineOT(text, x, y, font, flags, ot);
	s_drawScale[s_noticeDraws - 1] = scale;
}

b32 MainGameEnd_AllHumansFinished(struct GameTracker *gGT)
{
	(void)gGT;
	return s_allHumansFinished;
}

void RB_Burst_InitVisual(struct Instance *inst)
{
	s_crateExplosions++;
	s_lastExplodingCrate = inst;
}
void PlaySound3D(u32 soundID, struct Instance *inst)
{
	CTR_TEST_CHECK_EQ(soundID, HOWL_SFX_TNT_EXPLOSION, "explosive crates use explosion audio");
	CTR_TEST_CHECK(inst == s_lastExplodingCrate, "explosion audio originates at the touched crate");
	s_crateSounds++;
}
void VehPhysProc_SpinFirst_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
}
void VehStuckProc_PlantEaten_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
	CtrTest_Bail("Clock Curse must select spin damage");
}
void VehStuckProc_Tumble_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
	CtrTest_Bail("Clock Curse must select spin damage");
}
void GAMEPAD_ShockFreq(struct Driver *driver, int frame, int value)
{
	(void)driver;
	(void)frame;
	(void)value;
}
void OtherFX_Play_Echo(u32 soundID, int flags, int echoFlag)
{
	(void)flags;
	(void)echoFlag;
	CTR_TEST_CHECK_EQ(soundID, VEH_PICK_SOUND_SQUISH, "squish damage uses the retail audio");
	s_squishSounds++;
}
void RB_Player_KillPlayer(struct Driver *attacker, struct Driver *victim)
{
	(void)attacker;
	(void)victim;
	CtrTest_Bail("Clock Curse must not attribute damage to an attacker");
}
void CTC2(u32 value, s32 reg)
{
	(void)value;
	(void)reg;
	CtrTest_Bail("clock damage without an attacker needs no projection");
}
void MTC2(u32 value, s32 reg)
{
	(void)value;
	(void)reg;
	CtrTest_Bail("clock damage without an attacker needs no projection");
}
s32 MFC2_S(s32 reg)
{
	(void)reg;
	CtrTest_Bail("clock damage without an attacker needs no projection");
	return 0;
}
s32 doCOP2(s32 op)
{
	(void)op;
	CtrTest_Bail("clock damage without an attacker needs no projection");
	return 0;
}
int OtherFX_Play(u32 soundID, int flags)
{
	if (soundID == SOUND_INVISIBILITY)
	{
		CTR_TEST_CHECK_EQ(flags, 1, "Identity Crisis uses the normal invisibility sound flags");
		s_identitySounds++;
		return 0;
	}
	if (soundID == DOT_LIGHT_FX_COUNTDOWN || soundID == DOT_LIGHT_FX_GREEN)
	{
		s_startCountdownSounds += soundID == DOT_LIGHT_FX_COUNTDOWN;
		s_startGreenSounds += soundID == DOT_LIGHT_FX_GREEN;
		return 0;
	}
	if (soundID == 0x5e || soundID == 0x41)
	{
		CTR_TEST_CHECK_EQ(flags, soundID == 0x41, "item acquisition retains the normal Wumpa-dependent sound flags");
		s_itemSounds++;
		s_itemSound = soundID;
		if (soundID == 0x41)
		{
			s_juicedSounds++;
		}
		return 0;
	}
	CTR_TEST_CHECK_EQ(soundID, SOUND_CLOCK, "Clock Curse plays the retail clock sound");
	s_clockSounds++;
	return 0;
}
int OtherFX_Play_LowLevel(u32 soundID, u8 antiSpam, u32 flags)
{
	(void)antiSpam;
	s_activationSounds++;
	s_lastSound = soundID;
	s_lastSoundFlags = flags;
	return 0;
}

void howl_StopAudio(b32 backup, b32 music, b32 fx)
{
	(void)backup;
	(void)music;
	(void)fx;
}

void GhostTape_WriteBoosts(int addReserve, u32 type, int speedCap)
{
	(void)addReserve;
	(void)type;
	(void)speedCap;
}

void GAMEPAD_ShockForce1(struct Driver *driver, int frame, int value)
{
	(void)driver;
	(void)frame;
	(void)value;
}

struct Instance *INSTANCE_BirthWithThread(s32 modelID, const char *name, s32 poolType,
                                        s32 bucket, void *tick, s32 objSize, struct Thread *parent)
{
	(void)modelID;
	(void)name;
	(void)poolType;
	(void)bucket;
	(void)tick;
	(void)objSize;
	(void)parent;
	return NULL;
}

struct Instance *INSTANCE_Birth3D(struct Model *model, const char *name, struct Thread *thread)
{
	(void)model;
	(void)name;
	(void)thread;
	return NULL;
}

void VehTurbo_ThTick(struct Thread *thread)
{
	(void)thread;
}

void VehTurbo_ThDestroy(struct Thread *thread)
{
	(void)thread;
}

// These full-physics paths are outside the asset-free scale/contact checks.
s32 MATH_FastSqrt(u32 value, s32 shift)
{
	(void)value; (void)shift;
	CtrTest_Bail("scale/contact checks must not search collision buckets");
	return 0;
}
int ratan2(int y, int x)
{
	return (int)lround(atan2((double)y, (double)x) * ANG_HALF_PI / atan2(1.0, 0.0));
}
void ConvertRotToMatrix(MATRIX *matrix, const SVec3 *rotation)
{
	(void)matrix; (void)rotation;
	CtrTest_Bail("scale/contact checks must not animate matrices");
}
void MatrixRotate(void *dst, MATRIX *src, MATRIX *rotation)
{
	(void)dst; (void)src; (void)rotation;
	CtrTest_Bail("scale/contact checks must not animate matrices");
}
void OtherFX_DriverCrashing(u32 echo, u32 volume)
{
	(void)echo; (void)volume;
	CtrTest_Bail("squash checks must not use the ordinary crash sound");
}
void BOTS_CollideWithOtherAI(struct Driver *first, struct Driver *second)
{
	(void)first; (void)second;
	CtrTest_Bail("squash checks must not run bot navigation");
}
void GAMEPAD_JogCon1(struct Driver *driver, u8 value, u16 timeMS)
{
	(void)driver; (void)value; (void)timeMS;
	CtrTest_Bail("scale/contact checks must not run ordinary crash rumble");
}
void PROC_CollidePointWithBucket(struct Thread *thread, struct BucketSearchParams *search)
{
	(void)thread; (void)search;
	CtrTest_Bail("scale/contact checks must not search collision buckets");
}
u32 MFC2(int reg)
{
	(void)reg;
	CtrTest_Bail("scale/contact checks must not project geometry");
	return 0;
}
struct Particle *Particle_Init(u32 flags, struct IconGroup *icons, struct ParticleEmitter *emitter)
{
	(void)flags; (void)icons; (void)emitter;
	CtrTest_Bail("scale/contact checks must not spawn particles");
	return NULL;
}

static void SetupRace(int trigger)
{
	memset(data.metaPhys, 0, sizeof(data.metaPhys));
	memset(data.MetaDataCharacters, 0, sizeof(data.MetaDataCharacters));
	memset(data.characterIDs, 0, sizeof(data.characterIDs));
	// Stand-in retail storage: repeated entries must not weight uniform gifts.
	memset(data.RNG_itemSetRace2, HELD_ITEM_TURBO, sizeof(data.RNG_itemSetRace2));
	int poolIndex = 0;
	for (int item = HELD_ITEM_TURBO; item <= HELD_ITEM_MISSILE_3X; item++)
	{
		if (item != HELD_ITEM_SPRING)
		{
			data.RNG_itemSetRace2[poolIndex++] = (u8)item;
		}
	}
	s_allHumansFinished = false;
	s_activationSounds = 0;
	s_clockSounds = 0;
	s_identitySounds = 0;
	s_squishSounds = 0;
	s_crateExplosions = 0;
	s_crateSounds = 0;
	s_wrongWayDraws = 0;
	s_wrongWayVoices = 0;
	s_wrongWayVoiceID = 0;
	s_voicePlayFails = false;
	s_voiceDriver = NULL;
	s_characterModelReads = 0;
	memset(s_residentCharacterModels, 0, sizeof(s_residentCharacterModels));
	memset(s_engineStarted, 0, sizeof(s_engineStarted));
	memset(s_engineStopped, 0, sizeof(s_engineStopped));
	memset(s_engineStarts, 0, sizeof(s_engineStarts));
	memset(s_launchConversions, 0, sizeof(s_launchConversions));
	memset(s_launchVelocity, 0, sizeof(s_launchVelocity));
	s_convertLaunchSpeed = false;
	s_lastExplodingCrate = NULL;
	memset(&sdata_static, 0, sizeof(sdata_static));
	memset(&s_tracker, 0, sizeof(s_tracker));
	memset(&s_gamepads, 0, sizeof(s_gamepads));
	memset(s_defaultDrivers, 0, sizeof(s_defaultDrivers));
	memset(&g_config, 0, sizeof(g_config));
	sdata->gGT = &s_tracker;
	sdata->gGamepads = &s_gamepads;
	sdata->Loading.stage = LOAD_IDLE;
	s_tracker.gameMode1 = ADVENTURE_MODE;
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	s_tracker.numPlyrCurrGame = 2;
	for (int i = 0; i < s_tracker.numPlyrCurrGame; i++)
	{
		s_defaultDrivers[i].driverID = i;
		s_tracker.drivers[i] = &s_defaultDrivers[i];
		s_gamepads.gamepad[i].stickLX = CHAOS_STICK_CENTER;
		s_gamepads.gamepad[i].stickLY = CHAOS_STICK_CENTER;
		s_gamepads.gamepad[i].stickRY = CHAOS_STICK_CENTER;
	}
	sdata->chaosSettings = (struct ChaosSettings){
		.armed = true, .trigger = (u8)trigger, .durationSeconds = 30,
		.triggerIntervalSeconds = 5, .enabledCount = 1,
	};
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	Chaos_BeginRace(&s_tracker);
}

static void Tick(int elapsedMS)
{
	s_tracker.elapsedTimeMS = elapsedMS;
	Chaos_Update(&s_tracker);
}

static void EnableInputEffects(void)
{
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	memset(sdata->chaosSettings.effects, 0, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.effects[CHAOS_EFFECT_INVERTED_STEERING] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_NO_HOPPING] = true;
	sdata->chaosSettings.enabledCount = 2;
}

static void DrawNotice(void)
{
	s_noticeDraws = 0;
	UI_Chaos_DrawActivation(&s_tracker);
}

#endif

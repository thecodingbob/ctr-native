#ifndef CTR_MATCHING_NAMESPACE_VEHICLE_RETAIL_SYMBOLS_H
#define CTR_MATCHING_NAMESPACE_VEHICLE_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

// NOTE(aalhendi): Share address expressions between C bindings and scheduled assembly.
#define VEH_BAKED_GTE_MATH_ASM_NAME      "data+30036"
#define VEH_TRIG_APPROX_ASM_NAME         "data+15360"
#define VEH_LAST_FEEDBACK_FRAME_ASM_NAME "sdata_static+2252"

// NOTE(aalhendi): Retail Vehicle code addresses the game-tracker pointer as
// an individual symbol even though the native layout groups it in sData.
extern DriverModelExtraSlot veh_driverModelExtras[LOAD_DRIVER_MODEL_EXTRA_COUNT] asm("data+12400");
extern struct MetaPhys veh_metaPhys[65] asm("data+32876");
extern CtrPackedU32 veh_bakedGteMathWords[BAKED_GTE_MATRIX_COUNT * 2] asm(VEH_BAKED_GTE_MATH_ASM_NAME);
extern CtrPackedU32 veh_trigApprox[0x400] asm(VEH_TRIG_APPROX_ASM_NAME);
extern struct RngDeadCoedState veh_advRng asm("sdata_static+1788");
extern u8 veh_kartSpawnOrder[8] asm("sdata_static+1840");
extern char veh_itemSetRace1[0x14] asm("data+37324");
extern char veh_itemSetRace2[0x34] asm("data+37344");
extern char veh_itemSetRace3[0x14] asm("data+37396");
extern char veh_itemSetRace4[0x14] asm("data+37416");
extern char veh_itemSetBossRace[0x14] asm("data+37436");
extern char veh_itemSetBattleDefault[0x14] asm("data+37456");
extern struct Model *veh_modelMaskHints3D asm("sdata_static+388");
extern s32 veh_XAMaxSampleValInArr asm("sdata_static+1928");
extern s32 veh_xaState asm("sdata_static+1948");
extern u8 veh_maskThreadAlive __attribute__((section(".sdata")));
extern u8 veh_talkMaskDead __attribute__((section(".sdata")));
extern char veh_talkMaskHead[] asm("sdata_static+1780");
extern char veh_playerThreadName[] asm("sdata_static+1704");
extern char veh_turbo1Name[] asm("sdata_static+1712");
extern char veh_turbo2Name[] asm("sdata_static+1720");
extern struct Model **veh_playerObjectList asm("sdata_static+2308");
extern SVec3 veh_botCrashNavRotArg asm("veh_botCrashNavRotArg") __attribute__((section(".data")));
extern Vec3 veh_physCrashForward asm("veh_physCrashForward");
extern MATRIX veh_physCrashMatrix asm("veh_physCrashMatrix");
extern char veh_pickupDoctorName[] asm("sdata_static+1728");
extern char veh_pickupBombName[] asm("sdata_static+1736");
extern char veh_pickupNitroName[] asm("sdata_static+1752");
extern char veh_pickupTntName[] asm("sdata_static+1760");
extern char veh_pickupBeakerName[] asm("sdata_static+1768");
extern char veh_pickupShieldName[] asm("sdata_static+1744");
extern char veh_pickupShieldDarkName[] asm("rdata+6524");
extern char veh_pickupHighlightName[] asm("rdata+6536");
extern void *const veh_pickupShootNowJumpTable[] asm("rdata+6648");

// NOTE(aalhendi): These aliases preserve retail call sites where a value was
// already staged in an argument register or the fifth stack-argument slot.
extern void veh_convertVecToSpeedThreeArgs(struct Driver *driver, Vec3 *velocity, int unusedThirdArg) asm("VehPhysCrash_ConvertVecToSpeed");
extern int veh_mapToRangeFourArgs(int value, int inputMin, int inputMax, int outputMin) asm("VehCalc_MapToRange");
extern void veh_emitterJogCon1() asm("GAMEPAD_JogCon1");
extern void veh_emitterJogCon2() asm("GAMEPAD_JogCon2");

// NOTE(aalhendi): Bind shared data views to their retail resident symbols.
#define VEH_TUMBLE_INIT_PAGE                                                           0x80070000
#define VEH_TUMBLE_INIT_FROM_PAGE(page)                                                ((DriverFunc)((u32)(page) - 32092))
#define VEH_LOAD_CHARACTER_IDS_PAGE(page)                                              CTR_PSX_LOAD_SYMBOL_PAGE((page), RETAIL_CHARACTER_IDS_ASM_NAME)
#define VEH_ADD_CHARACTER_IDS_LOW(result, page) CTR_PSX_ADD_SYMBOL_LOW((result), (page), RETAIL_CHARACTER_IDS_ASM_NAME, GAME_CHARACTER_IDS)
#define VEH_DRIVER_MODEL_EXTRAS                                                        veh_driverModelExtras
#define VEH_META_PHYS                                                                  veh_metaPhys
#define VEH_BAKED_GTE_PHYS_ENTRY(index)                                                ((void *)(u32)veh_bakedGteMathWords[(index) * 2])
#define VEH_BAKED_GTE_NUM_ENTRIES(index)                                               ((int)veh_bakedGteMathWords[((index) * 2) + 1])
#define VEH_TRIG_APPROX(index)                                                         veh_trigApprox[(index)]
#define VEH_ADV_RNG                                                                    veh_advRng
#define VEH_KART_SPAWN_ORDER                                                           veh_kartSpawnOrder
#define VEH_ITEM_SET_RACE1                                                             veh_itemSetRace1
#define VEH_ITEM_SET_RACE2                                                             veh_itemSetRace2
#define VEH_ITEM_SET_RACE3                                                             veh_itemSetRace3
#define VEH_ITEM_SET_RACE4                                                             veh_itemSetRace4
#define VEH_ITEM_SET_BOSS_RACE                                                         veh_itemSetBossRace
#define VEH_ITEM_SET_BATTLE_DEFAULT                                                    veh_itemSetBattleDefault
#define VEH_MODEL_MASK_HINTS_3D                                                        veh_modelMaskHints3D
#define VEH_XA_MAX_SAMPLE_VALUE                                                        veh_XAMaxSampleValInArr
#define VEH_XA_STATE                                                                   veh_xaState
#define VEH_MASK_THREAD_ALIVE                                                          veh_maskThreadAlive
#define VEH_TALK_MASK_DEAD                                                             veh_talkMaskDead
#define VEH_TALK_MASK_HEAD                                                             veh_talkMaskHead
#define VEH_PLAYER_THREAD_NAME                                                         veh_playerThreadName
#define VEH_TURBO1_NAME                                                                veh_turbo1Name
#define VEH_TURBO2_NAME                                                                veh_turbo2Name
#define VEH_PLAYER_OBJECT_LIST                                                         veh_playerObjectList
#define VEH_BOT_CRASH_NAV_ROT_ARG                                                      veh_botCrashNavRotArg
#define VEH_PHYS_CRASH_FORWARD                                                         veh_physCrashForward
#define VEH_PHYS_CRASH_MATRIX                                                          veh_physCrashMatrix

// NOTE(aalhendi): Retail pickup dispatch reads the original resident jump table.
#define VEH_PICKUP_DOCTOR_NAME                                                         veh_pickupDoctorName
#define VEH_PICKUP_BOMB_NAME                                                           veh_pickupBombName
#define VEH_PICKUP_NITRO_NAME                                                          veh_pickupNitroName
#define VEH_PICKUP_TNT_NAME                                                            veh_pickupTntName
#define VEH_PICKUP_BEAKER_NAME                                                         veh_pickupBeakerName
#define VEH_PICKUP_SHIELD_NAME                                                         veh_pickupShieldName
#define VEH_PICKUP_SHIELD_DARK_NAME                                                    veh_pickupShieldDarkName
#define VEH_PICKUP_HIGHLIGHT_NAME                                                      veh_pickupHighlightName
#define VEH_PICKUP_SHOOT_NOW_DISPATCH(weaponId)                                        goto *veh_pickupShootNowJumpTable[(weaponId)]

// NOTE(aalhendi): Preserve call shapes with arguments prepared outside the C expression.
#define VEH_CONVERT_VEC_TO_SPEED(driver, velocity)                                     veh_convertVecToSpeedThreeArgs((driver), (velocity), 0)
#define VEH_MAP_TO_RANGE_STAGED_FIFTH(value, inputMin, inputMax, outputMin, outputMax) veh_mapToRangeFourArgs((value), (inputMin), (inputMax), (outputMin))

// NOTE(aalhendi): The first jog call inherits the current driver in a0.
#define VEH_EMITTER_JOG_CON2                                                           veh_emitterJogCon2
#define VEH_EMITTER_JOG_CON1_CURRENT_DRIVER(driver, value, duration) \
	do                                                               \
	{                                                                \
		register struct Driver *driverArg __asm__("$4");             \
		__asm__("" : "=r"(driverArg));                               \
		veh_emitterJogCon1(driverArg, (value), (duration));          \
	} while (0)

// NOTE(aalhendi): Preserve retail's absolute page/offset construction.
#define VEH_LOAD_META_PHYS_BASE(result, page)                                  \
	do                                                                         \
	{                                                                          \
		CTR_PSX_LOAD_SYMBOL_PAGE((page), "data+32876");                        \
		CTR_PSX_ADD_SYMBOL_LOW((result), (page), "data+32876", VEH_META_PHYS); \
	} while (0)
#define VEH_LOAD_GAME_TRACKER(result)                                                                \
	do                                                                                               \
	{                                                                                                \
		CTR_PSX_LOAD_SYMBOL_PAGE((result), RETAIL_GAME_TRACKER_ASM_NAME);                            \
		CTR_PSX_LOAD_WORD_FROM_PAGE((result), (result), RETAIL_GAME_TRACKER_ASM_NAME, GAME_TRACKER); \
	} while (0)

#endif

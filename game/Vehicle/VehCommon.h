#ifndef VEH_COMMON_H
#define VEH_COMMON_H

#include <common.h>
#include <ctr_gte_transfer.h>

// NOTE(aalhendi): These defaults use the shared runtime layout. The matching
// pipeline force-includes a private header that overrides only retail bindings
// and call shapes that GCC 2.8.1 must see differently.

#ifndef VEH_LOAD_GAME_TRACKER
#define VEH_LOAD_GAME_TRACKER(result) ((result) = GAME_TRACKER)
#endif

#ifndef VEH_TUMBLE_INIT_PAGE
#define VEH_TUMBLE_INIT_PAGE            0
#define VEH_TUMBLE_INIT_FROM_PAGE(page) ((void)sizeof(page), VehStuckProc_Tumble_Init)
#endif

#ifndef VEH_LOAD_CHARACTER_IDS_PAGE
#define VEH_LOAD_CHARACTER_IDS_PAGE(page) ((void)sizeof(page))
#endif

#ifndef VEH_ADD_CHARACTER_IDS_LOW
#define VEH_ADD_CHARACTER_IDS_LOW(result, page) \
	do                                          \
	{                                           \
		(void)sizeof(page);                     \
		(result) = GAME_CHARACTER_IDS;          \
	} while (0)
#endif


#ifndef VEH_XA_STATE
#define VEH_XA_STATE (sdata->XA_State)
#endif

#ifndef VEH_MASK_THREAD_ALIVE
#define VEH_MASK_THREAD_ALIVE (sdata->boolIsMaskThreadAlive)
#endif

#ifndef VEH_TALK_MASK_DEAD
#define VEH_TALK_MASK_DEAD (sdata->talkMask_boolDead)
#endif

#ifndef VEH_PLAYER_THREAD_NAME
#define VEH_PLAYER_THREAD_NAME (sdata->s_player)
#endif

#ifndef VEH_TURBO1_NAME
#define VEH_TURBO1_NAME (sdata->s_turbo1)
#endif

#ifndef VEH_TURBO2_NAME
#define VEH_TURBO2_NAME (sdata->s_turbo2)
#endif

#ifndef VEH_DRIVER_MODEL_EXTRAS
#define VEH_DRIVER_MODEL_EXTRAS (data.driverModelExtras)
#endif

#ifndef VEH_PLAYER_OBJECT_LIST
#define VEH_PLAYER_OBJECT_LIST ((struct Model **)sdata->PLYROBJECTLIST)
#endif

#ifndef VEH_META_PHYS
#define VEH_META_PHYS (data.metaPhys)
#endif

#ifndef VEH_BAKED_GTE_PHYS_ENTRY
#define VEH_BAKED_GTE_PHYS_ENTRY(index) (data.bakedGteMath[(index)].physEntry)
#endif

#ifndef VEH_BAKED_GTE_NUM_ENTRIES
#define VEH_BAKED_GTE_NUM_ENTRIES(index) (data.bakedGteMath[(index)].numEntries)
#endif

#ifndef VEH_TRIG_APPROX
#define VEH_TRIG_APPROX(index) CTR_ReadU32AlignedLE(&data.trigApprox[(index)])
#endif

#ifndef VEH_ADV_RNG
#define VEH_ADV_RNG (sdata->advRng)
#endif


#ifndef VEH_KART_SPAWN_ORDER
#define VEH_KART_SPAWN_ORDER (sdata->kartSpawnOrderArray)
#endif

#ifndef VEH_ITEM_SET_RACE1
#define VEH_ITEM_SET_RACE1          (data.RNG_itemSetRace1)
#define VEH_ITEM_SET_RACE2          (data.RNG_itemSetRace2)
#define VEH_ITEM_SET_RACE3          (data.RNG_itemSetRace3)
#define VEH_ITEM_SET_RACE4          (data.RNG_itemSetRace4)
#define VEH_ITEM_SET_BOSS_RACE      (data.RNG_itemSetBossrace)
#define VEH_ITEM_SET_BATTLE_DEFAULT (data.RNG_itemSetBattleDefault)
#endif

#ifndef VEH_MODEL_MASK_HINTS_3D
#define VEH_MODEL_MASK_HINTS_3D (sdata->modelMaskHints3D)
#endif

#ifndef VEH_XA_MAX_SAMPLE_VALUE
#define VEH_XA_MAX_SAMPLE_VALUE (sdata->XA_MaxSampleValInArr)
#endif

#ifndef VEH_BOT_CRASH_NAV_ROT_ARG
#define VEH_BOT_CRASH_NAV_ROT_ARG (sdata->botCrashNavRot)
#endif

#ifndef VEH_PHYS_CRASH_FORWARD
#define VEH_PHYS_CRASH_FORWARD (((struct VehPhysCrashAiScratch *)(void *)&sdata->dataLibFiller[0])->forward)
#endif

#ifndef VEH_PHYS_CRASH_MATRIX
#define VEH_PHYS_CRASH_MATRIX (((struct VehPhysCrashAiScratch *)(void *)&sdata->dataLibFiller[0])->matrix)
#endif

#ifndef VEH_PICKUP_DOCTOR_NAME
#define VEH_PICKUP_DOCTOR_NAME      (sdata->s_doctor1)
#define VEH_PICKUP_BOMB_NAME        (sdata->s_bomb1)
#define VEH_PICKUP_NITRO_NAME       (sdata->s_nitro1)
#define VEH_PICKUP_TNT_NAME         (sdata->s_tnt1)
#define VEH_PICKUP_BEAKER_NAME      (sdata->s_beaker1)
#define VEH_PICKUP_SHIELD_NAME      (sdata->s_shield)
#define VEH_PICKUP_SHIELD_DARK_NAME rdata.s_shielddark
#define VEH_PICKUP_HIGHLIGHT_NAME   rdata.s_highlight
#endif

#ifndef VEH_PICKUP_SHOOT_NOW_DISPATCH
#define VEH_PICKUP_SHOOT_NOW_DISPATCH(weaponId) ((void)(weaponId))
#endif

#ifndef VEH_CONVERT_VEC_TO_SPEED
#define VEH_CONVERT_VEC_TO_SPEED(driver, velocity) VehPhysCrash_ConvertVecToSpeed((driver), (velocity))
#endif

#ifndef VEH_MAP_TO_RANGE_STAGED_FIFTH
#define VEH_MAP_TO_RANGE_STAGED_FIFTH(value, inputMin, inputMax, outputMin, outputMax) \
	VehCalc_MapToRange((value), (inputMin), (inputMax), (outputMin), (outputMax))
#endif

#ifndef VEH_EMITTER_JOG_CON2
#define VEH_EMITTER_JOG_CON2 GAMEPAD_JogCon2
#endif

#ifndef VEH_EMITTER_JOG_CON1_CURRENT_DRIVER
#define VEH_EMITTER_JOG_CON1_CURRENT_DRIVER(driver, value, duration) GAMEPAD_JogCon1((driver), (value), (duration))
#endif

#ifndef VEH_LOAD_META_PHYS_BASE
#define VEH_LOAD_META_PHYS_BASE(result, page) \
	do                                        \
	{                                         \
		(void)sizeof(page);                   \
		(result) = VEH_META_PHYS;             \
	} while (0)
#endif

// NOTE(aalhendi): These are the shared native/retail forms of the PsyQ
// three-vector projection adapters used by the Vehicle render helpers.
#if defined(CTR_NATIVE)
#define VehGteLoadV0(vector)    CTR_GteLoadSV0(vector)
#define VehGteLoadV3(vector)    CTR_GteLoadSV3(&(vector)[0], &(vector)[1], &(vector)[2])
#define VehGteStoreSxy3(output) CTR_GteStoreSXY3(&(output)[0], &(output)[1], &(output)[2])
#define VehGteStoreSz3(output)  gte_stsz3(&(output)[0], &(output)[1], &(output)[2])
#else
#define VehGteLoadV0(vector) __asm__ volatile("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(vector), "m"(*(const SVECTOR *)(vector)))
#define VehGteLoadV3(vector)                                                                                                 \
	__asm__ volatile("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)\n\tlwc2 $2,8(%0)\n\tlwc2 $3,12(%0)\n\tlwc2 $4,16(%0)\n\tlwc2 $5,20(%0)" \
	                 :                                                                                                       \
	                 : "r"(vector), "m"(((const SVECTOR *)(vector))[0]), "m"(((const SVECTOR *)(vector))[1]), "m"(((const SVECTOR *)(vector))[2]))
#define VehGteStoreSxy3(output) __asm__ volatile("swc2 $12,0(%0)\n\tswc2 $13,4(%0)\n\tswc2 $14,8(%0)" : : "r"(output) : "memory")
#define VehGteStoreSz3(output)  __asm__ volatile("swc2 $17,0(%0)\n\tswc2 $18,4(%0)\n\tswc2 $19,8(%0)" : : "r"(output) : "memory")
#endif

static inline void VehGteSetRotTransMatrix(const MATRIX *matrix)
{
	const CtrPackedU32 *matrixWords = (const CtrPackedU32 *)matrix;
	register u32 matrixWord0 CTR_PSX_REGISTER("$12");
	register u32 matrixWord1 CTR_PSX_REGISTER("$13");
	register u32 matrixWord2 CTR_PSX_REGISTER("$14");

	// NOTE(aalhendi): Keeping one shared base reproduces the retail SDK load schedule.
	CTR_PSX_KEEP_VALUE(matrixWords);
	matrixWord0 = matrixWords[0];
	matrixWord1 = matrixWords[1];
	CTC2(matrixWord0, 0);
	CTC2(matrixWord1, 1);
	matrixWord0 = matrixWords[2];
	matrixWord1 = matrixWords[3];
	matrixWord2 = matrixWords[4];
	CTC2(matrixWord0, 2);
	CTC2(matrixWord1, 3);
	CTC2(matrixWord2, 4);
	matrixWord0 = matrixWords[5];
	matrixWord1 = matrixWords[6];
	CTC2(matrixWord0, 5);
	matrixWord2 = matrixWords[7];
	CTC2(matrixWord1, 6);
	CTC2(matrixWord2, 7);
}

static inline void VehGteSetColorMatrix(const MATRIX *matrix)
{
	const CtrPackedU32 *matrixWords = (const CtrPackedU32 *)matrix;
	register u32 matrixWord0 CTR_PSX_REGISTER("$12");
	register u32 matrixWord1 CTR_PSX_REGISTER("$13");
	register u32 matrixWord2 CTR_PSX_REGISTER("$14");

	CTR_PSX_KEEP_VALUE(matrixWords);
	matrixWord0 = matrixWords[0];
	matrixWord1 = matrixWords[1];
	CTC2(matrixWord0, 16);
	CTC2(matrixWord1, 17);
	matrixWord0 = matrixWords[2];
	matrixWord1 = matrixWords[3];
	matrixWord2 = matrixWords[4];
	CTC2(matrixWord0, 18);
	CTC2(matrixWord1, 19);
	CTC2(matrixWord2, 20);
}


#endif

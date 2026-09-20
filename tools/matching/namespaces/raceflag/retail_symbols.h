#define RACE_FLAG_LOADING_STAGE raceFlagLoadingStage
#define RACE_FLAG_CAN_DRAW      raceFlagCanDraw
#define RACE_FLAG_MATRIX        raceFlagMatrix
#define RACE_FLAG_WAVE          raceFlagWave
#define RACE_FLAG_TRIG          raceFlagTrig

#include "../../retail_bindings.h"

extern s32 raceFlagLoadingStage asm("sdata_static+396");
extern b16 raceFlagCanDraw;
extern MATRIX raceFlagMatrix asm("data+20768");
extern s32 raceFlagWave[5] asm("data+20800");
extern struct TrigTable raceFlagTrig[0x400] asm("data+15360");

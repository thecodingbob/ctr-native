#ifndef CTR_MATCHING_OVERLAY_231_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_231_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

// NOTE(aalhendi): Racing-overlay state uses absolute resident addresses.
#define sdata              (&sdata_static)

#define RB_MINE_POOL_TAKEN rb_minePoolTaken
#define RB_MINE_POOL_FREE  rb_minePoolFree
#define RB_MINE_POOL_ITEMS rb_minePoolItems

extern struct LinkedList rb_minePoolTaken asm("D231+0");
extern struct LinkedList rb_minePoolFree asm("D231+12");
extern struct WeaponSlot231 rb_minePoolItems[50] asm("D231+24");

// NOTE(aalhendi): Retail addresses the fade and particle height curves independently of the scale table.
#define RB_WARPBALL_FADE_Y rb_warpballFadeY
extern s32 rb_warpballFadeY[6] asm("R231+456");

#define RB_WARPBALL_PARTICLE_HEIGHT rb_warpballParticleHeight
extern u16 rb_warpballParticleHeight[11] asm("R231+396");

#define RB_SHIELD_POP_SCALE rb_shieldPopScale
extern s16 rb_shieldPopScale[11][2] asm("R231+560");

#define RB_MASK_POSITION rb_maskPosition
extern s16 rb_maskPosition[24] asm("R231+480");

#define RB_SHIELD_GROW_SCALE rb_shieldGrowScale
extern s16 rb_shieldGrowScale[8][2] asm("R231+528");

#define RB_SHIELD_PULSE_SCALE rb_shieldPulseScale
extern s16 rb_shieldPulseScale[6][2] asm("R231+604");

#endif

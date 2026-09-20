#ifndef CTR_MATCHING_OVERLAY_221_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_221_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"
// NOTE(aalhendi): These aliases name resident EXE state; no storage is allocated.
extern struct Instance *cc_hudCrystal asm("sdata_static+2640");
extern struct Instance *cc_menuCrystal asm("sdata_static+2644");

#define CC_HUD_CRYSTAL             cc_hudCrystal
#define CC_MENU_CRYSTAL            cc_menuCrystal

#endif

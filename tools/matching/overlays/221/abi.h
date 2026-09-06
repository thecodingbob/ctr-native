#ifndef CTR_MATCHING_OVERLAY_221_ABI_H
#define CTR_MATCHING_OVERLAY_221_ABI_H

#include <common.h>

// NOTE(aalhendi): GCC 2.8.1 classifies extern objects of at most eight bytes as
// small data under -G8. Three-element declarations keep these retail ABI
// aliases in absolute addressing; the linker binds each symbol to its retail
// address.
extern struct GameTracker *cc_gameTracker[3];
extern s32 cc_framesSinceRaceEnded[3];
extern struct Instance *cc_hudCrystal[3];
extern struct Instance *cc_menuCrystal[3];
extern struct Instance *cc_token[3];
extern char **cc_languageStrings[3];
extern s32 cc_menuReady[3];
extern s32 cc_anyPlayerTap[3];
extern struct AdvProgress cc_advProgress;
extern u32 cc_addConfig0[3];
extern u32 cc_removeConfig0[3];

#define CC_GAME_TRACKER            cc_gameTracker[0]
// NOTE(aalhendi): Volatile preserves retail's separate reads for the driver and
// comparison paths.
#define CC_READ_GAME_TRACKER()     (*(struct GameTracker *volatile *)&cc_gameTracker[0])
#define CC_FRAMES_SINCE_RACE_ENDED cc_framesSinceRaceEnded[0]
#define CC_HUD_CRYSTAL             cc_hudCrystal[0]
#define CC_MENU_CRYSTAL            cc_menuCrystal[0]
#define CC_TOKEN                   cc_token[0]
#define CC_LANGUAGE_STRINGS        cc_languageStrings[0]
#define CC_MENU_READY              cc_menuReady[0]
#define CC_ANY_PLAYER_TAP          cc_anyPlayerTap[0]
#define CC_ADV_PROGRESS            cc_advProgress
#define CC_ADD_CONFIG_0            cc_addConfig0[0]
#define CC_REMOVE_CONFIG_0         cc_removeConfig0[0]

#endif

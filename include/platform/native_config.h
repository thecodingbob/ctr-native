#include <stdbool.h>

#ifndef NATIVE_CONFIG_H
#define NATIVE_CONFIG_H

typedef struct {
    bool skipIntro;             // false = off (default), true = skip all intros
    bool skipHints;             // false = off (default), true = skip all mask hints in adventure mode
    int speedMultiplier;        // percent, 10%..200%, 100 = 1.0x (default)
    int gravityMultiplier;      // percent, 10%..300%, 100 = 1.0x (default)
    bool unlockAllCharacters;   // false = off (normal unlock logic), true = all characters already unlocked
    bool unlockAllGates;        // false = off, true = all adventure mode wood doors open without keys
    bool unlockAllPortals;      // false = off, true = all warp pads and boss garages unlocked
} NativeConfig;

extern NativeConfig g_config;

extern int numConfigOptions;
extern int numConfigSections;

void NativeConfig_Load(void);
void NativeConfig_Save(void);

#endif
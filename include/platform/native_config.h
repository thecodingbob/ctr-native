#ifndef NATIVE_CONFIG_H
#define NATIVE_CONFIG_H

#include <stdbool.h>

typedef struct {
    const char *name;
    int value;
} ConfigEnumValue;

typedef struct {
    bool skipIntro;             // false = off (default), true = skip all intros
    bool skipHints;             // false = off (default), true = skip all mask hints in adventure mode
    int speedMultiplier;        // percent, 10%..200%, 100 = 1.0x (default)
    int gravityMultiplier;      // percent, 10%..300%, 100 = 1.0x (default)
    int turnMultiplier;         // percent, 10%..400%, 100 = 1.0x (default)
    int jumpMultiplier;         // percent, 10%..300%, 100 = 1.0x (default)
    bool unlockAllCharacters;   // false = off (normal unlock logic), true = all characters already unlocked
    bool unlockAllGates;        // false = off, true = all adventure mode wood doors open without keys
    bool unlockAllPortals;      // false = off, true = all warp pads and boss garages unlocked
    bool increaseDrawDistance;  // false = off (default), true = render 3x farther
    bool disableSplitScreenLod; // false = off (default), true = load hi-res character models in 3+ multiplayer
    int aspectRatio;            // 0 = 4:3 (default), 1 = 16:9, 2 = 16:10, 3 = 21:10
} NativeConfig;

typedef enum { CFG_BOOL, CFG_INT, CFG_ENUM } ConfigType;

typedef struct {
    const char *section;
    const char *key;
    const char *label;
    ConfigType type;
    void *valuePtr;             // points into g_config (e.g. &g_config.skipIntro)
    int min, max, step;         // slider bounds, ignored for CFG_BOOL/CFG_ENUM
    const ConfigEnumValue *enumValues;  // value-name pairs for CFG_ENUM
    int numEnumValues;                  // number of entries in enumValues
} ConfigEntry;

extern NativeConfig g_config;
extern const ConfigEntry g_configEntries[];
extern const int g_numConfigEntries;

void NativeConfig_Load(void);
void NativeConfig_Save(void);

#endif

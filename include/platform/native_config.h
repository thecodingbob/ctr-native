#ifndef NATIVE_CONFIG_H
#define NATIVE_CONFIG_H

#include <stdbool.h>

typedef struct {
    const char *name;
    int value;
} ConfigEnumValue;

typedef struct {
    bool skipIntro;             // false = off (default), true = skip all intros
    bool showReservesMeter;     // true = show the in-game reserves meter (default = false)
    bool skipHints;             // false = off (default), true = skip all mask hints in adventure mode
    bool extendedAdventureCharacterSelect; // false = garage, true = extended selector for new Adventure
    int speedMultiplier;        // percent, 10%..200%, 100 = 1.0x (default)
    int gravityMultiplier;      // percent, 10%..300%, 100 = 1.0x (default)
    int turnMultiplier;         // percent, 10%..400%, 100 = 1.0x (default)
    int jumpMultiplier;         // percent, 10%..300%, 100 = 1.0x (default)
    int reserveMultiplier;      // percent, 0%..400%,  100 = 1.0x (default)
    bool unlockAllCharacters;   // false = off (normal unlock logic), true = all characters already unlocked
    bool unlockAllGates;        // false = off, true = all adventure mode wood doors open without keys
    bool unlockAllPortals;      // false = off, true = all warp pads and boss garages unlocked
    bool increaseDrawDistance;  // false = off (default), true = render 3x farther
    bool disableSplitScreenLod; // false = off (default), true = load hi-res character models in 3+ multiplayer
    bool fullscreen;            // false = windowed, true = borderless fullscreen
    int aspectRatio;            // 0 = 4:3 (default), 1 = 16:9, 2 = 16:10, 3 = 21:10
    bool dithering;             // true = dithering enabled (default), false = disabled
    int renderScale;            // 1 = original, 2/3/4 = fixed multiples, 0 = native window
    bool smoothScaling;         // true = linear filtering for direct presentation
    bool textureFiltering;      // true = bilinear PSX texture sampling
    bool saveAnywhere;          // false = off (default), true = save anywhere in adventure mode by pressing select or from the pause menu
    int missileSpeedMultiplier; // percent, 20%..500%, 100 = 1.0x (default)
    int bombSpeedMultiplier;    // percent, 20%..300%, 100 = 1.0x (default)
    int warpballSpeedMultiplier; // percent, 20%..300%, 100 = 1.0x (default)
    int bombExplosionRadiusMultiplier; // percent, 50%..600%, 100 = 1.0x (default)
    int tntExplosionRadiusMultiplier;  // percent, 50%..600%, 100 = 1.0x (default)
    int maskMode;               // MASK_MODE_NORMAL=0(default), RANDOM=1, INVERTED=2, ALL_UKA=3, ALL_AKU=4
    bool maskProtectsFromDamage; // true (default) = immune to damage while mask is active, false = can take damage through mask
    bool maskDamagesOthers;      // true (default) = mask damages drivers on contact, false = no damage
    bool maskPersistsAfterOOB;   // false (default) = mask is consumed on OOB, true = mask continues after OOB
    int maskDurationMultiplier;  // percent, 20%..250%, 100 = 1.0x (default)
    int maskExtraSpeedMultiplier; // percent, 0%..300%, 100 = 1.0x (default)
    int clockDurationMultiplier;  // percent, 20%..250%, 100 = 1.0x (default)
    bool allowWeaponsDuringClock; // false (default) = clock blocks weapon usage, true = weapons work during clock
} NativeConfig;

enum
{
    MASK_MODE_NORMAL = 0,
    MASK_MODE_RANDOM = 1,
    MASK_MODE_INVERTED = 2,
    MASK_MODE_ALL_UKA = 3,
    MASK_MODE_ALL_AKU = 4,
};

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

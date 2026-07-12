#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <platform/native_config.h>
#include <platform/native_assets.h>
#include <platform/native_path.h>

static const ConfigEnumValue kAspectRatioValues[] = {
    {"4:3", 0},
    {"16:9", 1},
    {"16:10", 2},
    {"21:9", 3},
};
#define NUM_ASPECT_RATIOS (sizeof(kAspectRatioValues) / sizeof(kAspectRatioValues[0]))

NativeConfig g_config = {
  .skipIntro = false,
  .skipHints = false,

  .speedMultiplier = 100,
  .gravityMultiplier = 100,
  .turnMultiplier = 100,
  .jumpMultiplier = 100,
  .reserveMultiplier = 100,

  .missileSpeedMultiplier =  100,
  .bombSpeedMultiplier = 100,
  .warpballSpeedMultiplier = 100,
  .bombExplosionRadiusMultiplier = 100,

  .unlockAllCharacters = false,
  .unlockAllGates = false,
  .unlockAllPortals = false,

  .increaseDrawDistance = false,
  .disableSplitScreenLod = false,

  .fullscreen = false,
  .aspectRatio = 0,
  .dithering = true,
  .saveAnywhere = false
};

const ConfigEntry g_configEntries[] = {

    {
        .section = "General",
        .key = "skip_intro",
        .label = "Skip Intros",
        .type = CFG_BOOL,
        .valuePtr = &g_config.skipIntro
    },
    {
        .section = "Adventure",
        .key = "skip_hints",
        .label = "Skip Mask Hints",
        .type = CFG_BOOL,
        .valuePtr = &g_config.skipHints
    },
    {
        .section = "Adventure",
        .key = "save_anywhere",
        .label = "Save Anywhere",
        .type = CFG_BOOL,
        .valuePtr = &g_config.saveAnywhere
    },
    {
        .section = "Adventure",
        .key = "unlock_all_gates",
        .label = "Open All Gates",
        .type = CFG_BOOL,
        .valuePtr = &g_config.unlockAllGates
    },
    {
        .section = "Adventure",
        .key = "unlock_all_portals",
        .label = "Open All Portals",
        .type = CFG_BOOL,
        .valuePtr = &g_config.unlockAllPortals
    },
    {
        .section = "Vehicle",
        .key = "speed_stat_multiplier",
        .label = "Kart Speed Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.speedMultiplier,
        .min = 10,
        .max = 200,
        .step = 10
    },
    {
        .section = "Vehicle",
        .key = "gravity_stat_multiplier",
        .label = "Gravity Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.gravityMultiplier,
        .min = 10,
        .max = 300,
        .step = 10
    },
    {
        .section = "Vehicle",
        .key = "turn_stat_multiplier",
        .label = "Kart Turn Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.turnMultiplier,
        .min = 10,
        .max = 400,
        .step = 10
    },
    {
        .section = "Vehicle",
        .key = "jump_stat_multiplier",
        .label = "Kart Jump Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.jumpMultiplier,
        .min = 10,
        .max = 300,
        .step = 10
    },
    {
        .section = "Vehicle",
        .key = "turbo_reserves_multiplier",
        .label = "Turbo Reserves Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.reserveMultiplier,
        .min = 0,
        .max = 400,
        .step = 20
    },
    {
        .section = "Weapons",
        .key = "missile_speed_multiplier",
        .label = "Missile Speed Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.missileSpeedMultiplier,
        .min = 20,
        .max = 500,
        .step = 20
    },
    {
        .section = "Weapons",
        .key = "bomb_speed_multiplier",
        .label = "Bomb Speed Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.bombSpeedMultiplier,
        .min = 20,
        .max = 300,
        .step = 20
    },
    {
        .section = "Weapons",
        .key = "warpball_speed_multiplier",
        .label = "Warpball Speed Multiplier",
        .type = CFG_INT,
        .valuePtr = &g_config.warpballSpeedMultiplier,
        .min = 20,
        .max = 300,
        .step = 20
    },
    {
        .section = "Weapons",
        .key = "bomb_explosion_radius_multiplier",
        .label = "Bomb Explosion Radius",
        .type = CFG_INT,
        .valuePtr = &g_config.bombExplosionRadiusMultiplier,
        .min = 50,
        .max = 600,
        .step = 50
    },
    {
        .section = "Unlocks",
        .key = "unlock_all_characters",
        .label = "Unlock All Characters",
        .type = CFG_BOOL,
        .valuePtr = &g_config.unlockAllCharacters
    },
    {
        .section = "Graphics",
        .key = "increase_draw_distance",
        .label = "Increase Draw Distance",
        .type = CFG_BOOL,
        .valuePtr = &g_config.increaseDrawDistance
    },
    {
        .section = "Graphics",
        .key = "disable_split_screen_lod",
        .label = "Hi-Res Models in Multiplayer",
        .type = CFG_BOOL,
        .valuePtr = &g_config.disableSplitScreenLod
    },
    {
        .section = "Graphics",
        .key = "dithering",
        .label = "Dithering",
        .type = CFG_BOOL,
        .valuePtr = &g_config.dithering
    },
    {
        .section = "Graphics",
        .key = "fullscreen",
        .label = "Fullscreen",
        .type = CFG_BOOL,
        .valuePtr = &g_config.fullscreen
    },
    {
        .section = "Graphics",
        .key = "aspect_ratio",
        .label = "Aspect Ratio",
        .type = CFG_ENUM,
        .valuePtr = &g_config.aspectRatio,
        .enumValues = kAspectRatioValues,
        .numEnumValues = NUM_ASPECT_RATIOS
    }
};

const int g_numConfigEntries = sizeof(g_configEntries) / sizeof(g_configEntries[0]);

static bool ParseBool(const char *s)
{
    return strcmp(s, "true") == 0 || strcmp(s, "1") == 0;
}

static char *trimWhitespace(char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    if (*s == '\0')
        return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
    return s;
}

void NativeConfig_Load(void)
{
    printf("[Config] Base:       %s\n", NativeAssets_GetBaseDir());
    char path[512];
    NativePath_Join(path, sizeof(path), NativeStr8_FromCString(NativeAssets_GetBaseDir()), NATIVE_STR8_LIT("config.ini"));
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("[Config] config.ini NOT FOUND (%s)\n", path);
        return;
    }

    printf("[Config] config.ini opened OK\n");

    char line[256];
    char section[64] = "";

    while (fgets(line, sizeof(line), f))
    {
        char *p = trimWhitespace(line);

        if (*p == '\0' || *p == ';' || *p == '#')
            continue;

        if (*p == '[')
        {
            char *end = strchr(p + 1, ']');
            if (end)
            {
                *end = '\0';
                strncpy(section, p + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
            }
            printf("[Config] Section: [%s]\n", section);
            continue;
        }

        char *eq = strchr(p, '=');
        if (!eq)
            continue;

        *eq = '\0';
        char *key = trimWhitespace(p);
        char *value = trimWhitespace(eq + 1);

        for (int i = 0; i < g_numConfigEntries; i++)
        {
            const ConfigEntry *e = &g_configEntries[i];
            if (strcmp(section, e->section) == 0 &&
                strcmp(key, e->key) == 0)
            {
                if (e->type == CFG_BOOL)
                    *(bool *)e->valuePtr = ParseBool(value);
                else if (e->type == CFG_ENUM)
                {
                    int matched = 0;
                    for (int j = 0; j < e->numEnumValues; j++)
                    {
                        if (strcmp(value, e->enumValues[j].name) == 0)
                        {
                            *(int *)e->valuePtr = e->enumValues[j].value;
                            matched = 1;
                            break;
                        }
                    }
                    if (!matched)
                        *(int *)e->valuePtr = atoi(value);
                }
                else
                    *(int *)e->valuePtr = atoi(value);
                printf("[Config] %s/%s = %s\n", e->section, e->key, value);
                break;
            }
        }
    }

    fclose(f);
}

void NativeConfig_Save(void)
{
    char path[512];
    NativePath_Join(path, sizeof(path), NativeStr8_FromCString(NativeAssets_GetBaseDir()), NATIVE_STR8_LIT("config.ini"));
    FILE *f = fopen(path, "w");
    if (!f)
        return;

    const char *lastSection = NULL;

    for (int i = 0; i < g_numConfigEntries; i++)
    {
        const ConfigEntry *e = &g_configEntries[i];

        if (lastSection == NULL || strcmp(e->section, lastSection) != 0)
        {
            if (lastSection != NULL)
                fprintf(f, "\n");
            fprintf(f, "[%s]\n", e->section);
            lastSection = e->section;
        }

        if (e->type == CFG_BOOL)
            fprintf(f, "%s = %s\n", e->key, *(bool *)e->valuePtr ? "true" : "false");
        else if (e->type == CFG_ENUM)
        {
            int val = *(int *)e->valuePtr;
            const char *name = "?";
            for (int j = 0; j < e->numEnumValues; j++)
            {
                if (e->enumValues[j].value == val)
                {
                    name = e->enumValues[j].name;
                    break;
                }
            }
            fprintf(f, "%s = %s\n", e->key, name);
        }
        else
            fprintf(f, "%s = %d\n", e->key, *(int *)e->valuePtr);
    }

    fclose(f);
}

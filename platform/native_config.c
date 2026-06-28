#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <platform/native_config.h>
#include <platform/native_assets.h>

NativeConfig g_config = {false, false, 100, 100, false, false, false, false, false};

const ConfigEntry g_configEntries[] = {
    {"General",   "skip_intro",               "Skip Intros",                  CFG_BOOL, &g_config.skipIntro},
    {"Adventure", "skip_hints",               "Skip Mask Hints",              CFG_BOOL, &g_config.skipHints},
    {"Adventure", "unlock_all_gates",         "Open All Gates",               CFG_BOOL, &g_config.unlockAllGates},
    {"Adventure", "unlock_all_portals",       "Open All Portals",             CFG_BOOL, &g_config.unlockAllPortals},
    {"Vehicle",   "speed_stat_multiplier",    "Kart Speed Multiplier",        CFG_INT,  &g_config.speedMultiplier,        10, 200, 10},
    {"Vehicle",   "gravity_stat_multiplier",  "Gravity Multiplier",           CFG_INT,  &g_config.gravityMultiplier,      10, 300, 10},
    {"Vehicle",   "turn_stat_multiplier",     "Kart Turn Multiplier",         CFG_INT, &g_config.turnMultiplier,          10, 400, 10},
    {"Unlocks",   "unlock_all_characters",    "Unlock All Characters",        CFG_BOOL, &g_config.unlockAllCharacters},
    {"Graphics",  "increase_draw_distance",   "Increase Draw Distance",       CFG_BOOL, &g_config.increaseDrawDistance},
    {"Graphics",  "disable_split_screen_lod", "Hi-Res Models in Multiplayer", CFG_BOOL, &g_config.disableSplitScreenLod},
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
    FILE *f = fopen("build/config.ini", "r");
    if (!f) {
        printf("[Config] config.ini NOT FOUND (CWD is not base dir)\n");
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
    FILE *f = fopen("build/config.ini", "w");
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
        else
            fprintf(f, "%s = %d\n", e->key, *(int *)e->valuePtr);
    }

    fclose(f);
}

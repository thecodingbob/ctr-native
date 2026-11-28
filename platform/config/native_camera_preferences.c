#include <stdbool.h>
#include <stdio.h>
#include <platform/config/native_camera_preferences.h>
#include <platform/native_assets.h>
#include <platform/native_path.h>
#include <platform/native_ini.h>

enum { CAMERA_PREFERENCE_PLAYERS = 4 };
static int g_cameraZoom[CAMERA_PREFERENCE_PLAYERS];
static bool g_cameraZoomLoaded;

static void NativeCameraPreferences_Load(void)
{
    if (g_cameraZoomLoaded)
    {
        return;
    }
    g_cameraZoomLoaded = true;
    char path[512];
    NativePath_Join(path, sizeof(path), NativeStr8_FromCString(NativeAssets_GetBaseDir()), NATIVE_STR8_LIT("camera.ini"));
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        return;
    }
    for (int i = 0; i < CAMERA_PREFERENCE_PLAYERS; i++)
    {
        int value;
        if (fscanf(f, "%d", &value) != 1)
        {
            break;
        }
        g_cameraZoom[i] = value == 1;
    }
    fclose(f);
}

int NativeCameraPreferences_GetZoom(int player)
{
    if (player < 0 || player >= CAMERA_PREFERENCE_PLAYERS)
    {
        return 0;
    }
    NativeCameraPreferences_Load();
    return g_cameraZoom[player];
}

static bool NativeCameraPreferences_WriteIni(FILE *file, const void *context)
{
    const int *zoom = context;
    for (int i = 0; i < CAMERA_PREFERENCE_PLAYERS; i++)
    {
        if (fprintf(file, "%d\n", zoom[i]) < 0)
        {
            return false;
        }
    }
    return true;
}

void NativeCameraPreferences_SetZoom(int player, int zoom)
{
    if (player < 0 || player >= CAMERA_PREFERENCE_PLAYERS)
    {
        return;
    }
    NativeCameraPreferences_Load();
    g_cameraZoom[player] = zoom == 1;
    char path[512];
    if (!NativePath_Join(path, sizeof(path), NativeStr8_FromCString(NativeAssets_GetBaseDir()), NATIVE_STR8_LIT("camera.ini")) ||
        !NativeIni_Save(path, NativeCameraPreferences_WriteIni, g_cameraZoom))
    {
        fprintf(stderr, "[Camera] Could not save camera.ini\n");
    }
}

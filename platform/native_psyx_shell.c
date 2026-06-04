/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/PsyX_main.cpp
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include <SDL3/SDL.h>

#include "PsyX/PsyX_globals.h"
#include "platform/native_renderer_types.h"
#include "PsyX/PsyX_public.h"
#include "PsyX/PsyX_version.h"
#include "platform/native_psyx_shell.h"
#include "platform/native_audio.h"
#include "platform/native_gpu.h"
#include "platform/native_glad.h"
#include "platform/native_renderer.h"
#include "../externals/PsyCross/src/platform.h"

#include <psx/libetc.h>
#include <psx/libgte.h>
#include <psx/libgpu.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <pla.h>
#include <windows.h>
#endif

// NOTE(aalhendi): Native PsyX shell preserves the existing PsyX host behavior
// while keeping window/log/frame-entry ownership outside the GL backend.

SDL_Window *g_window = NULL;
int g_swapInterval = 1;
int g_enableSwapInterval = 1;
int g_skipSwapInterval = 0;

int g_cfg_swapInterval = 0;
GameOnTextInputHandler g_cfg_gameOnTextInput = NULL;

GameDebugKeysHandlerFunc g_dbg_gameDebugKeys = NULL;
GameDebugMouseHandlerFunc g_dbg_gameDebugMouse = NULL;
int g_dbg_polygonSelected = 0;

enum EPsxCounters
{
	PsxCounter_VBLANK,

	PsxCounter_Num
};

volatile int g_psxSysCounters[PsxCounter_Num];

int g_vmode = -1;
int g_frameSkip = 0;

int PsyX_Sys_SetVMode(int mode)
{
	int old = g_vmode;
	g_vmode = mode;

	return old;
}

int PsyX_Sys_GetVBlankCount()
{
	if (g_skipSwapInterval)
	{
		g_psxSysCounters[PsxCounter_VBLANK] += 1;
		g_frameSkip++;
	}

	return g_psxSysCounters[PsxCounter_VBLANK];
}

static int PsyX_Sys_InitialiseCore()
{
	return 1;
}

char *g_appNameStr = NULL;

void PsyX_GetWindowName(char *buffer)
{
#ifdef _DEBUG
	sprintf(buffer, "%s | Debug", g_appNameStr);
#else
	sprintf(buffer, "%s", g_appNameStr);
#endif
}

FILE *g_logStream = NULL;

void PsyX_Log_Initialise()
{
	char appLogFilename[128];
	sprintf(appLogFilename, "%s.log", g_appNameStr);

	g_logStream = fopen(appLogFilename, "wb");

	if (!g_logStream)
		eprinterr("Error - cannot create log file '%s'\n", appLogFilename);
}

void PsyX_Log_Finalise()
{
	PsyX_Log_Warning("---- LOG CLOSED ----\n");

	if (g_logStream)
		fclose(g_logStream);

	g_logStream = NULL;
}

void PsyX_Log_Flush()
{
	if (g_logStream)
		fflush(g_logStream);
}

typedef enum
{
	SPEW_NORM,
	SPEW_INFO,
	SPEW_WARNING,
	SPEW_ERROR,
	SPEW_SUCCESS,
} SpewType_t;

#ifdef _WIN32
static unsigned short g_InitialColor = 0xFFFF;
static unsigned short g_LastColor = 0xFFFF;
static unsigned short g_BadColor = 0xFFFF;
static WORD g_BackgroundFlags = 0xFFFF;
CRITICAL_SECTION g_SpewCS;
char g_bSpewCSInitted = 0;

static void Spew_GetInitialColors()
{
	CONSOLE_SCREEN_BUFFER_INFO oldInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &oldInfo);
	g_InitialColor = g_LastColor = oldInfo.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	g_BackgroundFlags = oldInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);

	g_BadColor = 0;
	if (g_BackgroundFlags & BACKGROUND_RED)
		g_BadColor |= FOREGROUND_RED;
	if (g_BackgroundFlags & BACKGROUND_GREEN)
		g_BadColor |= FOREGROUND_GREEN;
	if (g_BackgroundFlags & BACKGROUND_BLUE)
		g_BadColor |= FOREGROUND_BLUE;
	if (g_BackgroundFlags & BACKGROUND_INTENSITY)
		g_BadColor |= FOREGROUND_INTENSITY;
}

static WORD Spew_SetConsoleTextColor(int red, int green, int blue, int intensity)
{
	WORD ret = g_LastColor;

	g_LastColor = 0;
	if (red)
		g_LastColor |= FOREGROUND_RED;
	if (green)
		g_LastColor |= FOREGROUND_GREEN;
	if (blue)
		g_LastColor |= FOREGROUND_BLUE;
	if (intensity)
		g_LastColor |= FOREGROUND_INTENSITY;

	if (g_LastColor == g_BadColor)
		g_LastColor = g_InitialColor;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), g_LastColor | g_BackgroundFlags);
	return ret;
}

static void Spew_RestoreConsoleTextColor(WORD color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color | g_BackgroundFlags);
	g_LastColor = color;
}

void Spew_ConDebugSpew(SpewType_t type, char *text)
{
	if (!g_bSpewCSInitted)
	{
		Spew_GetInitialColors();
		InitializeCriticalSection(&g_SpewCS);
		g_bSpewCSInitted = 1;
	}

	WORD old;
	EnterCriticalSection(&g_SpewCS);
	{
		if (type == SPEW_NORM)
		{
			old = Spew_SetConsoleTextColor(1, 1, 1, 0);
		}
		else if (type == SPEW_WARNING)
		{
			old = Spew_SetConsoleTextColor(1, 1, 0, 1);
		}
		else if (type == SPEW_SUCCESS)
		{
			old = Spew_SetConsoleTextColor(0, 1, 0, 1);
		}
		else if (type == SPEW_ERROR)
		{
			old = Spew_SetConsoleTextColor(1, 0, 0, 1);
		}
		else if (type == SPEW_INFO)
		{
			old = Spew_SetConsoleTextColor(0, 1, 1, 1);
		}
		else
		{
			old = Spew_SetConsoleTextColor(1, 1, 1, 1);
		}

		OutputDebugStringA(text);
		printf("%s", text);

		Spew_RestoreConsoleTextColor(old);
	}
	LeaveCriticalSection(&g_SpewCS);
}
#endif

void PrintMessageToOutput(SpewType_t spewtype, char const *pMsgFormat, va_list args)
{
	static char pTempBuffer[4096];
	int len = 0;
	vsprintf(&pTempBuffer[len], pMsgFormat, args);

#ifdef WIN32
	Spew_ConDebugSpew(spewtype, pTempBuffer);
#elif defined(__EMSCRIPTEN__)
	if (spewtype == SPEW_INFO)
	{
		EM_ASM({
			console.info(UTF8ToString($0));
		}, pTempBuffer);
	}
	else if (spewtype == SPEW_WARNING)
	{
		EM_ASM({
			console.warn(UTF8ToString($0));
		}, pTempBuffer);
	}
	else if (spewtype == SPEW_ERROR)
	{
		EM_ASM({
			console.error(UTF8ToString($0));
		}, pTempBuffer);
	}
	else
	{
		EM_ASM({
			console.log(UTF8ToString($0));
		}, pTempBuffer);
	}
#else
	printf(pTempBuffer);
#endif

	if (g_logStream)
		fprintf(g_logStream, pTempBuffer);
}

void PsyX_Log(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	PrintMessageToOutput(SPEW_NORM, fmt, argptr);
	va_end(argptr);
}

void PsyX_Log_Info(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	PrintMessageToOutput(SPEW_INFO, fmt, argptr);
	va_end(argptr);
}

void PsyX_Log_Warning(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	PrintMessageToOutput(SPEW_WARNING, fmt, argptr);
	va_end(argptr);
}

void PsyX_Log_Error(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	PrintMessageToOutput(SPEW_ERROR, fmt, argptr);
	va_end(argptr);
}

void PsyX_Log_Success(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	PrintMessageToOutput(SPEW_SUCCESS, fmt, argptr);
	va_end(argptr);
}

void PsyX_Initialise(char *appName, int width, int height, int fullscreen)
{
	char windowNameStr[128];

	g_appNameStr = appName;

	PsyX_Log_Initialise();
	PsyX_GetWindowName(windowNameStr);

#if defined(_WIN32) && defined(_DEBUG)
	if (AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
		SetConsoleTitleA(windowNameStr);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}
#endif

	eprintinfo("Initialising Psy-X %d.%d\n", PSYX_MAJOR_VERSION, PSYX_MINOR_VERSION);
	eprintinfo("Build date: %s:%s\n", PSYX_COMPILE_DATE, PSYX_COMPILE_TIME);

#if defined(__EMSCRIPTEN__)
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");
#endif

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		eprinterr("Failed to initialise SDL\n");
		PsyX_Shutdown();
		return;
	}

	if (!NativeRenderer_InitialiseRender(windowNameStr, width, height, fullscreen))
	{
		eprinterr("Failed to Intialise Window\n");
		PsyX_Shutdown();
		return;
	}

	if (!PsyX_Sys_InitialiseCore())
	{
		eprinterr("Failed to Intialise Psy-X Core.\n");
		PsyX_Shutdown();
		return;
	}

	if (!NativeRenderer_InitialisePSX())
	{
		eprinterr("Failed to Intialise PSX.\n");
		PsyX_Shutdown();
		return;
	}

	atexit(PsyX_Shutdown);
	SDL_HideCursor();
}

void PsyX_GetScreenSize(int *screenWidth, int *screenHeight)
{
	SDL_GetWindowSize(g_window, screenWidth, screenHeight);
}

void PsyX_SetCursorPosition(int x, int y)
{
	SDL_WarpMouseInWindow(g_window, x, y);
}

void PsyX_RequestExit(void)
{
	exit(0);
}

void PsyX_HandleHostWindowResize(int width, int height)
{
	g_windowWidth = width;
	g_windowHeight = height;
	NativeRenderer_ResetDevice();
}

void PsyX_HandleHostFullscreenToggle(void)
{
	int fullscreen = (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) != 0;

	SDL_SetWindowFullscreen(g_window, fullscreen == 0);
	SDL_GetWindowSize(g_window, &g_windowWidth, &g_windowHeight);
	NativeRenderer_ResetDevice();
}

void PsyX_HandleHostMouseMotion(int x, int y)
{
	if (g_dbg_gameDebugMouse)
		g_dbg_gameDebugMouse(x, y);
}

void PsyX_HandleHostTextInput(const char *text)
{
	if (g_cfg_gameOnTextInput)
		g_cfg_gameOnTextInput(text);
}

char begin_scene_flag = 0;

char PsyX_BeginScene()
{
	if (begin_scene_flag)
		return 0;

	assert(!begin_scene_flag);

	{
		int swapInterval = (g_cfg_swapInterval && g_enableSwapInterval && !g_skipSwapInterval) ? g_swapInterval : 0;

		SDL_DisplayID displayId = SDL_GetDisplayForWindow(g_window);
		const SDL_DisplayMode *curMode = displayId != 0 ? SDL_GetCurrentDisplayMode(displayId) : NULL;
		if (curMode != NULL)
		{
			const int mode_frequency = g_vmode == MODE_NTSC ? VBLANK_FREQUENCY_NTSC : VBLANK_FREQUENCY_PAL;
			if (curMode->refresh_rate < (float)mode_frequency)
				swapInterval--;
		}

		if (swapInterval < 0)
			swapInterval = 0;

		NativeRenderer_UpdateSwapIntervalState(swapInterval);
	}

	NativeRenderer_BeginScene();

	if (activeDrawEnv.isbg)
	{
		const RECT16 clipenv = activeDrawEnv.clip;
		const u_char r = activeDrawEnv.r0;
		const u_char g = activeDrawEnv.g0;
		const u_char b = activeDrawEnv.b0;

		NativeRenderer_Clear(clipenv.x, clipenv.y, clipenv.w, clipenv.h, r, g, b);
	}

	begin_scene_flag = 1;

	PsyX_Log_Flush();

	return 1;
}

uint PsyX_CalcFPS();

void PsyX_EndScene()
{
	if (!begin_scene_flag)
		return;

	assert(begin_scene_flag);
	begin_scene_flag = 0;

#if USE_PGXP
	PGXP_ClearCache();
#endif

	NativeRenderer_EndScene();

	NativeRenderer_StoreFrameBuffer(activeDispEnv.disp.x, activeDispEnv.disp.y, activeDispEnv.disp.w, activeDispEnv.disp.h);

	NativeRenderer_SwapWindow();
}

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
void PsyX_TakeScreenshot()
{
	u_char *pixels = (u_char *)malloc(g_windowWidth * g_windowHeight * 4);

#if defined(RENDERER_OGL)
	glReadPixels(0, 0, g_windowWidth, g_windowHeight, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
#elif defined(RENDERER_OGLES)
	glReadPixels(0, 0, g_windowWidth, g_windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#endif

#if defined(RENDERER_OGL)
	SDL_Surface *surface = SDL_CreateSurfaceFrom(g_windowWidth, g_windowHeight, SDL_PIXELFORMAT_BGRA8888, pixels, g_windowWidth * 4);
#elif defined(RENDERER_OGLES)
	SDL_Surface *surface = SDL_CreateSurfaceFrom(g_windowWidth, g_windowHeight, SDL_PIXELFORMAT_RGBA8888, pixels, g_windowWidth * 4);
#endif

	SDL_SaveBMP(surface, "SCREENSHOT.BMP");
	SDL_DestroySurface(surface);

	free(pixels);
}
#endif

void PsyX_HandleHostKey(int nKey, char down)
{
	if (g_dbg_gameDebugKeys)
		g_dbg_gameDebugKeys(nKey, down);

	if (nKey == SDL_SCANCODE_BACKSPACE)
	{
		if (down)
			g_skipSwapInterval = 1;
		else
			g_skipSwapInterval = 0;
	}

	if (!down)
	{
		switch (nKey)
		{
#ifdef _DEBUG
		case SDL_SCANCODE_F1:
			g_dbg_wireframeMode ^= 1;
			eprintwarn("wireframe mode: %d\n", g_dbg_wireframeMode);
			break;

		case SDL_SCANCODE_F2:
			g_dbg_texturelessMode ^= 1;
			eprintwarn("textureless mode: %d\n", g_dbg_texturelessMode);
			break;
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_DOWN:
			if (g_dbg_emulatorPaused)
			{
				g_dbg_polygonSelected += (nKey == SDL_SCANCODE_UP) ? 3 : -3;
			}
			break;
		case SDL_SCANCODE_F10:
			eprintwarn("saving VRAM.TGA\n");
			NativeRenderer_SaveVRAM("VRAM.TGA", 0, 0, VRAM_WIDTH, VRAM_HEIGHT, 1);
			break;
#endif
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
		case SDL_SCANCODE_F12:
			eprintwarn("Saving screenshot...\n");
			PsyX_TakeScreenshot();
			break;
#endif
		case SDL_SCANCODE_F3:
			g_cfg_bilinearFiltering ^= 1;
			eprintwarn("filtering mode: %d\n", g_cfg_bilinearFiltering);
			break;
		case SDL_SCANCODE_F5:
			g_cfg_pgxpTextureCorrection ^= 1;
			break;
		case SDL_SCANCODE_F6:
			g_cfg_pgxpZBuffer ^= 1;
			break;
		}
	}
}

uint PsyX_CalcFPS()
{
#define FPS_INTERVAL 1.0

	static unsigned int lastTime = 0;
	static unsigned int currentFps = 0;
	static unsigned int passedFrames = 0;

	lastTime = SDL_GetTicks();

	passedFrames++;
	if (lastTime < SDL_GetTicks() - FPS_INTERVAL * 1000)
	{
		lastTime = SDL_GetTicks();
		currentFps = passedFrames;
		passedFrames = 0;
	}

	return currentFps;
}

void PsyX_SetSwapInterval(int interval)
{
	g_swapInterval = interval;
}

void PsyX_EnableSwapInterval(int enable)
{
	g_enableSwapInterval = enable;
}

void PsyX_WaitForTimestep(int count)
{
	if (!g_skipSwapInterval)
	{
		static int swapLastVbl = 0;

		int vbl;
		do
		{
#ifdef __EMSCRIPTEN__
			emscripten_sleep(0);
#endif
			vbl = PsyX_Sys_GetVBlankCount();
		} while (vbl - swapLastVbl < count);

		swapLastVbl = PsyX_Sys_GetVBlankCount();
	}
}

void PsyX_Shutdown()
{
	if (!g_window)
		return;

	SDL_DestroyWindow(g_window);
	g_window = NULL;

	NativeAudio_Shutdown();
	NativeRenderer_Shutdown();
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);

	SDL_Quit();

	PsyX_Log_Finalise();
}

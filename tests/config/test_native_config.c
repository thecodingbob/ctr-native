// Configuration and profile tests use their own scratch directory.
#include <SDL3/SDL.h>
#include <macros.h>
#include <shared/ctr_test.h>
#include <shared/ctr_test_scratch.h>
#include <platform/native_path.h>
#include <platform/native_external_file.h>

static char s_testBaseDir[512];
static const char *s_testExecutableDir;

const char *NativeAssets_GetBaseDir(void) { return s_testBaseDir; }
const char *NativeAssets_GetExecutableDir(void) { return s_testExecutableDir != NULL ? s_testExecutableDir : s_testBaseDir; }
int NativeTextRuntime_LanguageSlot(const char *id) { (void)id; return -1; }
const char *NativeTextRuntime_LanguageId(int slot) { (void)slot; return NULL; }

#include <platform/config/native_config_ini.c>
#include <platform/config/native_config.c>
#include <platform/config/native_profile.c>
#include <platform/config/native_camera_preferences.c>

static void TestChaosModeOption(void)
{
	CTR_TEST_SECTION("Chaos setup is opt-in under General and persists independently of Expanded Options");
	const NativeConfig initial = g_config;
	CTR_TEST_CHECK(!g_config.chaosMode, "Chaos selection is disabled by default");
	const ConfigEntry *entry = NULL;
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		if (strcmp(g_configEntries[i].key, "chaos_mode") == 0)
		{
			entry = &g_configEntries[i];
			break;
		}
	}
	CTR_TEST_CHECK(entry != NULL && strcmp(entry->section, "General") == 0 &&
	               entry->type == CFG_BOOL && entry->valuePtr == &g_config.chaosMode,
	               "the General menu and INI share the Chaos toggle");
	g_config.chaosMode = true;
	g_config.expandedAdventureOptions = false;
	NativeConfig_Save();
	g_config.chaosMode = false;
	CTR_TEST_CHECK(NativeConfig_Load() == CONFIG_LOAD_OK && g_config.chaosMode && !g_config.expandedAdventureOptions,
	               "enabled session Chaos survives saving without requiring Adventure options");
	g_config.chaosMode = false;
	g_config.expandedAdventureOptions = true;
	NativeConfig_Save();
	g_config.chaosMode = true;
	CTR_TEST_CHECK(NativeConfig_Load() == CONFIG_LOAD_OK && !g_config.chaosMode && g_config.expandedAdventureOptions,
	               "disabled session Chaos survives saving independently of enabled Adventure options");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	remove(path);
	g_config = initial;
}

static void TestProfileStartup(void)
{
	CTR_TEST_SECTION("startup loads the selected profile before config overrides");
	char directory[512], profilePath[512], configPath[512], vanillaPath[512], expandedPath[512];
	NativePath_Join(directory, sizeof(directory), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("resources/presets"));
	NativePath_Join(profilePath, sizeof(profilePath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("profile.ini"));
	NativePath_Join(configPath, sizeof(configPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	NativePath_Join(vanillaPath, sizeof(vanillaPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("resources/presets/vanilla.ini"));
	NativePath_Join(expandedPath, sizeof(expandedPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("resources/presets/expanded.ini"));
	const NativeConfig initial = g_config;
	remove(profilePath);
	remove(configPath);
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_NO_PROFILE, "missing profile requests selection");
	CTR_TEST_CHECK(NativeProfile_GetStartupConfigResult() == CONFIG_LOAD_NOT_FOUND, "fresh install has no startup config");
	CTR_TEST_CHECK(SDL_CreateDirectory(directory), "scratch preset directory exists");
	FILE *file = fopen(vanillaPath, "w");
	CTR_TEST_CHECK(file != NULL, "Vanilla preset fixture can be written");
	if (file == NULL) { return; }
	fputs("[General]\nskip_intro = false\nshow_reserves_meter = true\n", file);
	fclose(file);
	file = fopen(expandedPath, "w");
	CTR_TEST_CHECK(file != NULL, "Expanded preset fixture can be written");
	if (file == NULL) { remove(vanillaPath); return; }
	fputs("[General]\nskip_intro = true\nshow_reserves_meter = true\n", file);
	fclose(file);
	remove(configPath);
	CTR_TEST_CHECK(NativeConfig_SaveProfile(CONFIG_PROFILE_EXPANDED), "fresh install profile can be saved");
	g_config.skipIntro = false;
	g_config.showReservesMeter = false;
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_READY && g_config.skipIntro && g_config.showReservesMeter,
	               "fresh install loads preset values without config.ini");
	file = fopen(configPath, "w");
	CTR_TEST_CHECK(file != NULL, "config fixture can be written");
	if (file == NULL) { remove(vanillaPath); remove(expandedPath); return; }
	fputs("[General]\nshow_reserves_meter = false\n", file);
	fclose(file);
	remove(profilePath);
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_NO_PROFILE && NativeProfile_GetStartupConfigResult() == CONFIG_LOAD_OK,
	               "missing profile with existing config is distinguishable from a fresh install");
	remove(configPath);
	CTR_TEST_CHECK(NativeProfile_GetStartupConfigResult() == CONFIG_LOAD_OK,
	               "startup config result is retained when the file changes later");
	file = fopen(configPath, "w");
	CTR_TEST_CHECK(file != NULL, "config fixture can be restored");
	if (file == NULL) { remove(vanillaPath); remove(expandedPath); return; }
	fputs("[General]\nshow_reserves_meter = false\n", file);
	fclose(file);
	g_config.skipIntro = false;
	g_config.showReservesMeter = false;
	g_config.engineSelection = true;
	CTR_TEST_CHECK(NativeProfile_Select(CONFIG_PROFILE_EXPANDED, false) == CONFIG_LOAD_OK,
	               "profile can be selected for future defaults only");
	CTR_TEST_CHECK(!g_config.skipIntro && !g_config.showReservesMeter && g_config.engineSelection,
	               "future-default selection preserves every current option");
	ConfigProfile selected = CONFIG_PROFILE_VANILLA;
	CTR_TEST_CHECK(NativeConfig_LoadProfile(&selected) == CONFIG_LOAD_OK && selected == CONFIG_PROFILE_EXPANDED,
	               "future-default selection persists the selected profile");
	CTR_TEST_CHECK(NativeProfile_Select(CONFIG_PROFILE_EXPANDED, true) == CONFIG_LOAD_OK,
	               "selected preset can be applied");
	CTR_TEST_CHECK(g_config.skipIntro && g_config.showReservesMeter && g_config.engineSelection,
	               "applying a preset changes only its listed options");
	CTR_TEST_CHECK(NativeProfile_Select((ConfigProfile)99, true) == CONFIG_LOAD_INVALID,
	               "invalid profile selection is rejected");
	CTR_TEST_CHECK(NativeConfig_SaveProfile(CONFIG_PROFILE_EXPANDED), "Expanded profile can be selected");
	g_config.skipIntro = false;
	g_config.showReservesMeter = true;
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_READY && NativeProfile_GetStartupStatus() == PROFILE_STARTUP_READY,
	               "selected Expanded preset loads successfully");
	CTR_TEST_CHECK(g_config.skipIntro && !g_config.showReservesMeter, "config overrides only its explicit value");
	CTR_TEST_CHECK(NativeConfig_SaveProfile(CONFIG_PROFILE_VANILLA), "Vanilla profile can be selected");
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_READY && !g_config.skipIntro,
	               "Vanilla selection loads the Vanilla preset");
	file = fopen(profilePath, "w");
	CTR_TEST_CHECK(file != NULL, "profile can be edited manually");
	if (file != NULL)
	{
		fputs("[Profile]\nactive = Expanded\n", file);
		fclose(file);
		g_config.skipIntro = false;
		g_config.showReservesMeter = true;
		CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_READY && g_config.skipIntro && !g_config.showReservesMeter,
		               "manual profile change affects missing values but preserves explicit config");
		NativeConfig_Save();
		file = fopen(profilePath, "w");
		CTR_TEST_CHECK(file != NULL, "profile can be changed after saving config");
		if (file != NULL)
		{
			fputs("[Profile]\nactive = Vanilla\n", file);
			fclose(file);
			g_config.skipIntro = false;
			g_config.showReservesMeter = true;
			CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_READY && g_config.skipIntro && !g_config.showReservesMeter,
			               "saved explicit settings survive reload and a manual profile change");
		}
	}
	remove(vanillaPath);
	CTR_TEST_CHECK(NativeProfile_Select(CONFIG_PROFILE_VANILLA, true) == CONFIG_LOAD_NOT_FOUND,
	               "missing preset cannot be selected");
	CTR_TEST_CHECK(NativeConfig_LoadProfile(&selected) == CONFIG_LOAD_OK && selected == CONFIG_PROFILE_VANILLA,
	               "failed selection preserves the previously saved profile");
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_PRESET_ERROR, "missing preset is reported without stopping startup");
	file = fopen(profilePath, "w");
	CTR_TEST_CHECK(file != NULL, "invalid profile fixture can be written");
	if (file != NULL)
	{
		fputs("[Profile]\nactive = Unknown\n", file);
		fclose(file);
		CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_INVALID_PROFILE, "invalid profile is reported");
	}
	remove(profilePath);
	remove(configPath);
	remove(expandedPath);
	SDL_RemovePath(directory);
	NativePath_Join(directory, sizeof(directory), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("resources"));
	SDL_RemovePath(directory);
	g_config = initial;
}

static void TestDeveloperDefaultsAreImmutable(void)
{
	CTR_TEST_SECTION("developer defaults do not depend on load order");
	const NativeConfig initial = g_config;
	g_config.developerHacksEnabled = false;
	g_config.oneLapRaces = true;
	const ConfigEntry *gate = NULL;
	for (int i = 0; i < g_numConfigEntries; i++)
	{
		if (g_configEntries[i].valuePtr == &g_config.developerHacksEnabled)
		{
			gate = &g_configEntries[i];
			break;
		}
	}
	CTR_TEST_CHECK(gate != NULL, "developer gate is registered");
	NativeConfig_ApplyDependencies(gate);
	CTR_TEST_CHECK(!g_config.oneLapRaces, "disabled developer hacks reset before any load");
	g_config = initial;
}

static void TestConfigPresetPrecedence(void)
{
	CTR_TEST_SECTION("configuration overrides preset defaults");
	char presetPath[512];
	char configPath[512];
	NativePath_Join(presetPath, sizeof(presetPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("preset-test.ini"));
	NativePath_Join(configPath, sizeof(configPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	const NativeConfig initial = g_config;
	FILE *file = fopen(presetPath, "w");
	CTR_TEST_CHECK(file != NULL, "scratch preset can be created");
	if (file == NULL) { return; }
	fputs("[General]\nshow_reserves_meter = true\nskip_intro = true\n", file);
	fclose(file);
	file = fopen(configPath, "w");
	CTR_TEST_CHECK(file != NULL, "scratch configuration can be created");
	if (file == NULL) { remove(presetPath); return; }
	fputs("[General]\nshow_reserves_meter = false\n", file);
	fclose(file);
	g_config.showReservesMeter = false;
	g_config.skipIntro = false;
	CTR_TEST_CHECK(NativeConfig_ReadPresetDefaults(presetPath) == CONFIG_LOAD_OK, "valid preset loads");
	NativeConfig_Load();
	CTR_TEST_CHECK(!g_config.showReservesMeter && g_config.skipIntro,
	               "explicit config value wins while a missing value uses the preset");
	file = fopen(presetPath, "w");
	CTR_TEST_CHECK(file != NULL, "invalid preset can be created");
	if (file != NULL)
	{
		fputs("[General]\nskip_intro = true\nunknown = true\n", file);
		fclose(file);
		g_config.skipIntro = false;
		g_config.showReservesMeter = true;
		CTR_TEST_CHECK(NativeConfig_ReadPresetDefaults(presetPath) == CONFIG_LOAD_INVALID, "invalid preset is reported");
		NativeConfig_Load();
		CTR_TEST_CHECK(!g_config.skipIntro && !g_config.showReservesMeter,
		               "invalid preset is discarded and the user's configuration still loads");
	}
	remove(presetPath);
	remove(configPath);
	g_config = initial;
}

static void TestProfileDeveloperDependencies(void)
{
	CTR_TEST_SECTION("developer dependencies apply after preset and config layers");
	char presetPath[512], configPath[512];
	NativePath_Join(presetPath, sizeof(presetPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("preset-test.ini"));
	NativePath_Join(configPath, sizeof(configPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	const NativeConfig initial = g_config;
	FILE *file = fopen(presetPath, "w");
	CTR_TEST_CHECK(file != NULL, "developer preset fixture can be written");
	if (file == NULL) { return; }
	fputs("[Developer Hacks]\ndeveloper_hacks_enabled = true\none_lap_races = true\n", file);
	fclose(file);
	file = fopen(configPath, "w");
	CTR_TEST_CHECK(file != NULL, "developer config fixture can be written");
	if (file == NULL) { remove(presetPath); return; }
	fputs("[Developer Hacks]\ndeveloper_hacks_enabled = false\n", file);
	fclose(file);
	CTR_TEST_CHECK(NativeConfig_ReadPresetDefaults(presetPath) == CONFIG_LOAD_OK, "developer preset loads");
	NativeConfig_Load();
	CTR_TEST_CHECK(!g_config.developerHacksEnabled && !g_config.oneLapRaces,
	               "config disabling the gate resets preset developer hacks");
	file = fopen(configPath, "w");
	CTR_TEST_CHECK(file != NULL, "enabled developer config fixture can be written");
	if (file != NULL)
	{
		fputs("[Developer Hacks]\none_lap_races = true\ndeveloper_hacks_enabled = true\n", file);
		fclose(file);
		g_config.developerHacksEnabled = false;
		g_config.oneLapRaces = false;
		NativeConfig_Load();
		CTR_TEST_CHECK(g_config.developerHacksEnabled && g_config.oneLapRaces,
		               "config enables hacks regardless of key order");
	}
	remove(configPath);
	file = fopen(presetPath, "w");
	CTR_TEST_CHECK(file != NULL, "disabled developer preset fixture can be written");
	if (file != NULL)
	{
		fputs("[Developer Hacks]\none_lap_races = true\ndeveloper_hacks_enabled = false\n", file);
		fclose(file);
		CTR_TEST_CHECK(NativeConfig_ReadPresetDefaults(presetPath) == CONFIG_LOAD_OK, "disabled developer preset loads");
		NativeConfig_Load();
		CTR_TEST_CHECK(!g_config.developerHacksEnabled && !g_config.oneLapRaces,
		               "missing config still resets disabled preset hacks");
		g_config.developerHacksEnabled = true;
		g_config.oneLapRaces = true;
		CTR_TEST_CHECK(NativeConfig_ApplyPresetFile(presetPath) == CONFIG_LOAD_OK, "developer preset applies directly");
		CTR_TEST_CHECK(!g_config.developerHacksEnabled && !g_config.oneLapRaces,
		               "direct preset application resets disabled hacks");
	}
	remove(presetPath);
	g_config = initial;
}

static void TestProfileFile(void)
{
	CTR_TEST_SECTION("profile persistence is separate from configuration");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("profile.ini"));
	remove(path);
	ConfigProfile profile = CONFIG_PROFILE_EXPANDED;
	CTR_TEST_CHECK(NativeConfig_LoadProfile(&profile) == CONFIG_LOAD_NOT_FOUND, "missing profile is reported");
	CTR_TEST_CHECK(profile == CONFIG_PROFILE_EXPANDED, "missing profile does not alter the output");
	CTR_TEST_CHECK(NativeConfig_SaveProfile(CONFIG_PROFILE_VANILLA), "Vanilla profile is saved");
	CTR_TEST_CHECK(NativeConfig_LoadProfile(&profile) == CONFIG_LOAD_OK && profile == CONFIG_PROFILE_VANILLA,
	               "Vanilla profile survives a round trip");
	CTR_TEST_CHECK(NativeConfig_SaveProfile(CONFIG_PROFILE_EXPANDED), "Expanded profile is saved");
	CTR_TEST_CHECK(NativeConfig_LoadProfile(&profile) == CONFIG_LOAD_OK && profile == CONFIG_PROFILE_EXPANDED,
	               "Expanded profile survives a round trip");
	CTR_TEST_CHECK(!NativeConfig_SaveProfile((ConfigProfile)99), "invalid profile cannot be saved");
	CTR_TEST_CHECK(NativeConfig_LoadProfile(&profile) == CONFIG_LOAD_OK && profile == CONFIG_PROFILE_EXPANDED,
	               "invalid save leaves the existing profile intact");
	FILE *file = fopen(path, "w");
	CTR_TEST_CHECK(file != NULL, "invalid profile fixture can be written");
	if (file != NULL)
	{
		fputs("[Profile]\nactive = unknown\n", file);
		fclose(file);
		CTR_TEST_CHECK(NativeConfig_LoadProfile(&profile) == CONFIG_LOAD_INVALID && profile == CONFIG_PROFILE_EXPANDED,
		               "invalid profile leaves the output intact");
	}
	file = fopen(path, "w");
	CTR_TEST_CHECK(file != NULL, "empty profile fixture can be written");
	if (file != NULL)
	{
		fputs("[Profile]\n", file);
		fclose(file);
		CTR_TEST_CHECK(NativeConfig_LoadProfile(&profile) == CONFIG_LOAD_INVALID && profile == CONFIG_PROFILE_EXPANDED,
		               "profile without an active value is invalid");
	}
	remove(path);
}

static void TestPresetFile(void)
{
	CTR_TEST_SECTION("preset files are validated before applying");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("preset-test.ini"));
	const NativeConfig initial = g_config;
	FILE *file = fopen(path, "w");
	CTR_TEST_CHECK(file != NULL, "scratch preset can be written");
	if (file == NULL)
	{
		return;
	}
	fputs("[General]\nshow_reserves_meter = true\n[Vehicle]\nspeed_stat_multiplier = 120\n[Graphics]\nrender_scale = Original\n", file);
	fclose(file);
	g_config.showReservesMeter = false;
	g_config.skipIntro = true;
	CTR_TEST_CHECK(NativeConfig_ApplyPresetFile(path) == CONFIG_LOAD_OK, "valid preset is accepted");
	CTR_TEST_CHECK(g_config.showReservesMeter && g_config.speedMultiplier == 120 && g_config.renderScale == 1,
	               "boolean, integer and enum preset values are applied");
	CTR_TEST_CHECK(g_config.skipIntro, "options omitted from the preset are preserved");
	char configPath[512];
	NativePath_Join(configPath, sizeof(configPath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	file = fopen(configPath, "w");
	CTR_TEST_CHECK(file != NULL, "scratch config can be written for parser comparison");
	if (file != NULL)
	{
		fputs("[General]\nshow_reserves_meter = true\n[Vehicle]\nspeed_stat_multiplier = 120\n[Graphics]\nrender_scale = Original\n", file);
		fclose(file);
		g_config.showReservesMeter = false;
		g_config.speedMultiplier = 100;
		g_config.renderScale = 0;
		NativeConfig_Load();
		CTR_TEST_CHECK(g_config.showReservesMeter && g_config.speedMultiplier == 120 && g_config.renderScale == 1,
		               "config and preset files interpret valid values identically");
		remove(configPath);
	}

	const char *invalid[] = {
	    "[General]\nshow_reserves_meter = false\nunknown = true\n",
	    "[General]\nshow_reserves_meter = perhaps\n",
	    "[Vehicle]\nspeed_stat_multiplier = 999\n",
	    "[Graphics]\nrender_scale = 99\n",
	    "[General\nshow_reserves_meter = false\n",
	};
	for (int i = 0; i < (int)(sizeof(invalid) / sizeof(invalid[0])); i++)
	{
		file = fopen(path, "w");
		CTR_TEST_CHECK(file != NULL, "invalid scratch preset can be written");
		if (file == NULL)
		{
			break;
		}
		fputs(invalid[i], file);
		fclose(file);
		CTR_TEST_CHECK(NativeConfig_ApplyPresetFile(path) == CONFIG_LOAD_INVALID, "invalid preset is rejected");
		CTR_TEST_CHECK(g_config.showReservesMeter && g_config.speedMultiplier == 120 && g_config.renderScale == 1,
		               "invalid preset does not partially alter configuration");
	}
	remove(path);
	CTR_TEST_CHECK(NativeConfig_ApplyPresetFile(path) == CONFIG_LOAD_NOT_FOUND, "missing preset is reported");
	g_config = initial;
}

static void TestAdventureOptionsWithoutLegacyMigration(void)
{
	CTR_TEST_SECTION("Adventure options ignore obsolete keys");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	const NativeConfig initial = g_config;
	const char *inputs[] = {
	    "[Adventure]\nchaos_mode = true\n",
	    "[Adventure]\nextra_options = true\nexpanded_options = false\n",
	    "[Adventure]\nexpanded_options = true\nextended_character_select = false\n",
	    "[Adventure]\nextra_options = true\n",
	};
	const bool initialValues[] = {false, true, false, true};
	const bool expected[] = {false, false, true, true};
	for (int i = 0; i < (int)(sizeof(inputs) / sizeof(inputs[0])); i++)
	{
		g_config = initial;
		g_config.expandedAdventureOptions = initialValues[i];
		FILE *file = fopen(path, "w");
		CTR_TEST_CHECK(file != NULL, "scratch config can be written");
		if (file == NULL)
		{
			break;
		}
		fputs(inputs[i], file);
		fclose(file);
		NativeConfig_Load();
		CTR_TEST_CHECK(g_config.expandedAdventureOptions == expected[i], "obsolete keys leave the initial value unchanged and explicit expanded_options is honored");
	}
	remove(path);
	g_config = initial;
}

static void TestRaceKartCount(void)
{
	CTR_TEST_SECTION("race kart count default and persistence");
	const NativeConfig initial = g_config;
	CTR_TEST_CHECK_EQ(g_config.raceKartCount, 8, "the default requests the retail field");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("config.ini"));
	const int values[] = {2, 12, 16};
	for (int i = 0; i < (int)len(values); i++)
	{
		g_config.raceKartCount = values[i];
		NativeConfig_Save();
		g_config.raceKartCount = initial.raceKartCount;
		CTR_TEST_CHECK(NativeConfig_Load() == CONFIG_LOAD_OK && g_config.raceKartCount == values[i],
		               "a valid kart count survives saving and loading");
	}
	remove(path);
	g_config = initial;
}

static void TestShippedPresets(void)
{
	CTR_TEST_SECTION("shipped presets");
	const NativeConfig initial = g_config;
	g_config.skipIntro = true;
	g_config.raceKartCount = 8;
	CTR_TEST_CHECK(NativeConfig_ApplyPresetFile(CTR_PRESET_SOURCE_DIR "/expanded.ini") == CONFIG_LOAD_OK,
	               "shipped Expanded preset is valid");
	CTR_TEST_CHECK(g_config.fxaa && g_config.expandedArcade && g_config.unlockNitrosOxide &&
	               g_config.botSelectionMode == BOT_SELECTION_RANDOM_UNLOCKED &&
	               g_config.bossCutscenes == BOSS_CUTSCENES_SKIPPABLE && g_config.loadingScreen == LOADING_SCREEN_FADE,
	               "Expanded preset applies the agreed options");
	CTR_TEST_CHECK(g_config.skipIntro, "shipped preset preserves options it does not list");
	CTR_TEST_CHECK(g_config.chaosMode, "Expanded enables the shared Chaos prompt");
	CTR_TEST_CHECK_EQ(g_config.raceKartCount, 16, "Expanded requests the full field");
	CTR_TEST_CHECK(NativeConfig_ApplyPresetFile(CTR_PRESET_SOURCE_DIR "/vanilla.ini") == CONFIG_LOAD_OK,
	               "shipped Vanilla preset is valid");
	CTR_TEST_CHECK(!g_config.fxaa && !g_config.expandedArcade && !g_config.unlockNitrosOxide &&
	               g_config.botSelectionMode == BOT_SELECTION_PREDETERMINED &&
	               g_config.bossCutscenes == BOSS_CUTSCENES_UNSKIPPABLE && g_config.loadingScreen == LOADING_SCREEN_VANILLA,
	               "Vanilla preset restores the corresponding options");
	CTR_TEST_CHECK(!g_config.chaosMode, "switching to Vanilla disables the shared Chaos prompt");
	CTR_TEST_CHECK_EQ(g_config.raceKartCount, 8, "Vanilla restores the default field");
	g_config = initial;
}

static void TestPresetExecutableDirectory(void)
{
	CTR_TEST_SECTION("presets beside executable with separate asset directory");
	const NativeConfig initial = g_config;
	char profilePath[512];
	NativePath_Join(profilePath, sizeof(profilePath), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("profile.ini"));
	s_testExecutableDir = CTR_PRESET_SOURCE_DIR "/../..";
	CTR_TEST_CHECK(NativeProfile_Select(CONFIG_PROFILE_EXPANDED, true) == CONFIG_LOAD_OK && g_config.fxaa,
	               "selection reads the preset beside the executable");
	CTR_TEST_CHECK(NativeProfile_LoadStartup() == PROFILE_STARTUP_READY,
	               "startup reads the preset beside the executable and the profile from the asset base");
	remove(profilePath);
	s_testExecutableDir = NULL;
	g_config = initial;
}

struct IniSaveFixture
{
	const char *value;
	bool reject;
	int *calls;
};

static bool WriteIniSaveFixture(FILE *file, const void *context)
{
	const struct IniSaveFixture *fixture = context;
	(*fixture->calls)++;
	return NativeIni_WriteValue(file, "value", fixture->value) && !fixture->reject;
}

static bool IniSaveFileEquals(const char *path, const char *expected)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		return false;
	}
	char buffer[64];
	size_t size = fread(buffer, 1, sizeof(buffer), file);
	bool matches = !ferror(file) && size == strlen(expected) && memcmp(buffer, expected, size) == 0;
	fclose(file);
	return matches;
}

static SDL_EnumerationResult SDLCALL CountIniTemporaries(void *context, const char *directory, const char *name)
{
	(void)directory;
	if (strncmp(name, "save-fixture", strlen("save-fixture")) == 0 && strstr(name, ".tmp") != NULL)
	{
		(*(int *)context)++;
	}
	return SDL_ENUM_CONTINUE;
}

static void TestIniSave(void)
{
	CTR_TEST_SECTION("shared INI saving preserves existing files on failure");
	char path[512], directory[512], missing[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("save-fixture.ini"));
	NativePath_Join(directory, sizeof(directory), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("save-fixture-directory"));
	NativePath_Join(missing, sizeof(missing), NativeStr8_FromCString(directory), NATIVE_STR8_LIT("missing/file.ini"));
	int calls = 0;
	struct IniSaveFixture fixture = {.value = "first", .calls = &calls};
	CTR_TEST_CHECK(NativeIni_Save(path, WriteIniSaveFixture, &fixture) && calls == 1 && IniSaveFileEquals(path, "value = first\n"),
	               "the save pipeline creates and closes a complete new INI file");
	FILE *exclusive = NativeExternalFile_CreateExclusive(path);
	int creationError = errno;
	if (exclusive != NULL)
	{
		fclose(exclusive);
	}
	CTR_TEST_CHECK(exclusive == NULL && creationError == EEXIST && IniSaveFileEquals(path, "value = first\n"),
	               "exclusive creation refuses to truncate an existing file on the host runtime");
	fixture.value = "second";
	CTR_TEST_CHECK(NativeIni_Save(path, WriteIniSaveFixture, &fixture) && calls == 2 && IniSaveFileEquals(path, "value = second\n"),
	               "a successful save replaces the previous complete file");
	fixture.value = "partial";
	fixture.reject = true;
	CTR_TEST_CHECK(!NativeIni_Save(path, WriteIniSaveFixture, &fixture) && calls == 3 && IniSaveFileEquals(path, "value = second\n"),
	               "a rejected serialization leaves the previous file unchanged");
	if (!SDL_CreateDirectory(directory))
	{
		CtrTest_Bail("could not create the save failure fixture");
	}
	fixture.reject = false;
	CTR_TEST_CHECK(!NativeIni_Save(missing, WriteIniSaveFixture, &fixture) && calls == 3,
	               "a temporary-file open failure does not invoke the serializer");
	SDL_PathInfo info;
	CTR_TEST_CHECK(!NativeIni_Save(directory, WriteIniSaveFixture, &fixture) && calls == 4 &&
	               SDL_GetPathInfo(directory, &info) && info.type == SDL_PATHTYPE_DIRECTORY,
	               "a failed replacement preserves the destination and reports failure");
	int temporaries = 0;
	CTR_TEST_CHECK(SDL_EnumerateDirectory(s_testBaseDir, CountIniTemporaries, &temporaries) && temporaries == 0,
	               "successful saves and both failure paths leave no sibling temporary files");
	remove(path);
	SDL_RemovePath(directory);
}

static void TestCameraPreferenceSaving(void)
{
	CTR_TEST_SECTION("legacy camera preferences use the shared save pipeline");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("camera.ini"));
	NativeCameraPreferences_SetZoom(1, 1);
	CTR_TEST_CHECK(IniSaveFileEquals(path, "0\n1\n0\n0\n"),
	               "camera saving retains the existing four-number file format");
	NativeCameraPreferences_SetZoom(3, 1);
	CTR_TEST_CHECK(IniSaveFileEquals(path, "0\n1\n0\n1\n") && NativeCameraPreferences_GetZoom(1) == 1,
	               "replacing camera preferences retains the other players' settings");
	NativeCameraPreferences_SetZoom(-1, 0);
	CTR_TEST_CHECK(IniSaveFileEquals(path, "0\n1\n0\n1\n"),
	               "an invalid camera player does not rewrite the saved preferences");
	remove(path);
}

static void TestSharedIni(void)
{
	CTR_TEST_SECTION("shared INI syntax and literal writing");
	char path[512];
	NativePath_Join(path, sizeof(path), NativeStr8_FromCString(s_testBaseDir), NATIVE_STR8_LIT("syntax.ini"));
	FILE *file = fopen(path, "w+b");
	if (file == NULL)
	{
		CtrTest_Bail("could not create the shared INI fixture");
	}
	fputs(" ; comment\r\n\t# another comment\r\n\r\n", file);
	CTR_TEST_CHECK(NativeIni_WriteSection(file, "Example") && NativeIni_WriteValue(file, "text", "a=b # literal; kept") &&
	               NativeIni_WriteInt(file, "count", -7) && NativeIni_WriteValue(file, "empty", ""),
	               "the shared writer emits sections, strings, integers and empty values");
	fputs("not a value\r\n[Example] trailing\r\n[broken\r\n[ ]\r\nlast = no newline", file);
	rewind(file);
	char buffer[256];
	struct NativeIniLine line;
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD &&
	               line.type == NATIVE_INI_SECTION && strcmp(line.section, "Example") == 0 && !line.trailingSectionText,
	               "the reader skips comments and blank CRLF lines before a section");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD &&
	               line.type == NATIVE_INI_VALUE && strcmp(line.key, "text") == 0 && strcmp(line.value, "a=b # literal; kept") == 0,
	               "values preserve additional equals signs and inline comment characters");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD &&
	               strcmp(line.key, "count") == 0 && strcmp(line.value, "-7") == 0,
	               "integers written by the shared helper retain their textual representation");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD &&
	               strcmp(line.key, "empty") == 0 && line.value[0] == '\0',
	               "empty values remain available for format-specific validation");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD && line.type == NATIVE_INI_MALFORMED,
	               "a missing separator is reported rather than silently accepted by every format");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD &&
	               line.type == NATIVE_INI_SECTION && strcmp(line.section, "Example") == 0 && line.trailingSectionText,
	               "callers can distinguish compatible section syntax from forbidden trailing text");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD &&
	               line.type == NATIVE_INI_SECTION && line.section == NULL,
	               "an unclosed section leaves recovery policy to the format owner");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD && strcmp(line.section, " ") == 0,
	               "section names are not normalized beyond the existing file syntax");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_RECORD && strcmp(line.value, "no newline") == 0,
	               "a bounded final line does not require a newline");
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, sizeof(buffer), true, &line) == NATIVE_INI_READ_END,
	               "end of file is distinct from malformed input and I/O errors");
	fclose(file);
	file = fopen(path, "w+b");
	if (file == NULL)
	{
		CtrTest_Bail("could not reopen the shared INI fixture");
	}
	fputs("key = 123456789\n", file);
	rewind(file);
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, 8, true, &line) == NATIVE_INI_READ_TOO_LONG,
	               "bounded metadata and strict config readers reject overlong lines");
	rewind(file);
	CTR_TEST_CHECK(NativeIni_Read(file, buffer, 8, false, &line) == NATIVE_INI_READ_RECORD &&
	               line.type == NATIVE_INI_VALUE && strcmp(line.key, "key") == 0 && strcmp(line.value, "1") == 0,
	               "compatible readers retain their previous fixed-buffer handling of long lines");
	fclose(file);
	remove(path);
}

int main(void)
{
	CtrTestScratch_Open(s_testBaseDir, sizeof(s_testBaseDir), "config-tests", NULL);
	TestSharedIni();
	TestIniSave();
	TestCameraPreferenceSaving();
	CTR_TEST_SECTION("native_config tests");
	TestChaosModeOption();
	TestRaceKartCount();
	TestDeveloperDefaultsAreImmutable();
	TestProfileStartup();
	TestConfigPresetPrecedence();
	TestProfileDeveloperDependencies();
	TestProfileFile();
	TestPresetFile();
	TestShippedPresets();
	TestPresetExecutableDirectory();
	TestAdventureOptionsWithoutLegacyMigration();
	CtrTestScratch_Remove("config-tests", NULL);
	return CtrTest_Summary();
}

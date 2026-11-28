#include <common.h>
#include <custom-characters/character_resources.h>
#include <game/custom-characters/character_policy.h>
#include <platform/native_renderer.h>
#include <game/DecalGlobal.h>

static struct
{
	struct Icon icon;
	char identity[sizeof(((struct CustomCharacterSelection *)0)->identity)];
	struct DecalIconTexture binding;
} s_characterIcons[CHARACTER_SELECT_EXTRA_SLOT_CAPACITY];

static void LOAD_ResetCharacterIcons(void)
{
	for (size_t index = 0; index < len(s_characterIcons); index++)
	{
		DecalGlobal_UnregisterIconTexture(&s_characterIcons[index].binding);
	}
	NativeRenderer_ClearCharacterIcons();
	memset(s_characterIcons, 0, sizeof(s_characterIcons));
}

static void LOAD_PrepareCharacterIcon(size_t slot, const char *identity, const char *pixelPath, const char *palettePath)
{
	memcpy(s_characterIcons[slot].identity, identity, sizeof(s_characterIcons[slot].identity));
	u16 pixels[NATIVE_CHARACTER_ICON_WIDTH * NATIVE_CHARACTER_ICON_HEIGHT];
	if (NativeCharacterLibrary_ReadIconFiles(pixelPath, palettePath, pixels))
	{
		u32 texture = NativeRenderer_UploadCharacterIcon(pixels);
		DecalGlobal_RegisterIconTexture(&s_characterIcons[slot].binding, &s_characterIcons[slot].icon, texture);
	}
	struct Icon *icon = &s_characterIcons[slot].icon;
	icon->texLayout.u1 = icon->texLayout.u3 = NATIVE_CHARACTER_ICON_WIDTH - 1;
	icon->texLayout.v2 = icon->texLayout.v3 = NATIVE_CHARACTER_ICON_HEIGHT - 1;
	icon->texLayout.tpage = 0x100;
}

void LOAD_RefreshCharacterIcons(void)
{
	LOAD_ResetCharacterIcons();
	size_t slot = 0;
	for (size_t index = 0; index < NativeCharacterLibrary_Count() && slot < len(s_characterIcons); index++)
	{
		const struct NativeCharacterMetadata *entry = CustomCharacter_GetMenuEntry(index);
		if (entry == NULL) { continue; }
		LOAD_PrepareCharacterIcon(slot++, entry->identity, entry->iconPixels, entry->iconClut);
	}
}

void LOAD_FinalizeCharacterIcons(void)
{
	LOAD_ResetCharacterIcons();
	size_t slot = 0;
	for (int driver = 0; driver < sdata->gGT->numPlyrCurrGame && driver < (int)len(sdata->customCharacters); driver++)
	{
		if (!CustomCharacter_IsSelected(driver)) { continue; }
		const struct CustomCharacterSelection *selection = &sdata->customCharacters[driver];
		bool prepared = false;
		for (size_t index = 0; index < slot; index++)
		{
			if (strcmp(s_characterIcons[index].identity, selection->identity) == 0) { prepared = true; }
		}
		if (!prepared && slot < len(s_characterIcons))
		{
			LOAD_PrepareCharacterIcon(slot++, selection->identity, selection->iconPixels, selection->iconClut);
		}
	}
}

static struct Icon *LOAD_ResolveCharacterIcon(int baseID, const char *identity)
{
	if (baseID < 0 || baseID >= GAME_CHARACTER_COUNT) { return NULL; }
	if (identity != NULL && identity[0] != '\0')
	{
		for (size_t index = 0; index < len(s_characterIcons); index++)
		{
			if (s_characterIcons[index].binding.texture != 0 && strcmp(s_characterIcons[index].identity, identity) == 0)
			{
				return &s_characterIcons[index].icon;
			}
		}
	}
	return sdata->gGT->ptrIcons[GAME_CHARACTER_METADATA[baseID].iconID];
}

struct Icon *LOAD_GetCharacterPreviewIcon(int baseID, const char *identity)
{
	return LOAD_ResolveCharacterIcon(baseID, identity);
}

struct Icon *LOAD_GetDriverIcon(int driverID)
{
	if (driverID < 0 || driverID >= (int)len(data.characterIDs)) { return NULL; }
	bool custom = driverID < sdata->gGT->numPlyrCurrGame && CustomCharacter_IsSelected(driverID);
	return LOAD_ResolveCharacterIcon(data.characterIDs[driverID], custom ? sdata->customCharacters[driverID].identity : NULL);
}

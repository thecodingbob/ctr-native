#ifndef CTR_CHARACTER_RESOURCES_H
#define CTR_CHARACTER_RESOURCES_H

#include <stdbool.h>
#include <stdint.h>

struct Model;
struct OtherFX;
struct Icon;

bool Bank_CustomCharacterVoice(uint16_t soundID, struct OtherFX *meta, uint16_t *spuAddr);

struct Model *LOAD_GetExtraCharacterModelByName(char *searchName);
void LOAD_FinalizeExtraCharacterModels(void);
void LOAD_ResetCharacterModelCache(void);
void LOAD_FinalizeCharacterModels(void);
struct Model *LOAD_GetDriverModel(int driverID, int baseID);
struct Model *LOAD_GetCharacterPreviewModel(int driverID, int baseID, const char *identity);
struct Model *LOAD_GetRetailCharacterModel(const char *name);

void LOAD_RefreshCharacterIcons(void);
void LOAD_FinalizeCharacterIcons(void);
struct Icon *LOAD_GetCharacterPreviewIcon(int baseID, const char *identity);
struct Icon *LOAD_GetDriverIcon(int driverID);

#endif

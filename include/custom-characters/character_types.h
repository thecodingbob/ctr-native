#ifndef CTR_CUSTOM_CHARACTER_TYPES_H
#define CTR_CUSTOM_CHARACTER_TYPES_H

#include <stdbool.h>

struct Model;

struct CustomCharacterSelection
{
	char identity[256];
	char modelPath[1024];
	char iconPixels[1024];
	char iconClut[1024];
	char audioDirectory[1024];
	int baseID;
};

struct CustomCharacterModelCache
{
	struct Model *model;
	char identity[256];
	bool attempted;
};

#endif

#ifndef CTR_DECAL_GLOBAL_H
#define CTR_DECAL_GLOBAL_H

#include <stdint.h>

struct Icon;

// Caller-owned binding. Unregister before destroying the icon or its texture.
// This does not change the packed Icon layout or take ownership of the texture.
struct DecalIconTexture
{
	const struct Icon *icon;
	uint32_t texture;
	struct DecalIconTexture *next;
};

void DecalGlobal_RegisterIconTexture(struct DecalIconTexture *binding, const struct Icon *icon, uint32_t texture);
void DecalGlobal_UnregisterIconTexture(struct DecalIconTexture *binding);
uint32_t DecalGlobal_GetIconTexture(const struct Icon *icon);

#endif

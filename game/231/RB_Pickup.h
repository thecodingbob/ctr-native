#ifndef RB_PICKUP_H
#define RB_PICKUP_H

#include <common.h>
#include <ctr_gte_transfer.h>

// NOTE(aalhendi): GTE screen coordinates are a word-aligned packed pair. The
// union exposes the signed halves without pointer casts that break native aliasing.
union RBPickupScreen
{
	u32 packed;
	s16 coords[2];
};

// Pickup HUD effects use the collecting driver's camera.
static inline void RB_Pickup_SetCamera(struct Driver *driver)
{
	MATRIX *m;

	m = &GAME_TRACKER->pushBuffer[driver->driverID].matrix_ViewProj;
	CTR_GteSetRotMatrix(m);
	CTR_GteSetTransMatrix(m);
}

#endif

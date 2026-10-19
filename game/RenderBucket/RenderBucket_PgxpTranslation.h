#ifndef RENDERBUCKET_PGXP_TRANSLATION_H
#define RENDERBUCKET_PGXP_TRANSLATION_H

#include <platform/native_pgxp.h>

// Exact counterpart of GetViewPosition + AdjustViewPositionForMvp.
// viewDepth must be the original depth, before the retail translation shifts.
static void RenderBucket_StoreMvpTranslationPrecise(struct PushBuffer *pb, struct Instance *inst, struct InstDrawPerPlayer *idpp, int viewDepth)
{
	if (!NATIVE_PGXP_ACTIVE())
	{
		return;
	}

	double view[9], unused[3], result[3], rotation[9];
	float cameraPos[3];

	NativePgxp_GetPosition(&pb->pos, &pb->pos.x, cameraPos);
	NativePgxp_GetTransform(&pb->matrix_ViewProj, &pb->matrix_ViewProj.m[0][0], pb->matrix_ViewProj.t, view, unused);

	if (inst->flags & SCREENSPACE_INSTANCE)
	{
		for (int i = 0; i < 3; i++)
		{
			result[i] = inst->matrix.t[i];
		}
	}
	else
	{
		NativePgxp_ModelViewTranslation(view, inst->matrix.t, &pb->pos.x, cameraPos, result);
	}

	// Mirror RenderBucket_AdjustViewPositionForMvp: near instances are shifted up
	// to the full-scale model matrix, and DRAW_HUGE shifts them back down.
	if ((s32)((u32)viewDepth - 0x1000u) < 0)
	{
		result[0] *= 4.0;
		result[1] *= 4.0;
		result[2] *= 4.0;
	}
	if ((inst->flags & DRAW_HUGE) != 0)
	{
		result[0] *= 0.25;
		result[1] *= 0.25;
		result[2] *= 0.25;
	}

	for (int i = 0; i < 9; i++)
	{
		rotation[i] = (&idpp->mvp.m[0][0])[i];
	}
	NativePgxp_SetTransform(&idpp->mvp, &idpp->mvp.m[0][0], idpp->mvp.t, rotation, result);
	NativePgxp_LoadTransform(&idpp->mvp, &idpp->mvp.m[0][0], idpp->mvp.t, 4);
}

#endif

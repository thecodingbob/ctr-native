#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac5a4-0x800ac638
int CS_Instance_GetNumAnimFrames(struct Instance *modelInst, int animIndex, int LOD)
{
	struct Model *model;
	struct ModelHeader *header;
	struct ModelAnim *anim;

	if (modelInst == NULL)
		return 0;

	model = modelInst->model;
	if (model == NULL)
		return 0;

	if (LOD >= model->numHeaders)
		return 0;

	header = &model->headers[LOD];
	if (header == NULL)
		return 0;

	if (animIndex >= header->numAnimations)
		return 0;

	if (header->ptrAnimations == NULL)
		return 0;

	anim = header->ptrAnimations[animIndex];
	if (anim == NULL)
		return 0;

	return (anim->numFrames & 0x7fff);
}

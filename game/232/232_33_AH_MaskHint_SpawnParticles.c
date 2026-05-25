#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b42b4-0x800b43cc.
void AH_MaskHint_SpawnParticles(s16 numParticles, struct ParticleEmitter *emSet, int maskAnim)

{
	struct Particle *particle;
	struct Instance *maskInst;
	int i, j;

	maskAnim = maskAnim + 0x1000;
	if (maskAnim > 0x3fff)
	{
		maskAnim = 0x3fff;
	}

	// "hubdustpuff"
	struct IconGroup *ig = sdata->gGT->iconGroup[0x10];

	// talking mask instance
	maskInst = sdata->instMaskHints3D;

	for (i = 0; i < numParticles; i++)
	{
		particle = Particle_Init(0, ig, emSet);
		if (particle == NULL)
			continue;

		for (j = 0; j < 3; j++)
			particle->axis[j].startVal += maskInst->matrix.t[j] * 0x100;

		particle->axis[5].startVal = (particle->axis[5].startVal * maskAnim) >> 0xc;
		particle->axis[5].velocity = (particle->axis[5].velocity * maskAnim) >> 0xc;

		particle->unk18 -= 5;
	}

	return;
}

#include <common.h>

void UI_WeaponBG_AnimateShine(void)
{
	int sine;
	u32 local_18;
	u32 local_14;
	u32 local_10;

	sine = MATH_Sin((int)sdata->wumpaShineTheta);
	sine = (sine < 0) ? -sine : sine;

	// Calculate wumpaShineColor1
	local_18 = ((sine * 0x7f) >> 0xc) + 0x7f;
	sdata->wumpaShineColor1[0][0] = local_18;
	sdata->wumpaShineColor1[0][1] = local_18;
	*(s16 *)&sdata->wumpaShineColor1[0][2] = 0;

	local_14 = ((sine * 0x32) >> 0xc) + 0x32;
	sdata->wumpaShineColor1[1][0] = local_18;
	sdata->wumpaShineColor1[1][1] = local_14;
	*(s16 *)&sdata->wumpaShineColor1[1][2] = 0;

	sdata->wumpaShineColor1[2][0] = ((sine * 0x21) >> 0xc) + 0x21;
	sdata->wumpaShineColor1[2][1] = ((sine * 0x10) >> 0xc) + 0x10;
	*(s16 *)&sdata->wumpaShineColor1[2][2] = 0;

	// Calculate wumpaShineColor2
	local_10 = ((sine * 0x5f) >> 0xc) + 0x5f;
	sdata->wumpaShineColor2[0][0] = local_10;
	sdata->wumpaShineColor2[0][1] = local_10;
	*(s16 *)&sdata->wumpaShineColor2[0][2] = local_10;

	local_10 = *(int *)&sdata->wumpaShineColor2[0][0];
	*(int *)&sdata->wumpaShineColor2[1][0] = local_10;
	*(int *)&sdata->wumpaShineColor2[2][0] = local_10;

	sdata->wumpaShineResult = (sine * 0xff >> 0xd) + 0x80;
	return;
}

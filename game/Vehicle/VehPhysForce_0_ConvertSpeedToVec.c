#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005e104-0x8005e214
void VehPhysForce_ConvertSpeedToVec(struct Driver *driver, Vec3 *vel)
{
	int yAngle = driver->axisRotationY;
	int ySine = MATH_Sin(yAngle);
	int yCos = MATH_Cos(yAngle);
	int yComponent = FP_MULT(driver->speed, yCos);

	int xAngle = driver->axisRotationX;
	int xCos = MATH_Cos(xAngle);
	int xSine = MATH_Sin(xAngle);

	vel->x = FP_MULT(yComponent, xSine);
	vel->y = FP_MULT(driver->speed, ySine);
	vel->z = FP_MULT(yComponent, xCos);
}

void DECOMP_VehPhysForce_ConvertSpeedToVec(struct Driver *driver)
{
	VehPhysForce_ConvertSpeedToVec(driver, &driver->velocity);
}

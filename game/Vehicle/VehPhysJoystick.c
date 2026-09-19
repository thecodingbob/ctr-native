#include <common.h>

enum
{
	RACING_WHEEL_DEFAULT_CENTER = 0x80,
	RACING_WHEEL_DEFAULT_DEAD_ZONE = 0x30,
	RACING_WHEEL_DEFAULT_RANGE = 0x7f,
	// NOTE(aalhendi): Retail's null-wheel strength path uses 0x5e directly,
	// not default range minus default dead zone.
	RACING_WHEEL_DEFAULT_STRENGTH_DISTANCE = 0x5e,
	JOYSTICK_STRENGTH_CURVE_SEGMENTS = 5,
};


int VehPhysJoystick_ReturnToRest(int stickVal, int half, struct RacingWheelData *rwd)
{
	register int outputHalf CTR_PSX_REGISTER("$3");
	int center;
	int deadZone;
	int range;

	outputHalf = half;

	if (rwd != 0)
	{
		center = rwd->gamepadCenter;
		deadZone = rwd->deadZone;
		range = rwd->range;
	}
	else
	{
		center = RACING_WHEEL_DEFAULT_CENTER;
		deadZone = RACING_WHEEL_DEFAULT_DEAD_ZONE;
		range = RACING_WHEEL_DEFAULT_RANGE;
	}

	stickVal = CTR_MipsSubLo(stickVal, center);
	if (stickVal < 0)
	{
		return CTR_MipsNegLo(VehCalc_MapToRange(CTR_MipsNegLo(stickVal), deadZone, range, 0, outputHalf));
	}

	return VehCalc_MapToRange(stickVal, deadZone, range, 0, outputHalf);
}

int VehPhysJoystick_GetStrength(int val, int max, struct RacingWheelData *rwd)
{
	int dead;
	int range;
	int dist;
	int halfDist;

	if (rwd != 0)
	{
		dead = rwd->deadZone;
		range = rwd->range;
		dist = CTR_MipsSubLo(range, dead);
	}
	else
	{
		dead = RACING_WHEEL_DEFAULT_DEAD_ZONE;
		range = RACING_WHEEL_DEFAULT_RANGE;
		dist = RACING_WHEEL_DEFAULT_STRENGTH_DISTANCE;
	}

	if (val < dead)
	{
		return 0;
	}

	if (range <= val)
	{
		return max;
	}

	val = CTR_MipsSubLo(val, dead);
	halfDist = CTR_MipsSra(CTR_MipsAddLo(dist, (u32)dist >> 31), 1);

	if (val < halfDist)
	{
		int product;

		max = max / JOYSTICK_STRENGTH_CURVE_SEGMENTS;
		product = CTR_MipsMulLo(val, max);
		product = CTR_MipsSll(product, 1);
		return CTR_MipsDiv(product, dist);
	}

	{
		int maxFifth = max / JOYSTICK_STRENGTH_CURVE_SEGMENTS;
		int product;

		val = CTR_MipsSubLo(val, halfDist);
		max = CTR_MipsSubLo(max, maxFifth);
		product = CTR_MipsMulLo(val, max);
		product = CTR_MipsSll(product, 1);
		return CTR_MipsAddLo(CTR_MipsDiv(product, dist), maxFifth);
	}
}

int VehPhysJoystick_GetStrengthAbsolute(int stickVal, int maxSteer, struct RacingWheelData *rwd)
{
	register int center CTR_PSX_REGISTER("$2") = RACING_WHEEL_DEFAULT_CENTER;
	int distFromCenter;

	if (rwd != NULL)
	{
		center = rwd->gamepadCenter;
	}
	distFromCenter = CTR_MipsSubLo(stickVal, center);

	// if steering right
	if (distFromCenter < 0)
	{
		return CTR_MipsNegLo(VehPhysJoystick_GetStrength(CTR_MipsNegLo(distFromCenter), maxSteer, rwd));
	}

	// steer left
	return VehPhysJoystick_GetStrength(distFromCenter, maxSteer, rwd);
}

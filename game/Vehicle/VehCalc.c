#include <common.h>

enum
{
	VEH_CALC_MIPS_SHIFT_MASK = 0x1f,
	VEH_CALC_FAST_SQRT_ADD_BIT_BIAS = 1,
	VEH_CALC_FAST_SQRT_ROOT_STEP_SHIFT = 2,
};


int VehCalc_InterpBySpeed(int val, int speed, int desired)
{
	if (val > desired)
	{
		val = CTR_MipsSubLo(val, speed);

		if (val < desired)
		{
			return desired;
		}
	}

	else if (val < desired)
	{
		val = CTR_MipsAddLo(val, speed);

		if (val > desired)
		{
			return desired;
		}
	}

	return val;
}

int VehCalc_MapToRange(int val, int oldMin, int oldMax, int newMin, int newMax)
{
	int distFromBottom;
	int newRange;
	int oldRange;
	int scaledDistance;

	if (val <= oldMin)
	{
		return newMin;
	}

	if (val >= oldMax)
	{
		return newMax;
	}

	distFromBottom = CTR_MipsSubLo(val, oldMin);
	newRange = CTR_MipsSubLo(newMax, newMin);
	oldRange = CTR_MipsSubLo(oldMax, oldMin);
	scaledDistance = CTR_MipsDiv(CTR_MipsMulLo(distFromBottom, newRange), oldRange);

	return CTR_MipsAddLo(newMin, scaledDistance);
}

int VehCalc_SteerAccel(int steeringFrameCount, int stage2FirstFrame, int stage2FrameLength, int stage4FirstFrame, int stage1MinSteer, int stage1MaxSteer)
{
	// Steering Stage 1,
	// if first 4 frames of steering
	// increase steer acceleration as time passes
	if (steeringFrameCount < stage2FirstFrame)
	{
		// map "frame" from [0,4] -> [0x800,0xC00]

		int steerAccel = VehCalc_MapToRange(steeringFrameCount, 0, stage2FirstFrame, stage1MinSteer, stage1MaxSteer);

		CTR_PSX_CLOBBER("$1");
		return steerAccel;
	}

	else
	{
		// Steering Stage 3
		// frames 12+
		// decrease steer acceleration as time passes
		if (CTR_MipsAddLo(stage2FirstFrame, stage2FrameLength) < steeringFrameCount)
		{
			// map "frame" from [12,64] -> [0xC00,0]

			return VehCalc_MapToRange(steeringFrameCount, CTR_MipsAddLo(stage2FirstFrame, stage2FrameLength), stage4FirstFrame, stage1MaxSteer, 0);
		}
	}

	// Steering Stage 2,
	// next 0x8 frames (frame 4 to 12)
	// max steer accel of 0xC00
	return stage1MaxSteer;

	// Steering Stage 4,
	// part of Stage 3's mapping,
	// for all steering after frame 64,
	// steer acceleration is zero, so
	// angular velocity is constant
}

u32 VehCalc_FastSqrt(u32 n, u32 shift)
{
	u32 result;
	s32 rootBitIndex;
	u32 lastApproximation;
	u32 testBit;
	u32 addBit;

	result = 0;
	testBit = (u32)CTR_MipsSll(1, shift);
	rootBitIndex = 1;
	lastApproximation = result;

	while ((testBit < n) && ((testBit << VEH_CALC_FAST_SQRT_ROOT_STEP_SHIFT) != 0))
	{
		rootBitIndex++;
		testBit <<= VEH_CALC_FAST_SQRT_ROOT_STEP_SHIFT;
	}

	addBit = (u32)CTR_MipsSll(1, CTR_MipsAddLo(rootBitIndex, CTR_MipsSubLo(shift, VEH_CALC_FAST_SQRT_ADD_BIT_BIAS)));

	while (testBit != 0)
	{
		u32 shiftedResult;
		u32 approximation;

		shiftedResult = (u32)CTR_MipsSll(result, (u32)rootBitIndex);

		if (rootBitIndex < 0)
		{
			shiftedResult = CTR_MipsSrl(result, (u32)CTR_MipsNegLo(rootBitIndex));
		}

		approximation = shiftedResult + (lastApproximation + testBit);

		if (approximation <= n)
		{
			lastApproximation = approximation;
			result += addBit;
		}

		addBit >>= 1;
		testBit >>= VEH_CALC_FAST_SQRT_ROOT_STEP_SHIFT;
		rootBitIndex--;
	}

	return result;
}

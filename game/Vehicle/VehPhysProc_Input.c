#include <common.h>

enum
{
	VEH_PHYS_PROC_JUMP_BUTTON_MASK = BTN_R1 | BTN_L1,
	VEH_PHYS_PROC_DEFAULT_DRIFT_BUTTON = BTN_R1,
	VEH_PHYS_PROC_JUMP_BUFFER_FRAMES = 10,
	VEH_PHYS_PROC_STICK_CENTER = 0x80,
};

static u32 VehPhysProc_JumpInput(struct Driver *driver, u32 tapped, u32 held, int rainCloudEffect, u32 actions)
{
	const u32 jumpTapped = tapped & VEH_PHYS_PROC_JUMP_BUTTON_MASK;
	if (jumpTapped != 0 && driver->kartState != KS_DRIFTING)
	{
		driver->buttonUsedToStartDrift = jumpTapped == VEH_PHYS_PROC_JUMP_BUTTON_MASK
		                                    ? VEH_PHYS_PROC_DEFAULT_DRIFT_BUTTON : jumpTapped;
		if (rainCloudEffect != RAIN_CLOUD_EFFECT_JUMP_LOCKOUT)
		{
			driver->jump_TenBuffer = VEH_PHYS_PROC_JUMP_BUFFER_FRAMES;
			return actions | ACTION_JUMP_BUTTON_HELD;
		}
		return actions;
	}
	if ((held & VEH_PHYS_PROC_JUMP_BUTTON_MASK) != 0 && rainCloudEffect != RAIN_CLOUD_EFFECT_JUMP_LOCKOUT)
	{
		if ((actions & ACTION_JUMP_BUTTON_HELD) == 0)
		{
			driver->jump_TenBuffer = VEH_PHYS_PROC_JUMP_BUFFER_FRAMES;
		}
		return actions | ACTION_JUMP_BUTTON_HELD;
	}
	if (driver->jump_TenBuffer > 0)
	{
		driver->jump_TenBuffer = 0;
	}
	return actions & ~ACTION_JUMP_BUTTON_HELD;
}

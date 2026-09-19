#include <common.h>

void RB_Turtle_ThTick(struct Thread *t)
{
	struct Turtle *turtleObj = t->object;
	struct Instance *turtleInst = t->inst;

	if (turtleObj->direction != TURTLE_DIRECTION_RISING)
	{
		if (turtleObj->timer < 0x3c0)
		{
			turtleObj->timer += GAME_TRACKER->elapsedTimeMS;
			if (turtleObj->timer > 0x5a0)
			{
				turtleObj->timer = 0x5a0;
			}
		}
		else if (turtleInst->animFrame + 1 < INSTANCE_GetNumAnimFrames(turtleInst, 0))
		{
			turtleInst->animFrame++;
		}
		else
		{
			turtleObj->direction = TURTLE_DIRECTION_RISING;
			turtleObj->timer = 0;
			turtleObj->state = TURTLE_STATE_FULLY_DOWN;
		}
	}
	else
	{
		if (turtleObj->timer < 0x3c0)
		{
			turtleObj->timer += GAME_TRACKER->elapsedTimeMS;
			if (turtleObj->timer > 0x5a0)
			{
				turtleObj->timer = 0x5a0;
			}
			if (turtleObj->timer == 0x5a0)
			{
				PlaySound3D(0x7d, turtleInst);
			}
		}
		else
		{
			// NOTE(aalhendi): Raising the shell changes jump height before the animation ends.
			turtleObj->state = TURTLE_STATE_NOT_FULLY_DOWN;
			if (turtleInst->animFrame - 1 > 0)
			{
				turtleInst->animFrame--;
			}
			else
			{
				turtleObj->direction = TURTLE_DIRECTION_FALLING;
				turtleObj->timer = 0;
			}
		}
	}
}

int RB_Turtle_LInC(struct Instance *inst, struct Thread *driverTh, struct ScratchpadStruct *sps)
{
	s32 speed;
	s32 jumpType;
	struct Driver *driver;
	struct Turtle *turtleObj;
	(void)sps;


	driver = driverTh->object;
	turtleObj = inst->thread->object;

	speed = abs(driver->speedApprox);

	if (speed > 0x1400)
	{
		// small jump
		jumpType = FORCED_JUMP_LOW;

		if (turtleObj->state != TURTLE_STATE_FULLY_DOWN)
		{
			// big jump
			jumpType = FORCED_JUMP_HIGH;
		}

		// make the player jump
		driver->forcedJumpType = jumpType;

		return 2;
	}

	return 1;
}

void RB_Turtle_LInB(struct Instance *inst)
{
	s32 turtleID;
	struct Thread *t;
	struct Turtle *turtleObj;

	inst->flags |= SPLIT_LINE;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Turtle), NONE, SMALL, STATIC),

	    RB_Turtle_ThTick, // behavior
	    "turtle",         // debug name
	    0                 // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	turtleObj = t->object;
	t->inst = inst;

	inst->scale.x = 0x1000;
	inst->scale.y = 0x1000;
	inst->scale.z = 0x1000;

	turtleID = inst->name[strlen(inst->name) - 1] - '0';

	turtleObj->turtleID = turtleID;
	turtleObj->direction = TURTLE_DIRECTION_FALLING;
	turtleObj->timer = 0;
	inst->animFrame = 0;

	// put turtles on different cycles, based on turtleID
	if ((turtleObj->turtleID & 1) != 0)
	{
		// fully down
		turtleObj->direction = TURTLE_DIRECTION_RISING;
		turtleObj->state = TURTLE_STATE_FULLY_DOWN;
		inst->animFrame = INSTANCE_GetNumAnimFrames(inst, 0);
	}
	else
	{
		// fully up
		turtleObj->direction = TURTLE_DIRECTION_FALLING;
		turtleObj->state = TURTLE_STATE_NOT_FULLY_DOWN;
	}
}

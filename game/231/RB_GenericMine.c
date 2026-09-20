#include <common.h>

void RB_GenericMine_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct GameTracker *gGT;
	struct MineWeapon *mw;

	RB_Default_LInB(inst);

	if (inst->thread != NULL)
	{
		return;
	}

	// if loaded Rocky Road or Nitro Court
	// from outside crystal challenge, skip boxes
	if ((GAME_TRACKER->gameMode1 & CRYSTAL_CHALLENGE) == 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct MineWeapon), NONE, SMALL, MINE),

	    RB_GenericMine_ThTick, // behavior
	    rb_nameNitro,          // debug name
	    0                      // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	t->inst = inst;

	// Retail parents crystal-challenge level mines to driver 0.
	inst->thread->funcThCollide = (void *)RB_Hazard_ThCollide_Generic;
	gGT = GAME_TRACKER;
	inst->thread->parentThread = gGT->drivers[0]->instSelf->thread;
	inst->thread->modelIndex = inst->model->id;

	mw = inst->thread->object;
	mw->instParent = gGT->drivers[0]->instSelf;
	mw->velocity.x = 0;
	mw->velocity.y = 0;
	mw->velocity.z = 0;
	mw->parentSafetyFrames = 0;
	mw->boolDestroyed = 0;
	mw->tntSpinY = 0;
	mw->driverTarget = NULL;
	mw->crateInst = NULL;
	mw->flags = 0;

	RB_MinePool_Add(mw);
	mw->stopFallAtY = inst->matrix.t[1];
}

void RB_GenericMine_ThTick(struct Thread *t)
{
	struct GameTracker *gGT;
	struct Instance *inst;
	struct Instance *coll;
	struct MineWeapon *mw;
	struct Driver *d;
	struct Crate *crate;
	struct Instance *instCrate;
	struct MineWeapon *tnt;
	u16 model;
	inst = t->inst;
	mw = inst->thread->object;

	// if weapon is "thrown" like Komodo Joe
	if ((mw->flags & MINE_WEAPON_FLAG_THROWN) != 0)
	{
		if (inst->model->id != STATIC_CRATE_TNT)
		{
			mw->cooldown = 0xf0;
			ThTick_SetAndExec(t, RB_Potion_ThTick_InAir);
		}
		else
		{
			// set scale (x, y, z)
			inst->scale.x = 0x800;
			inst->scale.y = 0x800;
			inst->scale.z = 0x800;
			ThTick_SetAndExec(t, RB_TNT_ThTick_ThrowOffHead);
		}

#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native transitions can return; retail switches out of this tick.
		return;
#endif
	}

	// === If not "thrown" ===

	// reduce cooldown
	mw->cooldown -= GAME_TRACKER->elapsedTimeMS;

	if (mw->cooldown < 0)
	{
		mw->cooldown = 0;
	}

	// if animation is not over
	if (inst->animFrame + 1 < INSTANCE_GetNumAnimFrames(inst, 0))
	{
		// increment animation frame
		inst->animFrame++;
	}
	// if animation is over
	else
	{
		// restart animation
		inst->animFrame = 0;
	}

	// increment posY by velY * time
	// do NOT use parenthesis
	inst->matrix.t[1] += (mw->velocity.y * GAME_TRACKER->elapsedTimeMS) >> 5;

	if (inst->matrix.t[1] < mw->stopFallAtY)
	{
		inst->matrix.t[1] = mw->stopFallAtY;
	}

	// decrease velocity by time, this is artificial gravity (negative acceleration)
	mw->velocity.y -= ((GAME_TRACKER->elapsedTimeMS << 2) >> 5);

	// terminal velocity
	if (mw->velocity.y < -0x60)
	{
		mw->velocity.y = -0x60;
	}

	// If scale is not big enough
	if (inst->scale.x < 0x1000)
	{
		// make scale larger each frame
		inst->scale.x += 0x200;
		inst->scale.y += 0x200;
		inst->scale.z += 0x200;
	}
	else
	{
		inst->scale.x = 0x1000;
		inst->scale.y = 0x1000;
		inst->scale.z = 0x1000;
	}

	coll = RB_Hazard_CollideWithDrivers(inst, mw->parentSafetyFrames, (u32)(u16)inst->model->id - STATIC_BEAKER_RED < 2 ? 0x1900 : 0x3840, mw->instParent);

	// if no collision
	if (coll == 0)
	{
		goto checkPoolState;
	}

	// get driver who hit tnt (or nitro)
	// from the object attached to thread
	d = coll->thread->object;

	if (((mw->crateInst != 0) && (mw->crateInst->thread != 0)) && (mw->crateInst->thread->object != 0))
	{
		crate = mw->crateInst->thread->object;
		crate->boolPauseCooldown = 0;
	}

	// red beaker or green beaker
	model = inst->model->id;
	if ((u32)model - STATIC_BEAKER_RED < 2)
	{
		// if collision, and if this was a red potion
		if (RB_Hazard_HurtDriver(d, 1, mw->instParent->thread->object, mw->cooldown != 0 ? 4 : 2) != 0 && (mw->flags & MINE_WEAPON_FLAG_RED_BEAKER) != 0)
		{
			RB_RainCloud_Init(d);
		}

		// if this driver is not an AI
		if ((d->actionsFlagSet & ACTION_BOT) == 0)
		{
			gGT = GAME_TRACKER;
			// current fade value (bright white)
			gGT->pushBuffer[d->driverID].fadeFromBlack_currentValue = 0x1fff;

			// desired fade value (neutral)
			gGT->pushBuffer[d->driverID].fadeFromBlack_desiredResult = 0x1000;

			// fade step
			gGT->pushBuffer[d->driverID].fade_step = 0xff78;
		}

		// Negative time colors the icon green; positive time colors it red.
		if (inst->model->id == STATIC_BEAKER_GREEN)
		{
			d->damageColorTimer = -0x1e;
		}
		else
		{
			d->damageColorTimer = 0x1e;
		}
		RB_GenericMine_ThDestroy(t, inst, mw);
	}

	// TNT/Nitro
	else
	{
		// if driver->instTntRecv is valid
		if (d->instTntRecv != NULL)
		{
			struct Instance *attached;
			// blasted driver
			RB_Hazard_HurtDriver(d, 2, 0, 2);

			// icon damage timer, draw icon as red
			d->damageColorTimer = 0x1e;

			// set scale (x, y, z) to zero
			attached = d->instTntRecv;
			attached->scale.z = 0;
			attached->scale.y = 0;
			attached->scale.x = 0;

			d->instTntRecv->flags |= HIDE_MODEL;

			// this thread is now dead
			d->instTntRecv->thread->flags |= THREAD_FLAG_DEAD;

			// erase instTntRecv
			d->instTntRecv = 0;

			RB_GenericMine_ThDestroy(t, inst, mw);
			goto checkPoolState;
		}

		// if driver has squished timer
		if (d->squishTimer != 0)
		{
			RB_Hazard_HurtDriver(d, 2, 0, 2);

			d->damageColorTimer = 0x1e;
			RB_GenericMine_ThDestroy(t, inst, mw);
			goto checkPoolState;
		}

		// if model is Nitro
		if ((s16)model == PU_EXPLOSIVE_CRATE)
		{
			RB_Hazard_HurtDriver(d, 2, mw->instParent->thread->object, 2);

			// icon damage timer, draw icon as green
			d->damageColorTimer = -0x1e;

			RB_GenericMine_ThDestroy(t, inst, mw);
			goto checkPoolState;
		}

		// if model is TNT
		if ((s16)model == STATIC_CRATE_TNT)
		{
			// damageType 0 keeps driving unless the shield/mask path absorbs TNT.
			if (RB_Hazard_HurtDriver(d, 0, mw->instParent->thread->object, 2) != 0)
			{
				// if Instance has no InstDef,
				// if this TNT is not part of the level,
				// use existing thread
				if (inst->instDef == 0)
				{
					// icon damage timer, draw icon as red
					d->damageColorTimer = 0x1e;

					// give driver to tnt object
					mw->driverTarget = d;

					// driver -> instTntRecv
					d->instTntRecv = inst;

					RB_MinePool_Remove(mw);

					// play Hit TNT "bounce" sound
					PlaySound3D(0x50, inst);

					inst->compressedNormalAndDriverIndex = 0;
					inst->flags |= PIXEL_LOD;
					mw->velocity.y = 0x30;
					mw->velocity.x = 0;
					mw->velocity.z = 0;
					mw->deltaPos.x = 0;
					mw->deltaPos.y = 0;
					mw->deltaPos.z = 0;
					mw->stopFallAtY = 0x3fff;

					ThTick_SetAndExec(t, RB_TNT_ThTick_ThrowOnHead);
#ifdef CTR_NATIVE
					// NOTE(aalhendi): Do not resume this tick after a native transition.
					return;
#endif
				}

				// if this TNT has an InstDef, then it is part of LEV,
				// hide the level instance and birth a carried TNT instance
				else
				{
					// create thread for TNT, get an Instance
					instCrate = INSTANCE_BirthWithThread(STATIC_CRATE_TNT, rb_nameTnt, SMALL, MINE, RB_GenericMine_ThTick, sizeof(struct MineWeapon), 0);

					instCrate->matrix = inst->matrix;

					instCrate->thread->funcThDestroy = PROC_DestroyInstance;

					instCrate->thread->funcThCollide = (void *)RB_Hazard_ThCollide_Generic;

					// Get object from thread
					tnt = instCrate->thread->object;

					tnt->instParent = d->instSelf;

					tnt->parentSafetyFrames = 10;
					tnt->boolDestroyed = 0;
					tnt->tntSpinY = 0;
					tnt->crateInst = 0;
					tnt->flags = 0;

					// give driver to tnt object
					// stopFallAtY (where it explodes)
					tnt->stopFallAtY = inst->matrix.t[1];
					tnt->driverTarget = d;

					// driver -> instTntRecv
					d->instTntRecv = instCrate;

					// TNT bounce sound
					PlaySound3D(0x50, instCrate);

					instCrate->compressedNormalAndDriverIndex = 0;
					instCrate->flags |= PIXEL_LOD;
					tnt->velocity.y = 0x30;
					tnt->velocity.x = 0;
					tnt->velocity.z = 0;
					tnt->deltaPos.x = 0;
					tnt->deltaPos.y = 0;
					tnt->deltaPos.z = 0;
					instCrate->thread->funcThTick = RB_TNT_ThTick_ThrowOnHead;

					RB_MinePool_Remove(mw);

					// set scale (x, y, z) to zero
					inst->scale.z = 0;
					inst->scale.y = 0;
					inst->scale.x = 0;

					// make invisible
					inst->flags |= HIDE_MODEL;

					// this thread is now dead
					t->flags |= THREAD_FLAG_DEAD;
				}
			}
			else
			{
				RB_GenericMine_ThDestroy(t, inst, mw);
			}
		}
	}
checkPoolState:

	if (mw->parentSafetyFrames != 0)
	{
		mw->parentSafetyFrames--;
	}

	// if mineWeapon->boolDestroyed == 0
	if (mw->boolDestroyed == 0)
	{
		return;
	}

	// if thread is dead, quit function
	// this is if GenericMine_ThDestroy already ran
	if ((t->flags & THREAD_FLAG_DEAD) != 0)
	{
		return;
	}

	// === If destroyed from MinePool overflow ===

	// instance -> model -> modelID
	{
		s16 overflowModel = inst->model->id;

		if (overflowModel == PU_EXPLOSIVE_CRATE)
		{
			// glass shatter sound
			PlaySound3D(0x3f, inst);

			RB_Blowup_Init(inst);
		}
		else if (overflowModel == STATIC_CRATE_TNT)
		{
			// tnt explosion sound
			PlaySound3D(0x3d, inst);

			RB_Blowup_Init(inst);
		}
		else if ((u16)(overflowModel - STATIC_BEAKER_RED) < 2)
		{
			// glass shatter sound
			PlaySound3D(0x3f, inst);

			RB_Explosion_InitPotion(inst);
		}
	}

	// this thread is now dead
	t->flags |= THREAD_FLAG_DEAD;

	return;
}

void RB_GenericMine_ThDestroy(struct Thread *t, struct Instance *inst, struct MineWeapon *mw)
{
	u32 model;
	u16 param;

	model = inst->model->id;

	if (model == PU_EXPLOSIVE_CRATE)
	{
		// glass shatter
		param = 0x3f;

		PlaySound3D(param, inst);

		RB_Blowup_Init(inst);
	}
	else if (model == STATIC_CRATE_TNT)
	{
		// tnt explosion sound
		param = 0x3d;

		PlaySound3D(param, inst);

		RB_Blowup_Init(inst);
	}
	else
	{
		// play sound of glass shatter
		PlaySound3D(0x3f, inst);

		RB_Explosion_InitPotion(inst);
	}

	// Set scale (x, y, z) to zero
	inst->scale.z = 0;
	inst->scale.y = 0;
	inst->scale.x = 0;

	// make invisible
	inst->flags |= HIDE_MODEL;

	RB_MinePool_Remove(mw);

	// this thread is now dead
	t->flags |= THREAD_FLAG_DEAD;

	return;
}

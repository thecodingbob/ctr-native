#include <common.h>
#include <ctr_gte_transfer.h>

#ifndef RB_WARPBALL_FADE_Y
#define RB_WARPBALL_FADE_Y R231.warpballFadeY
#endif

#ifndef RB_WARPBALL_PARTICLE_HEIGHT
#define RB_WARPBALL_PARTICLE_HEIGHT R231.warpballParticleHeight
#endif

void RB_Warpball_FadeAway(struct Thread *t)
{
	s16 frameId;
	struct TrackerWeapon *tw;
	struct Instance *inst;
	struct Driver *d;
	const s16(*scale)[3];

	tw = t->object;
	inst = t->inst;
	if ((u32)tw->fadeFrame >= 6)
	{
		d = tw->driverTarget;

		if (d != NULL)
		{
			// remove 2D square-target being drawn on the player's screen
			d->actionsFlagSet &= ~ACTION_TRACKER_TARGETED;
		}

		// remove active warpball flag
		GAME_TRACKER->gameMode1 &= ~WARPBALL_HELD;

		// This thread is now dead
		t->flags |= THREAD_FLAG_DEAD;
		return;
	}

	// set scale (x, y, z)
	frameId = tw->fadeFrame;
	scale = R231.warpballFadeScale;
	inst->scale.x = scale[frameId][0];
	inst->scale.y = scale[frameId][1];
	inst->scale.z = scale[frameId][2];

	inst->matrix.t[1] = CTR_MipsAddLo(tw->distFromGround, RB_WARPBALL_FADE_Y[frameId]);

	tw->fadeFrame = CTR_MipsAddLo(tw->fadeFrame, 1);

	return;
}

void RB_Warpball_Death(struct Thread *t)
{
	struct Instance *inst;
	struct TrackerWeapon *tw;

	inst = t->inst;
	tw = t->object;
	tw->distFromGround = inst->matrix.t[1];
	tw->ptrParticle->framesLeftInLife = 0;
	tw->fadeFrame = 0;

	// play sound of warpball death
	PlaySound3D(0x4f, inst);

	// stop audio of moving
	OtherFX_RecycleMute(&tw->soundIDCount);

	ThTick_SetAndExec(t, &RB_Warpball_FadeAway);
	return;
}

struct CheckpointNode *RB_Warpball_NewPathNode(struct CheckpointNode *cn, struct Driver *d)
{
	struct CheckpointNode *currNode;
	struct CheckpointNode *nodes;
	b16 foundLeftPath;
	s16 i;

	foundLeftPath = 0;
	if (d != NULL)
	{
		currNode = cn;
		if (d->checkpoint.branchChoiceIndex == cn->nextIndex_left)
		{
			return &GAME_TRACKER->level1->ptr_restart_points[cn->nextIndex_left];
		}
		if (cn->nextIndex_left != 0xff)
		{
			nodes = GAME_TRACKER->level1->ptr_restart_points;
			for (i = 0; i < 3; i++)
			{
				currNode = currNode->nextIndex_left == 0xff ? &nodes[currNode->nextIndex_forward] : &nodes[currNode->nextIndex_left];
				if (d->checkpoint.branchChoiceIndex == currNode->nextIndex_forward)
				{
					foundLeftPath = 1;
					break;
				}
			}
		}
		if (foundLeftPath)
		{
			return &GAME_TRACKER->level1->ptr_restart_points[cn->nextIndex_left];
		}
	}
	return &GAME_TRACKER->level1->ptr_restart_points[cn->nextIndex_forward];
}

void RB_Warpball_Start(struct TrackerWeapon *tw)
{
	struct CheckpointNode *node;
	s16 i;

	node = tw->ptrNodeCurr;
	// NOTE(aalhendi): Retail retains a halfword lookahead loop for this single step.
	for (i = 0; i < 1; i++)
	{
		node = RB_Warpball_NewPathNode(node, tw->driverTarget);
	}
	tw->ptrNodeCurr = node;
	tw->ptrNodeNext = RB_Warpball_NewPathNode(tw->ptrNodeCurr, tw->driverTarget);
	return;
}

struct Driver *RB_Warpball_GetDriverTarget(struct TrackerWeapon *tw, struct Instance *inst)
{
	struct Driver *bestDriver;
	struct CheckpointNode *nodes;
	struct CheckpointNode *node1;
	struct CheckpointNode *node2;
	SVec3 orbVector;
	SVec3 pathVector;
	s32 trackDistance, projectedDistance, distanceAlongTrack, bestDistance;
	s32 unusedMAC2;
	s16 i;

	bestDriver = NULL;
	if (tw->flags & TRACKER_FLAG_POWERED_UP)
	{
		nodes = GAME_TRACKER->level1->ptr_restart_points;
		node1 = &nodes[tw->ptrNodeCurr->nextIndex_forward];
		node2 = &nodes[node1->nextIndex_forward];
		trackDistance = nodes[0].distToFinish << 3;
		pathVector.x = node1->pos.x - node2->pos.x;
		pathVector.y = node1->pos.y - node2->pos.y;
		pathVector.z = node1->pos.z - node2->pos.z;
		MATH_VectorNormalize(&pathVector);

		orbVector.x = (u16)inst->matrix.t[0] - (u16)node1->pos.x;
		orbVector.y = (u16)inst->matrix.t[1] - (u16)node1->pos.y;
		orbVector.z = (u16)inst->matrix.t[2] - (u16)node1->pos.z;
		CTR_GteLoadDotProduct(&pathVector, &orbVector);
		CTR_GteLoadDelay();
		gte_mvmva(0, 0, 0, 3, 0);
		CTR_GteReadMAC12(projectedDistance, unusedMAC2);
		(void)unusedMAC2;
		projectedDistance >>= 12;
		bestDistance = 0x7fffffff;
		distanceAlongTrack = node1->distToFinish << 3;
		distanceAlongTrack += projectedDistance;
		distanceAlongTrack += 0x200;
		distanceAlongTrack %= trackDistance;

		for (i = 0; i < 8; i++)
		{
			struct Driver *driver = GAME_TRACKER->drivers[i];
			if (driver != NULL && !(tw->driversHit & (1u << i)) && !(driver->actionsFlagSet & ACTION_RACE_FINISHED) && driver->kartState != KS_MASK_GRABBED)
			{
				s32 distance = CTR_MipsSubLo(distanceAlongTrack, driver->distanceToFinish_curr);
				if (distance < 0)
				{
					distance = CTR_MipsAddLo(distance, trackDistance);
				}
				if (distance < bestDistance)
				{
					bestDistance = distance;
					bestDriver = driver;
				}
			}
		}
	}
	else
	{
		s16 rank;
		for (rank = 0; rank < 8; rank++)
		{
			struct Driver *driver = GAME_TRACKER->driversInRaceOrder[rank];
			if (driver != NULL && driver != tw->driverParent && !(driver->actionsFlagSet & ACTION_RACE_FINISHED))
			{
				bestDriver = driver;
				break;
			}
		}
	}
	return bestDriver;
}

void RB_Warpball_SetTargetDriver(struct TrackerWeapon *tw)
{
	struct CheckpointNode *targetNode;
	struct CheckpointNode *forwardTarget;
	struct CheckpointNode *backwardTarget;
	struct CheckpointNode *prevNode;
	struct CheckpointNode *pathNode;
	struct CheckpointNode *pathStarts[2];
	struct Driver *target;
	s32 targetDistance;
	s16 i, j, forwardStep;

	target = tw->driverTarget;
	if (target == NULL)
	{
		return;
	}
	targetDistance = target->distanceToFinish_curr;
	targetNode = &GAME_TRACKER->level1->ptr_restart_points[target->checkpoint.currentIndex];
	prevNode = targetNode;
	// Keep the last node before crossing the target's distance or reaching the finish.
	while (1)
	{
		if ((targetNode->distToFinish << 3) < targetDistance || targetNode == GAME_TRACKER->level1->ptr_restart_points)
		{
			targetNode = prevNode;
			break;
		}
		prevNode = targetNode;
		targetNode = RB_Warpball_NewPathNode(prevNode, tw->driverTarget);
	}

	pathStarts[0] = tw->ptrNodeCurr;
	pathStarts[1] = NULL;
	backwardTarget = targetNode;
	// Search backward, then inspect the alternate right branch discovered along it.
	for (i = 0; i < 2 && !(tw->flags & TRACKER_FLAG_WARPBALL_TARGET_PATH); i++)
	{
		if (pathStarts[i] != NULL)
		{
			struct CheckpointNode *pathNode = pathStarts[i];
			for (j = 0; j < 3; j++)
			{
				if (pathNode == backwardTarget)
				{
					tw->flags = (s16)(tw->flags | TRACKER_FLAG_WARPBALL_TARGET_PATH) & ~TRACKER_FLAG_WARPBALL_FALLBACK_PATH;
					break;
				}
				if (pathNode->nextIndex_right != 0xff)
				{
					pathStarts[1] = &GAME_TRACKER->level1->ptr_restart_points[pathNode->nextIndex_right];
				}
				pathNode = &GAME_TRACKER->level1->ptr_restart_points[pathNode->nextIndex_backward];
			}
		}
	}

	pathNode = tw->ptrNodeCurr;
	forwardTarget = targetNode;
	for (forwardStep = 0; forwardStep < 3; forwardStep++)
	{
		if (pathNode == forwardTarget)
		{
			tw->flags = (s16)(tw->flags | TRACKER_FLAG_WARPBALL_TARGET_PATH) & ~TRACKER_FLAG_WARPBALL_FALLBACK_PATH;
			return;
		}
		pathNode = RB_Warpball_NewPathNode(pathNode, tw->driverTarget);
	}
}

void RB_Warpball_SeekDriver(struct TrackerWeapon *tw, u32 checkpointIndex, struct Driver *d)
{
	struct CheckpointNode *cn;

	if (d == NULL)
	{
		return;
	}
	checkpointIndex &= 0xff;
	if (checkpointIndex == 0xff)
	{
		return;
	}

	cn = &GAME_TRACKER->level1->ptr_restart_points[checkpointIndex];
	while (1)
	{
		if ((s32)d->distanceToFinish_curr > (cn->distToFinish << 3) || cn == GAME_TRACKER->level1->ptr_restart_points)
		{
			tw->nodeCurrIndex = cn - GAME_TRACKER->level1->ptr_restart_points;
			return;
		}
		cn = RB_Warpball_NewPathNode(cn, tw->driverTarget);
	}
}

void RB_Warpball_TurnAround(struct Thread *t)
{
	struct TrackerWeapon *tw;
	struct Instance *inst;
	TrackerWeaponFlags flags;
	struct GameTracker *gGT;
	s16 rot;

	tw = t->object;
	inst = t->inst;
	flags = tw->flags;

	if (
	    // if turnaround was requested
	    ((flags & TRACKER_FLAG_WARPBALL_TURN_AROUND) != 0) ||

	    // if no driver is being chased
	    (tw->driverTarget == NULL))
	{
		struct CheckpointNode *cn;
		if ((flags & TRACKER_FLAG_WARPBALL_TARGET_PATH) != 0)
		{
			tw->flags = (flags & ~TRACKER_FLAG_WARPBALL_TARGET_PATH) | TRACKER_FLAG_WARPBALL_BACKTRACKING | TRACKER_FLAG_WARPBALL_FALLBACK_PATH;
		}

		tw->vel.x = 0u - (u32)tw->vel.x;
		tw->vel.y = 0u - (u32)tw->vel.y;
		tw->vel.z = 0u - (u32)tw->vel.z;

		gGT = GAME_TRACKER;
		inst->matrix.t[0] = CTR_MipsAddLo(inst->matrix.t[0], CTR_MipsMulLo(tw->vel.x, gGT->elapsedTimeMS) >> 5);
		inst->matrix.t[1] = CTR_MipsAddLo(inst->matrix.t[1], CTR_MipsMulLo(tw->vel.y, gGT->elapsedTimeMS) >> 5);
		inst->matrix.t[2] = CTR_MipsAddLo(inst->matrix.t[2], CTR_MipsMulLo(tw->vel.z, gGT->elapsedTimeMS) >> 5);

		// increment counter
		tw->turnAroundFrames++;

		if (
		    // if count too high
		    (0x78 < tw->turnAroundFrames) ||

		    // pointer to driver being chased,
		    // is null, so warpball is chasing nobody
		    (tw->driverTarget == 0))
		{
			tw->driverParent->instBombThrow = 0;

			// play sound warpball death
			PlaySound3D(0x4f, inst);

			RB_Warpball_Death(t);
		}

		// Step along the backward path every fourth turnaround tick.
		if ((tw->turnAroundFrames & 3) == 0)
		{
			struct CheckpointNode *first;
			tw->ptrNodeNext = tw->ptrNodeCurr;

			first = &GAME_TRACKER->level1->ptr_restart_points[0];

			// The old current node becomes the next endpoint of the reversed segment.
			tw->ptrNodeCurr = &first[tw->ptrNodeCurr->nextIndex_backward];
		}

		cn = tw->ptrNodeCurr;

		// rotation
		rot = ratan2(CTR_MipsSubLo(cn->pos.x, inst->matrix.t[0]), CTR_MipsSubLo(cn->pos.z, inst->matrix.t[2]));

		// rotation
		tw->dir.y = rot;
	}
	return;
}

static inline void RB_Warpball_AdvanceStraight(struct TrackerWeapon *tw, struct Instance *inst)
{
	struct GameTracker *gGT = GAME_TRACKER;
	inst->matrix.t[0] = CTR_MipsAddLo(inst->matrix.t[0], CTR_MipsMulLo(tw->vel.x, gGT->elapsedTimeMS) >> 5);
	inst->matrix.t[1] = CTR_MipsAddLo(inst->matrix.t[1], CTR_MipsMulLo(tw->vel.y, gGT->elapsedTimeMS) >> 5);
	inst->matrix.t[2] = CTR_MipsAddLo(inst->matrix.t[2], CTR_MipsMulLo(tw->vel.z, gGT->elapsedTimeMS) >> 5);
}

static inline s32 RB_Warpball_NodeDeltaLength(struct CheckpointNode *curr, struct CheckpointNode *next, Vec3 *delta)
{
	delta->x = next->pos.x - curr->pos.x;
	delta->y = next->pos.y - curr->pos.y;
	delta->z = next->pos.z - curr->pos.z;

	return SquareRoot0_stub((u32)delta->x * delta->x + (u32)delta->y * delta->y + (u32)delta->z * delta->z);
}

static inline void RB_Warpball_SetQuadblockIndex(struct TrackerWeapon *tw, struct ScratchpadStruct *sps)
{
	if (sps->hit.ptrQuadblock->checkpointIndex != 0xff)
	{
		tw->nodeNextIndex = sps->hit.ptrQuadblock->checkpointIndex;
	}
}

void RB_Warpball_ThTick(struct Thread *t)
{
	struct TrackerWeapon *tw;
	struct Instance *inst;
	struct Driver *target;
	struct ScratchpadStruct *sps;
	struct Instance *hitInst;
	SVec3 posTop;
	SVec3 posBottom;
	s32 distX;
	s32 distY;
	s32 distZ;
	s32 distXZ;

	inst = t->inst;
	tw = t->object;
	sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);

	tw->savedPos.x = inst->matrix.t[0];
	tw->savedPos.y = inst->matrix.t[1];
	tw->savedPos.z = inst->matrix.t[2];

	if ((int)inst->animFrame + 1 < INSTANCE_GetNumAnimFrames(inst, 0))
	{
		inst->animFrame++;
	}
	else
	{
		inst->animFrame = 0;
	}

	if (tw->driverTarget != NULL)
	{
		if ((tw->driverTarget->kartState == KS_MASK_GRABBED) && ((tw->flags & TRACKER_FLAG_WARPBALL_TARGET_PATH) != 0))
		{
			tw->flags = (tw->flags & ~TRACKER_FLAG_WARPBALL_TARGET_PATH) | TRACKER_FLAG_WARPBALL_FALLBACK_PATH | TRACKER_FLAG_WARPBALL_MASK_REPATH;
			tw->ptrNodeCurr = &GAME_TRACKER->level1->ptr_restart_points[tw->nodeNextIndex];
			tw->ptrNodeNext = RB_Warpball_NewPathNode(tw->ptrNodeCurr, tw->driverTarget);
			tw->driverTarget = RB_Warpball_GetDriverTarget(tw, inst);
			RB_Warpball_SetTargetDriver(tw);
		}

		if ((tw->flags & TRACKER_FLAG_WARPBALL_TARGET_REFRESH_BLOCKED) == 0)
		{
			tw->driverTarget = RB_Warpball_GetDriverTarget(tw, inst);

			if (tw->driverTarget != NULL)
			{
				RB_Warpball_SetTargetDriver(tw);
			}
		}
	}

	target = tw->driverTarget;
	tw->flags &= ~TRACKER_FLAG_WARPBALL_BACKTRACKING;

	if (target != NULL)
	{
		struct Thread *trackingThread;
		distX = CTR_MipsSubLo(target->posCurr.x >> 8, inst->matrix.t[0]);
		distZ = CTR_MipsSubLo(target->posCurr.z >> 8, inst->matrix.t[2]);
		distY = CTR_MipsSubLo(target->posCurr.y >> 8, inst->matrix.t[1]);
		tw->distanceToTarget = CTR_MipsAddLo(CTR_MipsMulLo(distX, distX), CTR_MipsMulLo(distZ, distZ));
		distXZ = tw->distanceToTarget;
		trackingThread = RB_GetThread_ClosestTracker(tw->driverTarget);
		// NOTE(aalhendi): Resolve the recipient after the callback, as retail does.
		tw->driverTarget->thTrackingMe = trackingThread;

		// NOTE(aalhendi): Retail tests the flags in the upper half of the aligned velocity/flags word.
		if (CTR_ReadU32AlignedLE(&tw->vel.z) & ((u32)TRACKER_FLAG_WARPBALL_PATH_MODE << 16))
		{
			s16 rotSpeed = 0x100;
			s16 desiredYaw;

			if ((tw->flags & TRACKER_FLAG_WARPBALL_TARGET_PATH) == 0)
			{
				struct CheckpointNode *pathNode = tw->ptrNodeCurr;

				distX = CTR_MipsSubLo(pathNode->pos.x, inst->matrix.t[0]);
				distZ = CTR_MipsSubLo(pathNode->pos.z, inst->matrix.t[2]);
				distY = CTR_MipsSubLo(pathNode->pos.y, inst->matrix.t[1]);
				distXZ = (u32)distX * distX + (u32)distZ * distZ;

				if (distXZ < 0x4000)
				{
					tw->flags = (tw->flags & ~TRACKER_FLAG_WARPBALL_FALLBACK_PATH) | TRACKER_FLAG_WARPBALL_TURN_AROUND;
				}
				else if (distXZ < 0x24000)
				{
					rotSpeed = 0x400;
				}
			}

			if (distXZ < 0x90000)
			{
				rotSpeed = 0x400 - (distXZ >> 9);

				if (rotSpeed < 0x100)
				{
					rotSpeed = 0x100;
				}
			}

			if (tw->parentSafetyFrames > 0)
			{
				rotSpeed = 0x40;
			}

			desiredYaw = ratan2(distX, distZ);
			tw->dir.x = 0;
			tw->dir.y = RB_Hazard_InterpolateValue(tw->dir.y, desiredYaw, rotSpeed);
			tw->dir.z = 0;
			tw->vel.x = ((u32)MATH_Sin(tw->dir.y) * 7) >> 8;
			tw->vel.z = ((u32)MATH_Cos(tw->dir.y) * 7) >> 8;
			tw->vel.x = tw->vel.x * g_config.warpballSpeedMultiplier / 100;
			tw->vel.z = tw->vel.z * g_config.warpballSpeedMultiplier / 100;

			if (distY > 0)
			{
				tw->vel.y = (u32)tw->vel.y + (CTR_MipsSll(GAME_TRACKER->elapsedTimeMS, 2) >> 5);

				if (distY < tw->vel.y)
				{
					tw->vel.y = distY;
				}

				if (tw->vel.y > 0x60)
				{
					tw->vel.y = 0x60;
				}
			}
			else if (distY < 0)
			{
				tw->vel.y = (u32)tw->vel.y - (CTR_MipsSll(GAME_TRACKER->elapsedTimeMS, 2) >> 5);

				if (tw->vel.y < distY)
				{
					tw->vel.y = distY;
				}

				if (tw->vel.y < -0x60)
				{
					tw->vel.y = -0x60;
				}
			}

			RB_Warpball_AdvanceStraight(tw, inst);
		}
		else
		{
			struct CheckpointNode *next = tw->ptrNodeNext;
			struct CheckpointNode *curr = tw->ptrNodeCurr;
			Vec3 delta;
			s32 segmentLength = RB_Warpball_NodeDeltaLength(curr, next, &delta);
			s32 progress = CTR_MipsAddLo(tw->pathProgress, ((s32)((u32)GAME_TRACKER->elapsedTimeMS * 0x70) >> 5) * g_config.warpballSpeedMultiplier / 100);
			s32 yaw;

			if (segmentLength <= progress)
			{
				progress = CTR_MipsSubLo(progress, segmentLength);

				do
				{
					b32 reachedSegment;

					curr = next;
					next = RB_Warpball_NewPathNode(curr, tw->driverTarget);
					segmentLength = RB_Warpball_NodeDeltaLength(curr, next, &delta);
					// NOTE(aalhendi): Test before subtracting; the final segment's length is restored below.
					reachedSegment = progress < segmentLength;
					progress = CTR_MipsSubLo(progress, segmentLength);

					if (reachedSegment)
					{
						break;
					}
				} while (1);

				progress = CTR_MipsAddLo(progress, segmentLength);
			}

			tw->pathProgress = progress;
			tw->ptrNodeCurr = curr;
			tw->ptrNodeNext = next;

			if (segmentLength != 0)
			{
				progress = CTR_MipsDiv(CTR_MipsSll(progress, 12), segmentLength);
			}
			else
			{
				progress = 0;
			}

			inst->matrix.t[0] = CTR_MipsAddLo(curr->pos.x, CTR_MipsMulLo(delta.x, progress) >> 12);
			inst->matrix.t[1] = CTR_MipsAddLo(curr->pos.y, CTR_MipsMulLo(delta.y, progress) >> 12);
			inst->matrix.t[2] = CTR_MipsAddLo(curr->pos.z, CTR_MipsMulLo(delta.z, progress) >> 12);

			yaw = ratan2(delta.x, delta.z);
			tw->dir.y = yaw;
			tw->vel.x = ((u32)MATH_Sin(yaw) * 7) >> 8;
			tw->vel.z = ((u32)MATH_Cos(yaw) * 7) >> 8;
			tw->vel.x = tw->vel.x * g_config.warpballSpeedMultiplier / 100;
			tw->vel.z = tw->vel.z * g_config.warpballSpeedMultiplier / 100;
			tw->vel.y = 0;
		}
	}
	else
	{
		RB_Warpball_AdvanceStraight(tw, inst);
	}

	PlaySound3D_Flags(&tw->soundIDCount, 0x4e, inst);

	posTop.x = (s16)inst->matrix.t[0];
	posTop.y = (u16)inst->matrix.t[1] - 0x80;
	posTop.z = (s16)inst->matrix.t[2];
	posBottom.x = (s16)inst->matrix.t[0];
	posBottom.y = (u16)inst->matrix.t[1] + 0x80;
	posBottom.z = (s16)inst->matrix.t[2];

	sps->Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND | QUADBLOCK_FLAG_TRIGGER;
	sps->Union.QuadBlockColl.quadFlagsIgnored = 0;
	sps->Union.QuadBlockColl.searchFlags = COLL_SEARCH_TEST_INSTANCES | COLL_SEARCH_FORCE_INSTANCE_HIT;

	if (GAME_TRACKER->numPlyrCurrGame < 3)
	{
		sps->Union.QuadBlockColl.searchFlags = COLL_SEARCH_TEST_INSTANCES | COLL_SEARCH_HIGH_LOD | COLL_SEARCH_FORCE_INSTANCE_HIT;
	}

	sps->ptr_mesh_info = GAME_TRACKER->level1->ptr_mesh_info;
	COLL_SearchBSP_CallbackQUADBLK(&posTop, &posBottom, sps, 0);
	RB_MakeInstanceReflective(sps, inst);

	if ((sps->collision.stepFlags & COLL_STEP_TRIGGER_WEAPON_REACT) != 0)
	{
		RB_Warpball_TurnAround(t);
	}

	if (sps->boolDidTouchHitbox != 0)
	{
		sps->Input1.modelID = DYNAMIC_WARPBALL;

		if (RB_Hazard_CollLevInst(sps, t) == 1)
		{
			RB_Warpball_TurnAround(t);
		}
	}

	if (sps->boolDidTouchQuadblock != 0)
	{
		tw->flags |= TRACKER_FLAG_WARPBALL_TURN_AROUND;
		RB_Warpball_SetQuadblockIndex(tw, sps);
		tw->vel.y = 0;

		if (((tw->flags & TRACKER_FLAG_WARPBALL_PATH_MODE) != 0) && (inst->matrix.t[1] < sps->Union.QuadBlockColl.hitPos.y))
		{
			inst->matrix.t[1] = sps->Union.QuadBlockColl.hitPos.y;
			inst->depthBiasNormal = sps->hit.ptrQuadblock->draw_order_low - 1;
		}
	}
	else
	{
		posTop.x = (s16)inst->matrix.t[0];
		posTop.y = (u16)inst->matrix.t[1] - 0x900;
		posTop.z = (s16)inst->matrix.t[2];
		COLL_SearchBSP_CallbackQUADBLK(&posTop, &posBottom, sps, 0);

		if (sps->boolDidTouchQuadblock != 0)
		{
			tw->flags |= TRACKER_FLAG_WARPBALL_TURN_AROUND;
			RB_Warpball_SetQuadblockIndex(tw, sps);
		}

		if ((sps->boolDidTouchQuadblock == 0) && (((tw->flags & TRACKER_FLAG_WARPBALL_PATH_MODE) != 0) || (tw->driverTarget == NULL)))
		{
			RB_Warpball_TurnAround(t);
		}
	}

	{
		u16 *curve;
		u16 *sample;
		s32 sampleOffset;

		curve = RB_WARPBALL_PARTICLE_HEIGHT;
		sampleOffset = 10;

		// NOTE(aalhendi): Retail always uses the final glow sample. Preserve its
		// explicit sample-to-byte shift instead of folding it into the load offset.
		CTR_PSX_SHIFT_LEFT_IN_PLACE(sampleOffset, 1);
		sample = (u16 *)((u8 *)curve + sampleOffset);

		if ((*sample != 0) && (tw->ptrParticle != NULL))
		{
			u32 height;
			tw->ptrParticle->axis[0].startVal = (u32)inst->matrix.t[0] << 8;
			tw->ptrParticle->axis[1].startVal = ((u32)inst->matrix.t[1] + *sample) << 8;
			tw->ptrParticle->axis[2].startVal = (u32)inst->matrix.t[2] << 8;
			height = *sample;
			tw->ptrParticle->axis[3].startVal = height << 8;
			tw->ptrParticle->axis[4].startVal = height * 0xc0;
			tw->ptrParticle->axis[5].startVal = height << 7;
			tw->ptrParticle->otIndexOffset = inst->depthBiasNormal + 1;
			tw->ptrParticle->framesLeftInLife = -1;
		}
	}

	hitInst = RB_Hazard_CollideWithDrivers(inst, tw->parentSafetyFrames, 0x9000, tw->instParent);

	if (hitInst != NULL)
	{
		struct Driver *hitDriver = hitInst->thread->object;

		if (hitDriver != tw->driverParent)
		{
			b16 hadTargetPathFlag = tw->flags & TRACKER_FLAG_WARPBALL_TARGET_PATH;
			TrackerWeaponFlags flagsBeforeHit;

			RB_Hazard_HurtDriver(hitDriver, 2, tw->driverParent, 0);
			hitDriver->damageColorTimer = 0x1e;

			flagsBeforeHit = tw->flags | TRACKER_FLAG_WARPBALL_HIT_DRIVER;
			tw->flags = flagsBeforeHit;

			if ((((flagsBeforeHit & TRACKER_FLAG_POWERED_UP) == 0) && (tw->driverTarget == hitDriver)) || (hitDriver->driverRank == 0))
			{
				tw->driverParent->instBombThrow = NULL;
				RB_Warpball_Death(t);
				return;
			}

			tw->driversHit |= CTR_MipsSll(1, hitDriver->driverID);

			{
				s16 rank;
				for (rank = hitDriver->driverRank; rank < 8; rank++)
				{
					struct Driver *rankDriver = GAME_TRACKER->driversInRaceOrder[rank];

					if (rankDriver != NULL)
					{
						tw->driversHit |= CTR_MipsSll(1, rankDriver->driverID);
					}
				}
			}

			if (tw->driverTarget == hitDriver)
			{
				tw->flags &= ~TRACKER_FLAG_WARPBALL_TARGET_PATH;
				tw->driverTarget = RB_Warpball_GetDriverTarget(tw, inst);

				if (tw->driverTarget != NULL)
				{
					RB_Warpball_SetTargetDriver(tw);
				}
				else
				{
					tw->driverParent->instBombThrow = NULL;
					RB_Warpball_Death(t);
				}

				if (hadTargetPathFlag != 0)
				{
					RB_Warpball_SeekDriver(tw, hitDriver->checkpoint.currentIndex, hitDriver);
				}

				if (((tw->flags & TRACKER_FLAG_WARPBALL_TARGET_PATH) == 0) && (hadTargetPathFlag != 0))
				{
					if (tw->nodeCurrIndex != 0xff)
					{
						tw->ptrNodeCurr = &GAME_TRACKER->level1->ptr_restart_points[tw->nodeCurrIndex];
						tw->ptrNodeNext = RB_Warpball_NewPathNode(tw->ptrNodeCurr, tw->driverTarget);
					}

					tw->flags |= TRACKER_FLAG_WARPBALL_FALLBACK_PATH;
				}
			}
		}
	}
	else
	{
		hitInst = RB_Hazard_CollideWithBucket(inst, t, GAME_TRACKER->threadBuckets[MINE].thread, tw->parentSafetyFrames, 0x2400, tw->instParent);

		if (hitInst != NULL)
		{
			struct Thread *hitTh = hitInst->thread;

			((ThreadScratchCollideFunc)hitTh->funcThCollide)(hitTh, t, hitTh->funcThCollide, NULL);
		}
	}

	if (tw->parentSafetyFrames != 0)
	{
		tw->parentSafetyFrames--;
	}
}

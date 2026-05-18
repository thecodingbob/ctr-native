// PORTED BUT NOT INCLUDED (exist on disk, still using D233 incbin blob):
//   233_20_CS_LoadBoss.c             — CS_LoadBoss
//   233_21_CS_Camera_ThTick_Boss.c   — CS_Camera_ThTick_Boss
//   233_22_CS_Camera_BoolGotoBoss.c  — CS_Camera_BoolGotoBoss
//   233_27_CS_Podium_Prize_ThTick2.c — CS_Podium_Prize_ThTick2
//   233_31_CS_Podium_Stand_ThTick.c  — CS_Podium_Stand_ThTick
//   233_32_CS_Podium_Stand_Init.c    — CS_Podium_Stand_Init
//   233_33_CS_Podium_FullScene_Init.c — CS_Podium_FullScene_Init (+ fwd decls for CS_Podium_Prize_Init, CS_Camera_ThTick_Podium)
//   233_36_CS_BoxScene_InstanceSplitLines.c — CS_BoxScene_InstanceSplitLines
//
// NOT PORTED (still in D233 incbin blob):
//   CS_Thread_Particles              (0x800abdd4) — particle effects in cutscenes
//   CS_Thread_InterpolateFramesMS    (0x800ae318) — frame interpolation (GTE)
//   CS_Camera_ThTick_Podium          (0x800aedf8) — camera for podium cutscenes
//   CS_Podium_Prize_Spin             (0x800af7c0) — prize spinning animation
//   CS_Podium_Prize_ThTick3          (0x800af994) — podium prize tick phase 3
//   CS_Podium_Prize_ThTick1          (0x800afcc4) — podium prize tick phase 1
//   CS_Podium_Prize_Init             (0x800afe90) — prize initialization
//   CS_Garage_GetMenuPtr             (0x800b854c) — returns garage menu pointer

#include <common.h>

void DECOMP_CS_Thread_MoveOnPath(struct Thread *t)
{
	struct CutsceneObj *cs = t->object;
	struct Instance *inst = t->inst;
	struct Level *level;
	struct GameTracker *gGT;
	short modelID;
	int switchVal;
	int digit;
	struct SpawnType2 *spawnEntry;
	short *coords;
	short *curr;
	short *next;
	u_short progress;
	int idx;
	u_short frac;
	short rot[3];

	if ((cs->flags & 1) != 0)
		return;

	if (inst == 0)
		return;

	modelID = inst->model->id;
	switchVal = (short)(modelID - 0xA1);

	if ((u_int)switchVal >= 63)
		return;

	gGT = sdata->gGT;
	level = gGT->level1;

	switch (switchVal)
	{
	case 0x00:
	case 0x3E:

		digit = inst->name[strlen(inst->name) - 1] - '0';

		if (level->numSpawnType2 <= digit)
			return;

		spawnEntry = (struct SpawnType2 *)((char *)level->ptrSpawnType2 + digit * 8);
		coords = spawnEntry->posCoords;

		if (coords == 0)
			return;

		progress = cs->unk28;
		idx = (short)(progress << 16) >> 21;
		cs->unk28 = progress + gGT->elapsedTimeMS;
		frac = progress & 0x1f;

		if (idx >= spawnEntry->numCoords - 1)
		{
			idx = 0;

			if (modelID == 0xDF)
			{
				idx = spawnEntry->numCoords - 2;
				cs->unk28 = idx << 5;
			}
			else
			{
				cs->unk28 = 0;
			}
		}

		curr = &coords[idx * 6];
		next = &curr[6];

		inst->matrix.t[0] = curr[0] + ((frac * (next[0] - curr[0])) >> 5);
		inst->matrix.t[1] = curr[1] + ((frac * (next[1] - curr[1])) >> 5);
		inst->matrix.t[2] = curr[2] + ((frac * (next[2] - curr[2])) >> 5);

		if (idx >= spawnEntry->numCoords - 1)
			return;

		if (modelID == 0xDF)
			return;

		rot[0] = cs->unk20;
		rot[1] = cs->unk22 + ratan2(next[0] - curr[0], next[2] - curr[2]);
		rot[2] = cs->unk24;

		ConvertRotToMatrix(&inst->matrix, rot);
		return;

	case 0x01:
	case 0x02:
	case 0x39:
	case 0x3A:

		digit = inst->name[strlen(inst->name) - 1] - '0';

		if (level->numSpawnType2_PosRot <= digit)
			return;

		spawnEntry = (struct SpawnType2 *)((char *)level->ptrSpawnType2_PosRot + digit * 8);
		coords = spawnEntry->posCoords;

		if (coords == 0)
			return;

		progress = cs->unk28;
		cs->unk28 = progress + gGT->elapsedTimeMS;
		idx = (short)(progress << 16) >> 21;

		if (idx >= spawnEntry->numCoords - 1)
		{
			idx = 0;
			cs->unk28 = 0;
		}

		{
			short *point = &coords[(idx * 2 + idx) * 4];

			inst->matrix.t[0] = point[0];
			inst->matrix.t[1] = point[1];
			inst->matrix.t[2] = point[2];

			rot[0] = point[3];
			rot[1] = point[4];
			rot[2] = point[5];
		}

		break;

	case 0x30:

		if (level->numSpawnType2 <= 0)
			return;

		spawnEntry = level->ptrSpawnType2;
		coords = spawnEntry->posCoords;

		if (coords == 0)
			return;

		{
			u_int prog = 0;

			if (cs->animIndex == 3)
				prog = cs->unk18;

			frac = prog & 0x1f;
			int numCoords = spawnEntry->numCoords;
			idx = prog >> 5;

			if (idx < numCoords - 1)
			{
				if (idx >= 0)
				{
					curr = &coords[idx * 3];
					next = &curr[3];
				}
				else
				{
					curr = &coords[0];
					next = curr;
				}
			}
			else
			{
				curr = &coords[(numCoords - 1) * 3];
				next = curr;
			}

			inst->matrix.t[0] = curr[0] + ((frac * (next[0] - curr[0])) >> 5);
			inst->matrix.t[1] = curr[1] + ((frac * (next[1] - curr[1])) >> 5);
			inst->matrix.t[2] = curr[2] + ((frac * (next[2] - curr[2])) >> 5);
		}

		return;

	default:
		return;
	}

	ConvertRotToMatrix(&inst->matrix, rot);
}

void DECOMP_CS_Thread_Particles(struct Thread *t)
{
}

void DECOMP_CS_Thread_InterpolateFramesMS(struct Thread *t)
{
}

#include <common.h>

union RBBannerScratchVertex
{
	struct
	{
		s16 x;
		s16 z;
		s16 y;
		s16 residue;
	} components;
	struct
	{
		u32 xz;
		u32 yResidue;
	} words;
};

enum
{
	RB_BANNER_TEMP_VERTEX_SCRATCH_OFFSET = 0x300,
};

CTR_STATIC_ASSERT(sizeof(union RBBannerScratchVertex) == 0x8);

static inline u8 *RB_Banner_FirstVertex(struct ModelHeader *mh)
{
	return (u8 *)mh->ptrFrameData + mh->ptrFrameData->vertexOffset;
}

static inline union RBBannerScratchVertex *RB_Banner_VertexSlot(u32 scratchOffset)
{
	return CTR_SCRATCHPAD_PTR(union RBBannerScratchVertex, scratchOffset);
}

static inline union RBBannerScratchVertex *RB_Banner_TempVertex(void)
{
	return RB_Banner_VertexSlot(RB_BANNER_TEMP_VERTEX_SCRATCH_OFFSET);
}

int RB_Banner_Animate_Init(struct ModelHeader *mh)
{
	union RBBannerScratchVertex *temp = RB_Banner_TempVertex();
	s16 count = 0;
	u8 *vertex = RB_Banner_FirstVertex(mh);
	u32 *cmd = (u32 *)mh->ptrCommandList;
	u16 numColors = CTR_ReadU16AlignedLE(cmd);

	cmd++;
	if ((s16)numColors < 0x40)
	{
		return 0;
	}

	// NOTE(aalhendi): Commands cache X/Z/Y in eight-byte scratch slots.
	// The unused fourth halfword travels with the two-word copy.
	while (*cmd != 0xffffffffU)
	{
		if ((*cmd & 0xffff0000U) != 0)
		{
			if ((s32)*cmd < 0)
			{
				s16 i = 0;
				u8 *yz = vertex + 1;

				// A negative command starts a three-vertex group.
				for (; i < 3; i++, cmd++)
				{
					s32 xQuarter;

					if ((*cmd & 0x04000000U) == 0)
					{
						temp->components.x = vertex[0];
						temp->components.z = yz[1];
						temp->components.y = yz[0];
						RB_Banner_VertexSlot((u8)(*cmd >> 16) * 8)->words.xz = temp->words.xz;
						*CTR_SCRATCHPAD_PTR(u32, 4 + (u8)(*cmd >> 16) * 8) = temp->words.yResidue;
						*cmd &= 0xffff01ffU;
						xQuarter = vertex[0] >> 2;
						vertex += 3;
						yz += 3;
						count++;
						*cmd |= (u32)xQuarter << 9;
					}
					else
					{
						temp->words.xz = RB_Banner_VertexSlot((*cmd >> 13) & 0x7f8)->words.xz;
						temp->words.yResidue = *CTR_SCRATCHPAD_PTR(u32, 4 + (u8)(*cmd >> 16) * 8);
						*cmd &= 0xf7ff01ffU;
						xQuarter = (s32)((u32)(u16)temp->components.x << 16) >> 18;
						*cmd |= (u32)xQuarter << 9;
					}
				}
			}
			else
			{
				s32 xQuarter;

				if ((*cmd & 0x04000000U) == 0)
				{
					temp->components.x = vertex[0];
					temp->components.z = vertex[2];
					temp->components.y = vertex[1];
					RB_Banner_VertexSlot((u8)(*cmd >> 16) * 8)->words.xz = temp->words.xz;
					*CTR_SCRATCHPAD_PTR(u32, 4 + (u8)(*cmd >> 16) * 8) = temp->words.yResidue;
					*cmd &= 0xffff01ffU;
					xQuarter = vertex[0] >> 2;
					vertex += 3;
					count++;
					*cmd |= (u32)xQuarter << 9;
					cmd++;
				}
				else
				{
					temp->words.xz = RB_Banner_VertexSlot((*cmd >> 13) & 0x7f8)->words.xz;
					temp->words.yResidue = *CTR_SCRATCHPAD_PTR(u32, 4 + (u8)(*cmd >> 16) * 8);
					*cmd &= 0xf7ff01ffU;
					xQuarter = (s32)((u32)(u16)temp->components.x << 16) >> 18;
					*cmd |= (u32)xQuarter << 9;
					cmd++;
				}
			}
		}
		else
		{
			cmd++;
		}
	}

	if (GAME_TRACKER->numPlyrCurrGame >= 4)
	{
		u32 end;

		vertex = RB_Banner_FirstVertex(mh);
		end = (u32)vertex + (s32)count * 3;
		for (; (u32)vertex < end; vertex += 3)
		{
			vertex[1] = 0x80;
		}
	}
	return count;
}

void RB_Banner_Animate_Play(struct ModelHeader *mh, s16 numVertices)
{
	u32 *colors;
	u32 *palette = mh->ptrColors;
	s16 i;
	u8 *cursor = (u8 *)(palette + 1);
	u32 firstColor = palette[0];
	u32 end;

	for (i = 0; i < 0x3f; i++)
	{
		*palette++ = CTR_ReadU32AlignedLE(cursor);
		cursor += 4;
	}
	*palette = firstColor;

	// NOTE(aalhendi): Reuse the read cursor only after palette rotation has finished.
	cursor = RB_Banner_FirstVertex(mh);
	colors = mh->ptrColors;
	end = (u32)cursor + (s32)numVertices * 3;
	for (; (u32)cursor < end; cursor += 3)
	{
		s16 x = cursor[0];
		u8 color = ((u8 *)colors)[(((x >> 2) + 10) & 0x3f) * 4];
		s32 wave = (s32)color - 0x80;
		s32 factor;
		b32 nearLeftPole = x < 0x40;
		s32 scaled = wave;

		// Taper the outer quarters toward the poles; the middle keeps full amplitude.
		if (nearLeftPole)
		{
			factor = x * 4;
		}
		else if (x > 0xc0)
		{
			factor = (0x100 - x) * 4;
		}
		else
		{
			goto store_height;
		}
		// NOTE(aalhendi): Discard the eight fractional bits before narrowing to the stored Y byte.
		{
			u32 product = wave * factor;
			scaled = product >> 8;
		}

	store_height:
		cursor[1] = (u8)(scaled - 0x80);
	}
}

void RB_Banner_ThTick(struct Thread *t)
{
	struct StartBanner *banner = t->object;
	s16 numVertices = banner->numVertices;
	struct Instance *inst = t->inst;

	if (numVertices != 0)
	{
		RB_Banner_Animate_Play(inst->model->headers, numVertices);
	}
}

static const char s_startbanner[] = "startbanner";

void RB_Banner_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct StartBanner *banner;
	struct Model *model;

	if (inst->thread != NULL)
	{
		return;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct StartBanner), NONE, SMALL, STATIC), RB_Banner_ThTick, s_startbanner, NULL);
	inst->thread = t;
	if (t == NULL)
	{
		return;
	}

	if (GAME_TRACKER->numPlyrCurrGame >= 4)
	{
		t->funcThTick = NULL;
	}

	banner = t->object;
	t->inst = inst;
	banner->unused = 0;
	banner->numVertices = 0;

	model = GAME_TRACKER->modelPtr[STATIC_STARTBANNERWAVE];
	if (model == NULL)
	{
		return;
	}

	inst->model = model;
	banner->numVertices = RB_Banner_Animate_Init(model->headers);
	if (banner->numVertices == 0)
	{
		return;
	}

	{
		s16 i;
		u8 *color = (u8 *)inst->model->headers->ptrColors;

		// The palette carries two sine cycles; animation rotates it one sample per tick.
		for (i = 0; i < 0x40;)
		{
			s32 phase = i;
			u32 packed = CTR_ReadU32AlignedLE(&data.trigApprox[(phase * 128) & ANG_QUADRANT_MASK]);
			s32 value;

			if (((phase * 128) & ANG_QUADRANT_BIT) == 0)
			{
				packed <<= 16;
			}
			value = (s32)packed >> 16;
			if (((phase * 128) & ANG_SIGN_BIT) != 0)
			{
				value = -value;
			}
			value = (value >> 6) + 0x80;

			if (GAME_TRACKER->numPlyrCurrGame >= 4)
			{
				value = 0x80;
			}

			i++;
			color[2] = (u8)value;
			color[1] = (u8)value;
			color[0] = (u8)value;
			color += 4;
		}
	}
}

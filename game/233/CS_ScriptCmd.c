#include <common.h>

#ifndef CS_OPCODE_META_TABLE
// NOTE(aalhendi): Retail indexes the overlay bytes here, including the script
// bytes following the metadata prefix for otherwise undefined opcode values.
#define CS_OPCODE_META_TABLE ((const u8 *)&D233 + OFFSETOF(struct OverlayDATA_233, csOpcodeMetaPrefix))
#endif

s16 CS_ScriptCmd_ReadOpcode_GetShort(char **cursor)
{
	char *bytes = *cursor;
	u32 high = (u8)bytes[1];
	u32 low = (u8)bytes[0];
	*cursor = bytes + 2;
	return (s16)(low | (high << 8));
}

u32 CS_ScriptCmd_ReadOpcode_GetInt(char **cursor)
{
	char *bytes = *cursor;
	u32 byte3 = (u8)bytes[3];
	u32 byte2 = (u8)bytes[2];
	u32 byte1 = (u8)bytes[1];
	u32 byte0 = (u8)bytes[0];
	*cursor = bytes + 4;
	return (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
}

u32 CS_ScriptCmd_ReadOpcode_GetInt_dup(char **cursor)
{
	char *bytes = *cursor;
	u32 byte3 = (u8)bytes[3];
	u32 byte2 = (u8)bytes[2];
	u32 byte1 = (u8)bytes[1];
	u32 byte0 = (u8)bytes[0];
	*cursor = bytes + 4;
	return (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
}

void CS_ScriptCmd_ReadOpcode_Main(struct CutsceneObj *cs)
{
	char *opcodes;
	char *cursor;
	struct CsOpcodeMeta *decoded;
	u8 metaFlags;
	opcodes = cs->currOpcode[0];
	if (opcodes == cs->prevOpcode)
		return;
	cursor = opcodes;
	decoded = cs->metadataMeta;
	cs->prevOpcode = opcodes;
	decoded->opcode = (u8)*opcodes;
	cursor = opcodes + 1;
	// NOTE(aalhendi): The field-address form preserves GCC 2.8.1's opcode
	// reload after advancing the cursor, without changing the native access.
	metaFlags = CS_OPCODE_META_TABLE[*(&decoded->opcode)];
	if (metaFlags & CS_OPCODE_META_HAS_ANIM_INDEX)
	{
		decoded->animIndex = CS_ScriptCmd_ReadOpcode_GetShort(&cursor);
	}
	else
	{
		decoded->animIndex = 0;
	}

	if (metaFlags & CS_OPCODE_META_HAS_FRAME_START)
	{
		decoded->frameStart = CS_ScriptCmd_ReadOpcode_GetShort(&cursor);
	}
	else
	{
		decoded->frameStart = 0;
	}

	if (metaFlags & CS_OPCODE_META_HAS_FRAME_END)
	{
		decoded->frameEnd = CS_ScriptCmd_ReadOpcode_GetShort(&cursor);
	}
	else
	{
		decoded->frameEnd = 0;
	}

	if (metaFlags & CS_OPCODE_META_HAS_ARG0)
	{
		decoded->arg0.u = CS_ScriptCmd_ReadOpcode_GetInt(&cursor);
	}
	else
	{
		decoded->arg0.u = 0;
	}

	if (metaFlags & CS_OPCODE_META_HAS_ARG1)
	{
		decoded->arg1.u = CS_ScriptCmd_ReadOpcode_GetInt(&cursor);
	}
	else
	{
		decoded->arg1.u = 0;
	}

	if (metaFlags & CS_OPCODE_META_HAS_ALIGNED_ARG1)
	{
		while ((u32)cursor & 3)
		{
			cursor++;
		}

		decoded->arg1.u = CS_ScriptCmd_ReadOpcode_GetInt_dup(&cursor);
		cursor += 1;
	}

	if (metaFlags & CS_OPCODE_META_HAS_ROT_START)
	{
		decoded->rotStart = CS_ScriptCmd_ReadOpcode_GetShort(&cursor);
	}
	else
	{
		decoded->rotStart = 0;
	}

	if (metaFlags & CS_OPCODE_META_HAS_ROT_END)
	{
		decoded->rotEnd = CS_ScriptCmd_ReadOpcode_GetShort(&cursor);
	}
	else
	{
		decoded->rotEnd = 0;
	}

	cs->prevOpcode = cursor;
}

void CS_ScriptCmd_OpcodeNext(struct CutsceneObj *cs)
{
	char *prev = cs->prevOpcode;
	cs->prevOpcode = (char *)-1;
	cs->currOpcode[0] = prev;
	CS_ScriptCmd_ReadOpcode_Main(cs);
}

// CTR_NATIVE translates retail bytecode branch targets before the retail body.
void CS_ScriptCmd_OpcodeAt(struct CutsceneObj *cs, char *opCodeAt)
{
#ifdef CTR_NATIVE
	opCodeAt = CS_OVR233_TranslateRetailOpcodePointer(opCodeAt);
#endif

	cs->currOpcode[0] = opCodeAt;
	cs->prevOpcode = (char *)-1;
	CS_ScriptCmd_ReadOpcode_Main(cs);
}

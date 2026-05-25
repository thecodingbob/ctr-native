#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80046b60-0x80046bc0.
u32 RefreshCard_GhostEncodeByte(int currByte)
{
	s16 byte = currByte;

	if (byte < 10)
		return (currByte + '0') & 0xff;

	if (byte < 0x24)
		return (currByte + 0x37) & 0xff;

	if (byte < 0x3e)
		return (currByte + 0x3d) & 0xff;

	if (byte == 0x3e)
		return '-';

	return '_';
}

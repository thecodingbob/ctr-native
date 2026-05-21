#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80054a08-0x80054a78
u32 UI_VsQuipReadDriver(struct Driver *d, int offset, int size)
{
	char *data = (char *)d + offset;

	if (size == 2)
		return *(s16 *)data;

	if (size < 3)
	{
		if (size == 1)
			return *(u8 *)data;

		return 0;
	}

	if (size == 4)
		return *(u32 *)data;

	return 0;
}

u32 DECOMP_UI_VsQuipReadDriver(struct Driver *d, int offset, int size)
{
	return UI_VsQuipReadDriver(d, offset, size);
}

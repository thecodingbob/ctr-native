#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002991c-0x80029988
u32 howl_InstrumentPitch(int basePitch, int pitchIndex, u32 distort)
{
	// param_3
	// (>> 0) & 0x40 - distortion
	// (>> 6) & 0xXX - pitch/octave?

	u32 freq = data.noteFrequency[pitchIndex + ((int)distort >> 6) - 2] * basePitch >> 0xc;

	distort &= 0x3f;
	freq &= 0xffff;

	if (distort != 0)
	{
		freq = freq * (data.distortConst_Music[distort] + 0x100000) >> 0x14;
	}

	return freq & 0xffff;
}

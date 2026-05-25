#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b43cc-0x800b4470.
void AH_MaskHint_LerpVol(int param_1)
{
	int diff;
	int volume;
	u8 backup;

	for (char i = 0; i < 3; i++)
	{
		backup = D232.audioBackup[i];

		diff = D232.maskAudioSettings[i] - backup;
		volume = backup + ((diff * param_1) >> 12);

		// restore backups of Volume settings,
		// that were originally saved in AH_MaskHint_Start
		howl_VolumeSet(i, volume & 0xFF);
	}
}

#include <platform/native_audio.h>

int PsyX_SPUAL_PlayXATrack(int categoryID, int xaID, int volumeLeft, int volumeRight);
int PsyX_SPUAL_GetXATrackLength(int categoryID, int xaID);
int PsyX_SPUAL_IsXAPlaying(void);
void PsyX_SPUAL_StopXA(void);

int NativeAudio_PlayXATrack(int categoryID, int xaID, int volumeLeft, int volumeRight)
{
	return PsyX_SPUAL_PlayXATrack(categoryID, xaID, volumeLeft, volumeRight);
}

int NativeAudio_GetXATrackLength(int categoryID, int xaID)
{
	return PsyX_SPUAL_GetXATrackLength(categoryID, xaID);
}

int NativeAudio_IsXAPlaying(void)
{
	return PsyX_SPUAL_IsXAPlaying();
}

void NativeAudio_StopXA(void)
{
	PsyX_SPUAL_StopXA();
}

// NOTE(aalhendi): Name the resident audio fields individually so small scalar
// accesses use $gp; native uses the same storage through the canonical sData.
#define HOWL_AUDIO_ENABLED   howlEnabled
#define HOWL_STEREO          howlStereo
#define HOWL_VOLUME_FX       howlVolumeFX
#define HOWL_VOLUME_MUSIC    howlVolumeMusic
#define HOWL_VOLUME_VOICE    howlVolumeVoice
#define HOWL_CHANNELS        howlChannels
#define HOWL_CHANNEL_UPDATES howlChannelUpdates
#define HOWL_CHANNEL_ATTRS   howlChannelAttrs
#define HOWL_SONG_SEQUENCES  howlSongSequences
#define HOWL_SONGS           howlSongs
#define HOWL_SLIDER_PLAYING  howlSliderPlaying
#define HOWL_SLIDER_ROW      howlSliderRow
#define HOWL_SLIDER_SOUND    howlSliderSound

#include "../../retail_bindings.h"

extern u8 howlEnabled;
extern u8 howlStereo;
extern u8 howlVolumeFX;
extern u8 howlVolumeMusic;
extern u8 howlVolumeVoice;
extern struct LinkedList howlChannels;
extern u32 howlChannelUpdates[NUM_SFX_CHANNELS];
extern struct ChannelAttr howlChannelAttrs[NUM_SFX_CHANNELS];
extern struct SongSeq howlSongSequences[NUM_SFX_CHANNELS];
extern struct Song howlSongs[2];
extern s32 howlSliderPlaying;
extern s32 howlSliderRow;
extern u32 howlSliderSound;

#include <common.h>

int CseqMusic_Start(u16 songID, int p2, struct SongSet *p3, int p4, int p5)
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return 0;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return 0;
	}

	if (sdata->ptrCseqHeader->numSongs <= songID)
	{
		return 0;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is free
		if ((song->flags & 1) == 0)
		{
			// start song in this pool
			SongPool_Start(song, songID, p2, p5, p3, p4);

			Smart_ExitCriticalSection();
			return 1;
		}
	}

	Smart_ExitCriticalSection();
	return 0;
}

// pause all songs
void CseqMusic_Pause()
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if ((song->flags & 1) != 0)
		{
			// pause song
			song->flags |= 2;
		}
	}

	Smart_ExitCriticalSection();
}

// resume all songs
void CseqMusic_Resume()
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if ((song->flags & 1) != 0)
		{
			// unpause song
			song->flags &= ~(2);
		}
	}

	Smart_ExitCriticalSection();
}

void CseqMusic_ChangeVolume(u16 songID, int p2, int p3)
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader->numSongs <= songID)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if (((song->flags & 1) != 0) && (song->id == songID))
		{
			SongPool_Volume(song, p2 & 0xff, p3 & 0xff, 0);
		}
	}

	Smart_ExitCriticalSection();
}

void CseqMusic_Restart(u16 songID, int p2)
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader->numSongs <= songID)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if (((song->flags & 1) != 0) && (song->id == songID))
		{
			song->flags |= 4;
			SongPool_Volume(song, 0, p2 & 0xff, 0);
		}
	}

	Smart_ExitCriticalSection();
}

void CseqMusic_ChangeTempo(u16 songID, int p2)
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader->numSongs <= songID)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if (((song->flags & 1) != 0) && (song->id == songID))
		{
			SongPool_ChangeTempo(song, p2);
		}
	}

	Smart_ExitCriticalSection();
}

void CseqMusic_AdvHubSwap(u16 songId, struct SongSet *songSet, int songSetActiveBits)
{
	struct Song *song;
	int i;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader->numSongs <= songId)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if song is playing
		if (song->flags & 1)
		{
			if (song->id == songId)
			{
				SongPool_AdvHub2(song, songSet, songSetActiveBits);
			}
		}
	}

	Smart_ExitCriticalSection();
	return;
}

void CseqMusic_Stop(u16 songID)
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader->numSongs <= songID)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if (((song->flags & 1) != 0) && (song->id == songID))
		{
			SongPool_StopAllCseq(song);
		}
	}

	Smart_ExitCriticalSection();
}

void CseqMusic_StopAll()
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
	{
		return;
	}
	if (sdata->ptrCseqHeader == 0)
	{
		return;
	}

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if ((song->flags & 1) != 0)
		{
			SongPool_StopAllCseq(song);
		}
	}

	Smart_ExitCriticalSection();
}

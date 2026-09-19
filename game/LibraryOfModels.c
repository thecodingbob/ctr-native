#include <common.h>

enum LibraryOfModelsConstants
{
	// NOTE(aalhendi): Retail leaves the final modelPtr slot untouched.
	LIBRARY_OF_MODELS_CLEAR_COUNT = 0xe2,
};

void LibraryOfModels_Store(struct GameTracker *gGT, u32 numModels, struct Model **ptrModelArray)
{
	while (numModels != 0)
	{
		struct Model *m = *ptrModelArray;
		if (m == NULL)
		{
			return;
		}
		if (m->id != -1)
		{
			gGT->modelPtr[m->id] = m;
		}
		numModels--;
		ptrModelArray++;
	}
}

void LibraryOfModels_Clear(struct GameTracker *gGT)
{
	s32 i;

	for (i = 0; i < LIBRARY_OF_MODELS_CLEAR_COUNT; i++)
	{
		gGT->modelPtr[i] = 0;
	}
}

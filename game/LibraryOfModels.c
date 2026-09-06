#include <common.h>

enum LibraryOfModelsConstants
{
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
	for (s32 i = 0; i < LIBRARY_OF_MODELS_CLEAR_COUNT; i++)
	{
		gGT->modelPtr[i] = 0;
	}
}

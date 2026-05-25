#include <common.h>

u8 MEMCARD_Save(int slotIdx, char *name, char *param_3, u8 *ptrMemcard, int memcardFileSize, u32 param6)

{
#ifdef CTR_NATIVE
	char path[0x40];
	char *nameNoDevice;
	FILE *file;
	int crcResult;
	size_t wroteIcon;
	size_t wroteData;

	(void)slotIdx;
	(void)param_3;
	(void)param6;

	nameNoDevice = name;
	if (strncmp(nameNoDevice, "bu00:", 5) == 0)
		nameNoDevice += 5;

	sdata->crc16_checkpoint_byteIndex = 0;
	sdata->crc16_checkpoint_status = 0;
	do
	{
		crcResult = MEMCARD_ChecksumSave(ptrMemcard, memcardFileSize);
	} while (crcResult == MC_RETURN_PENDING);

	if (crcResult != MC_RETURN_IOE)
		return MC_RETURN_TIMEOUT;

	snprintf(path, sizeof(path), "%s", nameNoDevice);

	file = fopen(path, "wb");
	if (file == NULL)
		return MC_RETURN_FULL;

	wroteIcon = fwrite(&data.memcardIcon_Ghost[0], 1, 0x100, file);
	wroteData = fwrite(ptrMemcard, 1, memcardFileSize, file);
	fclose(file);

	if ((wroteIcon != 0x100) || (wroteData != (size_t)memcardFileSize))
		return MC_RETURN_TIMEOUT;

	return MC_RETURN_IOE;
#else
	if (sdata->memcard_stage != MC_STAGE_IDLE)
		return MC_RETURN_TIMEOUT;

	MEMCARD_NewTask(slotIdx, name, ptrMemcard, memcardFileSize, 0);


	int numBlock = 1;
	if (memcardFileSize == 0x3E00)
		numBlock = 2;

	// ====== Open 1 ======

	sdata->memcard_fd = open(sdata->s_memcardFileCurr, (numBlock << 16) | FCREATE);

	if (sdata->memcard_fd == -1)
	{
// do NOT return,
// the -1 could mean the file already exists
#if 0
		// MEMCARD_CloseFile();
        // return MC_RETURN_FULL;
#endif
	}
	else
	{
		close(sdata->memcard_fd);
		sdata->memcard_fd = -1;
	}

	// ====== Open 2 ======

	sdata->memcard_fd = open(sdata->s_memcardFileCurr, FASYNC | FWRITE);

	if (sdata->memcard_fd == -1)
	{
		MEMCARD_CloseFile();
		return MC_RETURN_FULL;
	}
	else
	{
		sdata->memcardIconSize = 0x100;
		sdata->crc16_checkpoint_byteIndex = 0;
		sdata->crc16_checkpoint_status = 0;
		sdata->memcard_stage = MC_STAGE_SAVE_PART0_START;
		return MC_RETURN_PENDING;
	}
#endif
}

#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032438-0x80032498
int LOAD_FindFile(char *filename, CdlFILE *cdlFile)
{
	if (filename == 0)
		return 0;
	if (cdlFile == 0)
		return 0;

	CDSYS_SetMode_StreamData();
	LOAD_StringToUpper(filename);

	return (CdSearchFile(cdlFile, filename) != 0);
}

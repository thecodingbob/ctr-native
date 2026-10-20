#include <shared/ctr_test_scratch.h>
#include <macros.h>

static char s_fixture[1024];
static bool s_useDisc;
static u8 s_data[6144];
static int s_callbacks;

#include <platform/native_cd.c>

FILE *NativeAssets_OpenHostStr8(NativeStr8 path, const char *mode)
{
	(void)path;
	return s_useDisc ? NULL : fopen(s_fixture, mode);
}
int NativeDiscImage_FindFile(const char *path, struct NativeDiscImageFile *file)
{
	(void)path;
	*file = (struct NativeDiscImageFile){ .lba = 100, .size = sizeof(s_data) };
	return 1;
}
int NativeDiscImage_ReadDataSectors(const struct NativeDiscImageFile *file, u32 sector, u32 count, void *destination)
{
	CTR_TEST_CHECK_EQ(file->lba, 100u, "disc fallback reads the requested file");
	if (((u64)sector + count) * 2048 > sizeof(s_data))
	{
		return 0;
	}
	memcpy(destination, s_data + sector * 2048, count * 2048);
	return 1;
}
static void ReadCallback(u8 result, u8 *details)
{
	(void)result; (void)details;
	s_callbacks++;
}

static void TestPositionalRead(bool disc)
{
	s_useDisc = disc;
	s_callbacks = 0;
	NativeCD_Init();
	CdlFILE file;
	CTR_TEST_CHECK(CdSearchFile(&file, "__ctr_test_audio_fixture__") != NULL, "the audio source is opened through the CD facade");
	CdControl(CdlSetloc, (u8 *)&file.pos, NULL);
	CdReadCallback(ReadCallback);
	u32 output[512];
	CTR_TEST_CHECK(NativeCD_ReadFileSectorsAt(&file, 2, 1, output), "positional reads work for either asset source");
	CTR_TEST_CHECK(memcmp(output, s_data + 4096, 2048) == 0, "positional reads select the requested sector");
	CTR_TEST_CHECK_EQ(s_callbacks, 0, "extra audio reads do not invoke the active CD callback");
	CdRead(1, output, CdlModeSpeed);
	CTR_TEST_CHECK(memcmp(output, s_data, 2048) == 0, "the active stream resumes at its original cursor");
	CTR_TEST_CHECK_EQ(s_callbacks, 1, "the active read callback is preserved");
	CTR_TEST_CHECK(!NativeCD_ReadFileSectorsAt(&file, 3, 1, output), "out-of-file reads are rejected");
	CTR_TEST_CHECK(!NativeCD_ReadFileSectorsAt(&file, ~0u, 2, output), "sector bounds cannot wrap around");
	CTR_TEST_CHECK(NativeCD_ReadFileSectorsAt(&file, 0, 1, output), "positional reads also work after stream advancement");
	CdRead(1, output, CdlModeSpeed);
	CTR_TEST_CHECK(memcmp(output, s_data + 2048, 2048) == 0, "positional reads preserve an advanced stream cursor");
	NativeCD_Init();
}

int main(void)
{
	char directory[1024];
	CtrTestScratch_Open(directory, sizeof(directory), "test-native-cd", "audio.bin");
	const int length = snprintf(s_fixture, sizeof(s_fixture), "%saudio.bin", directory);
	if (length < 0 || (size_t)length >= sizeof(s_fixture))
	{
		CtrTest_Bail("the fixture path must fit");
	}
	memset(s_data, 0x11, 2048);
	memset(s_data + 2048, 0x22, 2048);
	memset(s_data + 4096, 0x33, 2048);
	FILE *fixture = fopen(s_fixture, "wb");
	if (fixture == NULL || fwrite(s_data, 1, sizeof(s_data), fixture) != sizeof(s_data))
	{
		CtrTest_Bail("the disposable audio fixture must be writable");
	}
	fclose(fixture);
	CTR_TEST_SECTION("extra sample reads preserve the extracted-file stream");
	TestPositionalRead(false);
	CTR_TEST_SECTION("extra sample reads preserve the disc-fallback stream");
	TestPositionalRead(true);
	CtrTestScratch_Remove("test-native-cd", "audio.bin");
	return CtrTest_Summary();
}

#include <shared/ctr_test_scratch.h>
#include <platform/custom-characters/native_character_model.h>
#include <platform/custom-characters/native_character_library.h>
#include <platform/native_external_file.h>
#include <stdint.h>

static void Write32(unsigned char *p, uint32_t value)
{
	p[0] = (unsigned char)value;
	p[1] = (unsigned char)(value >> 8);
	p[2] = (unsigned char)(value >> 16);
	p[3] = (unsigned char)(value >> 24);
}

static void TestCharacterMetadata(const char *directory, const char *modelPath)
{
	CTR_TEST_SECTION("character metadata sections and local audio keys");
	char characterDirectory[1200], iniPath[1400], copiedModelPath[1400];
	snprintf(characterDirectory, sizeof(characterDirectory), "%scharacter", directory);
	snprintf(iniPath, sizeof(iniPath), "%s/character.ini", characterDirectory);
	snprintf(copiedModelPath, sizeof(copiedModelPath), "%s/model.ctr", characterDirectory);
	if (!SDL_CreateDirectory(characterDirectory) || !SDL_CopyFile(modelPath, copiedModelPath))
	{
		CtrTest_Bail("could not create a character metadata fixture");
	}
	FILE *file = fopen(iniPath, "wb");
	if (file == NULL)
	{
		CtrTest_Bail("could not open the character metadata fixture");
	}
	fputs("[character]\nid=fixture\nname=Fixture\nshort_name=FX\nauthor=Tests\nbase=polar\n"
	      "[assets]\nhi=model.ctr\n[audio]\nshort_00=audio/impact.adpcm\nline_18=audio/last.xa\n"
	      "short_1=../ignored\nshort_02=../ignored\nline_19=../ignored\n"
	      "[unknown]\nid=ignored\nline_18=../ignored\n", file);
	if (fclose(file) != 0)
	{
		CtrTest_Bail("could not write the character metadata fixture");
	}
	CTR_TEST_CHECK(NativeCharacterLibrary_Refresh(directory) && NativeCharacterLibrary_Count() == 1,
	               "metadata scanning ignores unknown sections and unsupported audio keys");
	const struct NativeCharacterMetadata *metadata = NativeCharacterLibrary_Get(0);
	if (metadata == NULL)
	{
		CtrTest_Bail("the character metadata fixture was not admitted");
	}
	CTR_TEST_CHECK(strcmp(metadata->identity, "fixture") == 0 && strcmp(metadata->name, "Fixture") == 0 &&
	               strcmp(metadata->shortName, "FX") == 0 && strcmp(metadata->author, "Tests") == 0 && strcmp(metadata->base, "polar") == 0,
	               "the character section stores every supported metadata field");
	CTR_TEST_CHECK(strstr(metadata->modelHi, "/model.ctr") != NULL,
	               "the assets section resolves the character-local model path");
	CTR_TEST_CHECK(strstr(metadata->shortVoices[0], "/audio/impact.adpcm") != NULL && metadata->shortVoices[1][0] == '\0',
	               "short voice keys require exactly two digits and remain optional");
	CTR_TEST_CHECK(NativeCharacterLibrary_GetVoicePath(metadata->directory, 18, 1) == metadata->xaVoices[18] &&
	               strstr(metadata->xaVoices[18], "/audio/last.xa") != NULL,
	               "the last supported XA key resolves through the public voice lookup");
	file = fopen(iniPath, "wb");
	if (file == NULL)
	{
		CtrTest_Bail("could not reopen the character metadata fixture");
	}
	fputs("[character]\nid=fixture\nname=Fixture\nbase=polar\n[assets]\nhi=model.ctr\n[audio]\nshort_00=../escape.adpcm\n", file);
	if (fclose(file) != 0)
	{
		CtrTest_Bail("could not write the invalid character metadata fixture");
	}
	CTR_TEST_CHECK(NativeCharacterLibrary_Refresh(directory) && NativeCharacterLibrary_Count() == 0,
	               "recognized audio keys still reject paths outside the character directory");
	NativeCharacterLibrary_Shutdown();
	SDL_RemovePath(iniPath);
	SDL_RemovePath(copiedModelPath);
	SDL_RemovePath(characterDirectory);
}

int main(int argc, char **argv)
{
	/* Optional local check: ordinary CTest runs never need game assets. */
	if (argc == 3 && strcmp(argv[1], "--check-file") == 0)
	{
		struct NativeExternalFile *file = NativeExternalFile_Open(argv[2]);
		if (file == NULL)
		{
			CtrTest_Bail("could not open the requested model");
		}
		size_t capacity = NativeExternalFile_SectorBytes(file);
		void *buffer = malloc(capacity);
		CTR_TEST_CHECK(buffer != NULL && NativeCharacterModel_Read(file, buffer, capacity), "the requested model has a valid envelope");
		free(buffer);
		NativeExternalFile_Close(file);
		return CtrTest_Summary();
	}
	CTR_TEST_SECTION("character model envelope");
	unsigned char model[4 + 24 + 64 + 8] = {0};
	Write32(model, 24 + 64);
	model[4 + 18] = 1;
	Write32(model + 4 + 20, 24);
	Write32(model + 4 + 24 + 64, 4);
	Write32(model + 8 + 24 + 64, 20);
	CTR_TEST_CHECK(NativeCharacterModel_CheckEnvelope(model, sizeof(model)), "a bounded group and relocated header are accepted");
	CTR_TEST_CHECK(!NativeCharacterModel_CheckEnvelope(model, sizeof(model) - 1), "a truncated relocation table is rejected");
	model[4 + 18] = 0;
	CTR_TEST_CHECK(!NativeCharacterModel_CheckEnvelope(model, sizeof(model)), "a model without headers is rejected");
	model[4 + 18] = 2;
	CTR_TEST_CHECK(!NativeCharacterModel_CheckEnvelope(model, sizeof(model)), "headers extending into the pointer map are rejected");
	model[4 + 18] = 1;
	Write32(model + 4 + 20, 25);
	CTR_TEST_CHECK(!NativeCharacterModel_CheckEnvelope(model, sizeof(model)), "unaligned headers are rejected");
	Write32(model + 4 + 20, 24);
	Write32(model + 8 + 24 + 64, 0);
	CTR_TEST_CHECK(!NativeCharacterModel_CheckEnvelope(model, sizeof(model)), "the header pointer must be included in the relocation table");

	CTR_TEST_SECTION("character model file");
	char directory[1024], path[1200];
	CtrTestScratch_Open(directory, sizeof(directory), "test-character-model", "model.ctr");
	snprintf(path, sizeof(path), "%smodel.ctr", directory);
	Write32(model + 8 + 24 + 64, 20);
	FILE *stream = fopen(path, "wb");
	if (stream == NULL || fwrite(model, 1, sizeof(model), stream) != sizeof(model) || fclose(stream) != 0)
	{
		CtrTest_Bail("could not write model fixture");
	}
	struct NativeExternalFile *file = NativeExternalFile_Open(path);
	unsigned char buffer[2048];
	CTR_TEST_CHECK(file != NULL && NativeExternalFile_Size(file) == sizeof(model), "a model can be opened from a local directory");
	CTR_TEST_CHECK(!NativeCharacterModel_Read(file, buffer, sizeof(buffer) - 1), "an undersized destination is rejected");
	CTR_TEST_CHECK(NativeCharacterModel_Read(file, buffer, sizeof(buffer)), "a model with a valid envelope can be read");
	CTR_TEST_CHECK(memcmp(model, buffer, sizeof(model)) == 0 && buffer[sizeof(buffer) - 1] == 0, "the source bytes and zero padding are preserved");
	NativeExternalFile_Close(file);
	TestCharacterMetadata(directory, path);
	CtrTestScratch_Remove("test-character-model", "model.ctr");
	return CtrTest_Summary();
}

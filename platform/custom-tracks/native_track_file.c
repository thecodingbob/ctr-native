#include <platform/custom-tracks/native_track_file.h>
#include <platform/custom-tracks/native_track_validate.h>
#include <platform/native_external_file.h>

#include <stdlib.h>

struct NativeTrackFile
{
	struct NativeExternalFile *file;
	enum NativeTrackFileRole role;
};

struct NativeTrackFile *NativeTrackFile_Open(const struct NativeTrackRegistry *registry, int levelID, enum NativeTrackFileRole role)
{
	const struct NativeTrackMetadata *metadata = NativeTrackRegistry_Get(registry, levelID);
	if (metadata == NULL)
	{
		return NULL;
	}
	const char *path = NULL;
	switch (role)
	{
	case NATIVE_TRACK_FILE_LEV: path = metadata->levPath; break;
	case NATIVE_TRACK_FILE_VRM: path = metadata->vrmPath; break;
	case NATIVE_TRACK_FILE_SCA: path = metadata->scaPath; break;
	default: return NULL;
	}
	struct NativeExternalFile *source = NativeExternalFile_Open(path);
	if (source == NULL)
	{
		return NULL;
	}
	struct NativeTrackFile *file = malloc(sizeof(*file));
	if (file == NULL)
	{
		NativeExternalFile_Close(source);
		return NULL;
	}
	file->file = source;
	file->role = role;
	return file;
}

size_t NativeTrackFile_Size(const struct NativeTrackFile *file)
{
	return file != NULL ? NativeExternalFile_Size(file->file) : 0;
}

size_t NativeTrackFile_SectorBytes(const struct NativeTrackFile *file)
{
	return file != NULL ? NativeExternalFile_SectorBytes(file->file) : 0;
}

int NativeTrackFile_Read(struct NativeTrackFile *file, void *destination, size_t capacity)
{
	if (file == NULL || !NativeExternalFile_Read(file->file, destination, capacity))
	{
		return 0;
	}
	size_t size = NativeExternalFile_Size(file->file);
	if ((file->role == NATIVE_TRACK_FILE_LEV && !NativeTrackValidate_Lev(destination, size)) ||
	    (file->role == NATIVE_TRACK_FILE_VRM && !NativeTrackValidate_Vrm(destination, size)))
	{
		return 0;
	}
	return 1;
}

void NativeTrackFile_Close(struct NativeTrackFile *file)
{
	if (file != NULL)
	{
		NativeExternalFile_Close(file->file);
		free(file);
	}
}

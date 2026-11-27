#include <platform/native_ini.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "native_config_internal.h"

static bool NativeConfig_ParseBool(const char *value, bool *destination, ConfigParseMode mode)
{
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0)
    {
        *destination = true;
        return true;
    }
    if (mode == CONFIG_PARSE_COMPATIBLE || strcmp(value, "false") == 0 || strcmp(value, "0") == 0)
    {
        *destination = false;
        return true;
    }
    return false;
}

static bool NativeConfig_ParseNumber(const char *value, int *destination)
{
    errno = 0;
    char *end;
    long number = strtol(value, &end, 10);
    if (end == value || *end != '\0' || errno == ERANGE || number < INT_MIN || number > INT_MAX)
    {
        return false;
    }
    *destination = (int)number;
    return true;
}

static bool NativeConfig_ParseInt(const ConfigEntry *entry, const char *value, int *destination, ConfigParseMode mode)
{
    if (mode == CONFIG_PARSE_COMPATIBLE)
    {
        *destination = atoi(value);
        return true;
    }
    int number;
    if (!NativeConfig_ParseNumber(value, &number) || number < entry->min || number > entry->max)
    {
        return false;
    }
    *destination = number;
    return true;
}

static bool NativeConfig_ParseEnum(const ConfigEntry *entry, const char *value, int *destination, ConfigParseMode mode)
{
    for (int i = 0; i < entry->numEnumValues; i++)
    {
        if (strcmp(value, entry->enumValues[i].name) == 0)
        {
            *destination = entry->enumValues[i].value;
            return true;
        }
    }
    if (mode == CONFIG_PARSE_COMPATIBLE)
    {
        *destination = atoi(value);
        return true;
    }
    int number;
    if (!NativeConfig_ParseNumber(value, &number)) { return false; }
    for (int i = 0; i < entry->numEnumValues; i++)
    {
        if (number == entry->enumValues[i].value)
        {
            *destination = number;
            return true;
        }
    }
    return false;
}

static bool NativeConfig_ParseValue(const ConfigEntry *entry, const char *value, void *destination,
                                    ConfigParseMode mode, ConfigParseOverride parseOverride)
{
    bool result;
    if (parseOverride != NULL && parseOverride(entry, value, destination, mode, &result)) { return result; }
    switch (entry->type)
    {
        case CFG_BOOL: return NativeConfig_ParseBool(value, destination, mode);
        case CFG_INT: return NativeConfig_ParseInt(entry, value, destination, mode);
        case CFG_ENUM: return NativeConfig_ParseEnum(entry, value, destination, mode);
        case CFG_ACTION:
        case CFG_SUBSECTION:
        default: return false;
    }
}

ConfigLoadResult NativeConfig_ReadFile(const char *path, void *context, ConfigDestination destination,
                                       const ConfigEntry *entries, int entryCount, ConfigParseMode mode,
                                       ConfigParseOverride parseOverride)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return errno == ENOENT ? CONFIG_LOAD_NOT_FOUND : CONFIG_LOAD_FILE_ERROR;
    }
    ConfigLoadResult result = CONFIG_LOAD_OK;
    char buffer[256];
    char section[64] = "";
    struct NativeIniLine line;
    enum NativeIniReadResult readResult;
    while ((readResult = NativeIni_Read(file, buffer, sizeof(buffer), mode == CONFIG_PARSE_STRICT, &line)) == NATIVE_INI_READ_RECORD)
    {
        if (line.type == NATIVE_INI_SECTION)
        {
            if (mode == CONFIG_PARSE_STRICT &&
                (line.section == NULL || line.section[0] == '\0' || line.trailingSectionText || strlen(line.section) >= sizeof(section)))
            {
                result = CONFIG_LOAD_INVALID;
                break;
            }
            if (line.section != NULL)
            {
                strncpy(section, line.section, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
            }
            continue;
        }
        if (line.type == NATIVE_INI_MALFORMED)
        {
            if (mode == CONFIG_PARSE_STRICT) { result = CONFIG_LOAD_INVALID; break; }
            continue;
        }
        const ConfigEntry *entry = NULL;
        for (int i = 0; i < entryCount; i++)
        {
            if (strcmp(section, entries[i].section) == 0 && strcmp(line.key, entries[i].key) == 0)
            {
                entry = &entries[i];
                break;
            }
        }
        if (entry == NULL || entry->type == CFG_ACTION || entry->type == CFG_SUBSECTION)
        {
            if (mode == CONFIG_PARSE_STRICT) { result = CONFIG_LOAD_INVALID; break; }
            continue;
        }
        if (!NativeConfig_ParseValue(entry, line.value, destination(entry, context), mode, parseOverride))
        {
            result = CONFIG_LOAD_INVALID;
            break;
        }
    }
    if (readResult == NATIVE_INI_READ_TOO_LONG) { result = CONFIG_LOAD_INVALID; }
    if (readResult == NATIVE_INI_READ_ERROR || ferror(file)) { result = CONFIG_LOAD_FILE_ERROR; }
    if (fclose(file) != 0) { result = CONFIG_LOAD_FILE_ERROR; }
    return result;
}

const char *NativeConfig_EnumName(const ConfigEntry *entry, int value)
{
    for (int i = 0; i < entry->numEnumValues; i++)
    {
        if (entry->enumValues[i].value == value) { return entry->enumValues[i].name; }
    }
    return NULL;
}

bool NativeConfig_WriteValue(FILE *file, const ConfigEntry *entry, const void *value, ConfigEnumNameOverride nameOverride)
{
    switch (entry->type)
    {
        case CFG_BOOL: return NativeIni_WriteValue(file, entry->key, *(const bool *)value ? "true" : "false");
        case CFG_INT: return NativeIni_WriteInt(file, entry->key, *(const int *)value);
        case CFG_ENUM:
        {
            int number = *(const int *)value;
            const char *name = NativeConfig_EnumName(entry, number);
            if (name == NULL && nameOverride != NULL) { name = nameOverride(entry, number); }
            return NativeIni_WriteValue(file, entry->key, name != NULL ? name : "?");
        }
        case CFG_ACTION:
        case CFG_SUBSECTION:
        default: return false;
    }
}

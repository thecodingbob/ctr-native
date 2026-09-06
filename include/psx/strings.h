/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/include/psx/strings.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef STRINGS_H
#define STRINGS_H

#if defined(CTR_NATIVE)
#include <ctype.h>
#include <string.h>
#else
void *memcpy(void *dst, const void *src, u32 count);
void *memmove(void *dst, const void *src, u32 count);
void *memset(void *dst, int value, u32 count);
char *strcat(char *dst, const char *src);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, u32 count);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, u32 count);
u32 strlen(const char *text);
int tolower(int value);
#endif

#endif

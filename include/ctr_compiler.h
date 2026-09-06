#ifndef CTR_COMPILER_H
#define CTR_COMPILER_H

#if defined(CTR_NATIVE)
#include <stdlib.h>
#endif

// NOTE(aalhendi): The MSVC C runtime exposes non-standard min/max macros in C mode. They collide with the project's typed helpers even when NOMINMAX is set.
#if defined(_MSC_VER)
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

#if (defined(__GNUC__) && ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)))) || defined(__clang__)
#define CTR_MAY_ALIAS __attribute__((may_alias))
#else
#define CTR_MAY_ALIAS
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CTR_PRINTF_FORMAT(fmtArg, firstVararg) __attribute__((format(printf, fmtArg, firstVararg)))
#define CTR_TRAP()                             __builtin_trap()
#else
#define CTR_PRINTF_FORMAT(fmtArg, firstVararg)
#define CTR_TRAP() abort()
#endif

// NOTE(aalhendi): These constraints shape GCC 2.8.1 register allocation and
// scheduling without embedding game functions in assembly. Native expansions
// preserve only their C semantics.
#if defined(CTR_NATIVE)
#define CTR_PSX_MATCH_SECTION(sectionName)
#define CTR_PSX_REGISTER(registerName)
#define CTR_PSX_BIND_VALUE_CLOBBER(value, registerName) ((void)(value))
#define CTR_PSX_CLOBBER(registerName)                   ((void)0)
#define CTR_PSX_KEEP_VALUE(value)                       ((void)(value))
#define CTR_PSX_MEMORY_BARRIER()                        ((void)0)
#define CTR_PSX_FORGET_VALUE(value)                     ((void)(value))
#define CTR_PSX_RELOAD(value)                           ((void)(value))
#define CTR_PSX_LOAD_SYMBOL_PAGE(page, symbolExpression) \
	do                                                   \
	{                                                    \
		(void)sizeof(page);                              \
	} while (0)
#define CTR_PSX_LOAD_WORD_FROM_PAGE(value, page, symbolExpression, nativeValue) \
	do                                                                          \
	{                                                                           \
		(void)sizeof(page);                                                     \
		(value) = (nativeValue);                                                \
	} while (0)
#define CTR_PSX_PAGE_LVALUE(type, page, offset, nativeLvalue) (nativeLvalue)
#define CTR_PSX_ADD_SYMBOL_LOW(result, page, symbolExpression, nativeValue) \
	do                                                                      \
	{                                                                       \
		(void)sizeof(page);                                                 \
		(result) = (nativeValue);                                           \
	} while (0)
#else
#define CTR_PSX_MATCH_SECTION(sectionName)                                      __attribute__((section(sectionName)))
#define CTR_PSX_REGISTER(registerName)                                          __asm__(registerName)
#define CTR_PSX_BIND_VALUE_CLOBBER(value, registerName)                         __asm__("" : "+r"(value) : : registerName)
#define CTR_PSX_CLOBBER(registerName)                                           __asm__ volatile("" : : : registerName)
#define CTR_PSX_KEEP_VALUE(value)                                               __asm__ volatile("" : "+r"(value))
#define CTR_PSX_MEMORY_BARRIER()                                                __asm__ volatile("" : : : "memory")
#define CTR_PSX_FORGET_VALUE(value)                                             __asm__ volatile("" : "=r"(value) : "0"(value))
#define CTR_PSX_RELOAD(value)                                                   __asm__("" : "+m"(value))
#define CTR_PSX_LOAD_SYMBOL_PAGE(page, symbolExpression)                        __asm__("lui %0,%%hi(" symbolExpression ")" : "=r"(page))
#define CTR_PSX_LOAD_WORD_FROM_PAGE(value, page, symbolExpression, nativeValue) __asm__("lw %0,%%lo(" symbolExpression ")(%1)" : "=r"(value) : "r"(page))
#define CTR_PSX_PAGE_LVALUE(type, page, offset, nativeLvalue)                   (*(type *)((u32)(page) + (s32)(offset)))
#define CTR_PSX_ADD_SYMBOL_LOW(result, page, symbolExpression, nativeValue)     __asm__("addiu %0,%1,%%lo(" symbolExpression ")" : "=r"(result) : "r"(page))
#endif

#endif

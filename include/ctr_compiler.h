#ifndef CTR_COMPILER_H
#define CTR_COMPILER_H

#include <stdlib.h>

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
#else
#define CTR_PRINTF_FORMAT(fmtArg, firstVararg)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CTR_TRAP() __builtin_trap()
#else
#define CTR_TRAP() abort()
#endif

// NOTE(aalhendi): These constraints isolate GCC 2.8.1 register allocation and
// scheduling requirements from shared game logic. Native expansions preserve
// only their C semantics.
#define CTR_PSX_MATCH_SECTION(sectionName)
#if defined(__GNUC__) || defined(__clang__)
#define CTR_PSX_REGISTER(registerName) __attribute__((unused))
#else
#define CTR_PSX_REGISTER(registerName)
#endif
#define CTR_PSX_BIND_VALUE_CLOBBER(value, registerName) ((void)(value))
#define CTR_PSX_DEPEND_VALUE(value, dependency)         ((void)(value), (void)(dependency))
#define CTR_PSX_ORDER_VALUES(value, dependency)         ((void)(value), (void)(dependency))
#define CTR_PSX_DEPEND_MEMORY(pointer, dependency)      ((void)(pointer), (void)(dependency))
#define CTR_PSX_CLOBBER(registerName)                   ((void)0)
#define CTR_PSX_KEEP_VALUE(value)                       ((void)(value))
#define CTR_PSX_KEEP_VALUE_RELAXED(value)               ((void)(value))
#define CTR_PSX_OBSERVE_VALUE(value)                    ((void)(value))
#define CTR_PSX_OBSERVE_MEMORY(value)                   ((void)sizeof(value))
#define CTR_PSX_ZERO_VALUE(value)                       ((value) = 0)
#define CTR_PSX_MEMORY_BARRIER()                        ((void)0)
#define CTR_PSX_FORGET_VALUE(value)                     ((void)(value))
#define CTR_PSX_RELOAD(value)                           ((void)(value))
#define CTR_PSX_CAPTURE_REGISTER(value, nativeValue)    ((value) = (nativeValue))
#define CTR_PSX_COPY_VALUE(result, value)               ((result) = (value))
#define CTR_PSX_LOAD_SYMBOL_PAGE(page, symbolExpression) \
	do                                                   \
	{                                                    \
		(page) = 0;                                      \
	} while (0)
#define CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(page, symbolExpression, dependency) \
	do                                                                     \
	{                                                                      \
		(page) = 0;                                                        \
		(void)(dependency);                                                \
	} while (0)
#define CTR_PSX_LOAD_WORD_FROM_PAGE(value, page, symbolExpression, nativeValue) \
	do                                                                          \
	{                                                                           \
		(void)sizeof(page);                                                     \
		(value) = (nativeValue);                                                \
	} while (0)
#define CTR_PSX_LOAD_WORD_FROM_PAGE_AFTER(value, page, symbolExpression, nativeValue, dependency) \
	do                                                                                            \
	{                                                                                             \
		(void)sizeof(page);                                                                       \
		(void)(dependency);                                                                       \
		(value) = (nativeValue);                                                                  \
	} while (0)
#define CTR_PSX_ADD_POINTER_OFFSET(result, base, offset)                         ((result) = (void *)((char *)(base) + (offset)))
#define CTR_PSX_ADD_POINTER_OFFSET_OFFSET_FIRST(result, base, offset)            ((result) = (void *)((char *)(base) + (offset)))
#define CTR_PSX_ADD_POINTER_IMMEDIATE(result, base, byteOffset, nativeValue)     ((result) = (nativeValue))
#define CTR_PSX_NEGATE_IN_PLACE(value)                                           ((value) = CTR_MipsNegLo(value))
#define CTR_PSX_NEGATE(result, value)                                            ((result) = CTR_MipsNegLo(value))
#define CTR_PSX_LOAD_IMMEDIATE(result, value)                                    ((result) = (value))
#define CTR_PSX_LOAD_STACK_WORD(result, byteOffset, nativeValue)                 ((result) = (nativeValue))
#define CTR_PSX_LOAD_WORD(result, nativeValue)                                   ((result) = (nativeValue))
#define CTR_PSX_LOAD_WORD_DELAYED(result, nativeValue)                           ((result) = (nativeValue))
#define CTR_PSX_LOAD_WORD_VOLATILE(result, nativeValue)                          ((result) = (nativeValue))
#define CTR_PSX_LOAD_SIGNED_BYTE(result, base, byteOffset, nativeValue)          ((result) = (s8)(nativeValue))
#define CTR_PSX_LOAD_SIGNED_HALF(result, base, byteOffset, nativeValue)          ((result) = (s16)(nativeValue))
#define CTR_PSX_LOAD_SIGNED_HALF_VOLATILE(result, base, byteOffset, nativeValue) ((result) = (s16)(nativeValue))
#define CTR_PSX_LOAD_SIGNED_HALF_AFTER(result, base, byteOffset, nativeValue, dependency) \
	do                                                                                    \
	{                                                                                     \
		(result) = (s16)(nativeValue);                                                    \
		(void)(dependency);                                                               \
	} while (0)
#define CTR_PSX_LOAD_UNSIGNED_BYTE(result, base, byteOffset, nativeValue) ((result) = (u8)(nativeValue))
#define CTR_PSX_LOAD_UNSIGNED_HALF(result, base, byteOffset, nativeValue) ((result) = (u16)(nativeValue))
#define CTR_PSX_SHIFT_LEFT_IN_PLACE(value, shift)                         ((value) = CTR_MipsSll((value), (shift)))
#define CTR_PSX_SHIFT_RIGHT_ARITHMETIC(result, value, shift)              ((result) = CTR_MipsSra((value), (shift)))
#define CTR_PSX_SHIFT_RIGHT_ARITHMETIC_IN_PLACE(value, shift)             ((value) = CTR_MipsSra((value), (shift)))
#define CTR_PSX_STORE_HALF(lvalue, value)                                 ((lvalue) = (s16)(value))
#define CTR_PSX_SUBTRACT(result, lhs, rhs)                                ((result) = CTR_MipsSubLo((lhs), (rhs)))
#define CTR_PSX_STORE_COP2_WORD(address, cop2Register)                    CTR_WriteU32LE((address), (u32)MFC2_S(cop2Register))
#define CTR_PSX_STORE_COP2_VEC3(address, xRegister, yRegister, zRegister) \
	do                                                                    \
	{                                                                     \
		(address)[0] = (s32)MFC2_S(xRegister);                            \
		(address)[1] = (s32)MFC2_S(yRegister);                            \
		(address)[2] = (s32)MFC2_S(zRegister);                            \
	} while (0)
#define CTR_PSX_STORE_COP2_HALF_DEAD_SCRATCH(base, byteOffset, cop2Register, scratchRegister) \
	do                                                                                        \
	{                                                                                         \
		*(s16 *)((u8 *)(base) + (byteOffset)) = (s16)MFC2_S(cop2Register);                    \
	} while (0)
#define CTR_PSX_GTE_PIPELINE_DELAY()                          ((void)0)
#define CTR_PSX_GTE_READ_DELAY()                              ((void)0)
#define CTR_PSX_PAGE_LVALUE(type, page, offset, nativeLvalue) (nativeLvalue)
#define CTR_PSX_ADD_SYMBOL_LOW(result, page, symbolExpression, nativeValue) \
	do                                                                      \
	{                                                                       \
		(void)sizeof(page);                                                 \
		(result) = (nativeValue);                                           \
	} while (0)
#define CTR_PSX_ADD_SYMBOL_LOW_DISTINCT(result, page, symbolExpression, nativeValue, identity) \
	CTR_PSX_ADD_SYMBOL_LOW(result, page, symbolExpression, nativeValue)
#define CTR_PSX_ADD_SYMBOL_LOW_IN_PLACE(value, symbolExpression, nativeValue) \
	do                                                                        \
	{                                                                         \
		(value) = (nativeValue);                                              \
	} while (0)

#endif

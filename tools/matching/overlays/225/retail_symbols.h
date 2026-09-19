#ifndef CTR_MATCHING_OVERLAY_225_RETAIL_SYMBOLS_H
#define CTR_MATCHING_OVERLAY_225_RETAIL_SYMBOLS_H

#include "../../retail_bindings.h"

// NOTE(aalhendi): Overlay 225 addresses resident EXE state absolutely rather
// than through gp. Keep these artifact bindings out of shared layout headers.
#define VB_GAME_TRACKER_PAGE        0x80090000u
#define VB_GAME_TRACKER_PAGE_OFFSET (-0x2d54)
#define VB_STANDINGS_SUFFIX_PAGE    ((s16 *)0x800a0000u)
// NOTE(aalhendi): suffixBase is pinned to $2 in shared C. Keep this basic asm
// register-specific so GCC 2.8.1 does not extend the value's live range.
#define VB_ADD_STANDINGS_SUFFIX_LOW(value)                         \
	do                                                             \
	{                                                              \
		(void)sizeof(value);                                       \
		__asm__("addiu $2,$2,%lo(s_standingsSuffixStringIds225)"); \
	} while (0)
// NOTE(aalhendi): Retail keeps the GameTracker page in $3 at the viewport-loop
// tail. The page-relative load preserves that lifetime without leaking the
// artifact offset into shared source.
#define VB_PLAYER_COUNT_FROM_PAGE(page)      (CTR_PSX_PAGE_LVALUE(struct GameTracker *, (page), VB_GAME_TRACKER_PAGE_OFFSET, GAME_TRACKER)->numPlyrCurrGame)
#define VB_VIEW_TYPE                         struct GameTracker
#define VB_VIEW_FROM_OFFSET(offset)          ((struct GameTracker *)((u8 *)GAME_TRACKER + (offset)))
#define VB_VIEW_PUSH_BUFFER(view)            ((view)->pushBuffer[0])
#define VB_PUSH_BUFFER_FROM_OFFSET(offset)   (((struct GameTracker *)((u8 *)GAME_TRACKER + (offset)))->pushBuffer[0])

// NOTE(aalhendi): GCC 2.8.1 needs this otherwise unused lifetime to preserve
// the retail row-state spill homes and setup schedule without emitted code.
#define VB_MATCH_ROW_ALLOCATION_BEGIN(value) __asm__ volatile("" : "=g"(value))
// NOTE(aalhendi): The shared C produces retail's row-preheader instructions,
// but GCC schedules those independent instructions differently. Replace only
// that preheader with retail's order. $L245 is GCC's generated loop target and
// must be updated if surrounding source or headers renumber local labels.
#define VB_MATCH_ROW_SCHEDULE_BEGIN()        __asm__ volatile(".if 0")
#define VB_MATCH_ROW_SCHEDULE_END()                                      \
	__asm__ volatile(".endif\n\t"                                        \
	                 "lui $30,%hi(" RETAIL_GAME_TRACKER_ASM_NAME ")\n\t" \
	                 "sw $3,132($sp)\n\t"                                \
	                 "addu $9,$3,-2\n\t"                                 \
	                 "sw $9,136($sp)\n\t"                                \
	                 "slt $9,$3,3\n\t"                                   \
	                 "move $22,$20\n\t"                                  \
	                 "sw $9,140($sp)\n\t"                                \
	                 "li $9,30\n\t"                                      \
	                 "sw $9,144($sp)\n\t"                                \
	                 "li $9,5\n\t"                                       \
	                 "sw $9,148($sp)\n\t"                                \
	                 "$L245:")
#define VB_MATCH_ROW_SETUP_ORDER(rowCount, configIndex) __asm__ volatile("" : "+g"(rowCount), "+g"(configIndex))
#define VB_MATCH_ROW_ALLOCATION_END(value)              __asm__ volatile("" : : "g"(value))

// NOTE(aalhendi): Keep the hand-scheduled retail preheader tied to the shared
// C constants. GCC 2.8.1 supports this array-bound assertion even though
// CTR_STATIC_ASSERT intentionally emits nothing for that compiler.
#define VB_VALIDATE_MATCHING_CONSTANTS()                 \
	typedef char vb_matching_constants_must_match_retail \
	    [(VB_MIN_PLAYERS == 2 && VB_STANDINGS_EXPANDED_MIN_ENTRIES == 3 && VB_ROW_INITIAL_DELAY_FRAMES == 30 && VB_ROW_STAGGER_FRAMES == 5) ? 1 : -1];

// NOTE(aalhendi): Retail passes this packed color by address at this call site.
// Native copies the resident u32 through its adapter.
extern Color vb_battleColor asm("sdata_static+1228");

#define VB_BATTLE_COLOR_PTR        (&vb_battleColor)
#define VB_DRAW_OUTER_RECT         RECTMENU_DrawOuterRect_HighLevel
#define VB_DRAW_POLY_FT4           DecalHUD_DrawPolyFT4

#endif

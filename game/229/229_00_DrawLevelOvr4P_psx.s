# Matching-only NTSC-U 926 4-player level renderer.
# Native builds use the portable renderer implementation in this overlay.
# NOTE(aalhendi): The shared template owns the PSX implementation.

.include "DrawLevelOvr_split.inc"
DRAW_LEVEL_OVR_SPLIT DrawLevelOvr4P, 4, R229

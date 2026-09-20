# Matching-only NTSC-U 926 1-player level renderer.
# Native builds use the portable renderer implementation in this overlay.
# NOTE(aalhendi): The shared template owns the PSX implementation.

.include "DrawLevelOvr_full.inc"
DRAW_LEVEL_OVR_FULL DrawLevelOvr1P, 1, R226

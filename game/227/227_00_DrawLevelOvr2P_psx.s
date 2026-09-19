# Matching-only NTSC-U 926 2-player level renderer.
# Native builds use the portable renderer implementation in this overlay.
# NOTE(aalhendi): The shared template owns the PSX implementation.

.include "DrawLevelOvr_full.inc"
DRAW_LEVEL_OVR_FULL DrawLevelOvr2P, 2, R227

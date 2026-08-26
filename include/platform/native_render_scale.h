#ifndef NATIVE_RENDER_SCALE_H
#define NATIVE_RENDER_SCALE_H

// Internal-only integer scale for the MAIN render target. This is a build-time
// experiment knob, not a user-facing option: default 1 keeps every runtime
// behavior of the shipped renderer, including the VRAM-roundtrip presentation.
// A proof build passes -DNATIVE_RENDER_SCALE=2.
//
// Coordinate spaces (the contract every helper below serves):
//
//   PSX display   activeDispEnv.disp, ~320x240. Logical space: game vertices,
//                 draw-area clips, AP HUD coordinates all live here. Never
//                 multiplied by the scale.
//   Main target   The RGBA framebuffer normal draws land in. Its raster is
//                 (display * scale). Projection still maps logical display
//                 coordinates onto the full viewport, so geometry scales in
//                 rasterization, not in game code. Scissor and clear rects
//                 must be scaled into this space.
//   Offscreen     Warp/heat/clock feedback targets. Semantically PSX-sized;
//                 the scale never applies to them.
//   VRAM          The persistent 1024x512 RG8 texture of packed 15-bit PSX
//                 pixels. Always PSX-sized rects. Loading the main target
//                 FROM VRAM samples a PSX-sized source rect into the scaled
//                 target; packing the main target INTO VRAM downsamples back
//                 to the PSX-sized rect (the pack quad samples the whole
//                 source with normalized UVs, so destination viewport size
//                 alone decides the decimation).
//   Host window   The presentation viewport (letterboxed). At scale 1 the
//                 frame reaches it through the VRAM roundtrip exactly as
//                 shipped; at scale > 1 the main target is blitted to it
//                 directly, skipping the 15-bit PSX downsample for the
//                 presented image only.
// The helpers below are pinned by tools/test-render-scale.c (standalone:
// gcc -Wall -Wextra tools/test-render-scale.c && ./a.out).
#ifndef NATIVE_RENDER_SCALE
#define NATIVE_RENDER_SCALE 1
#endif

#define NATIVE_RENDER_SCALE_MAX 8

static inline int NativeRenderScale_Factor(void)
{
	int scale = NATIVE_RENDER_SCALE;
	if (scale < 1)
	{
		scale = 1;
	}
	if (scale > NATIVE_RENDER_SCALE_MAX)
	{
		scale = NATIVE_RENDER_SCALE_MAX;
	}
	return scale;
}

// Main render target raster size for a logical PSX display dimension.
static inline int NativeRenderScale_MainTargetDim(int logicalDim, int scale)
{
	if (logicalDim <= 0)
	{
		return logicalDim;
	}
	return logicalDim * scale;
}

// VRAM source rect dimension when seeding the main target from VRAM. This is
// deliberately the logical dimension, NOT the target dimension: a 320x240
// display loaded into a 640x480 target must still sample 320x240 texels of
// VRAM. (Using the target dimension here reproduces the rejected black-screen
// class of bugs; the unit test pins this.)
static inline int NativeRenderScale_MainVramSourceDim(int logicalDim, int scale)
{
	(void)scale;
	return logicalDim;
}

// Scale one scissor/clear coordinate from logical display space into main
// target space. Callers apply it to x, y, w and h alike. Takes float and
// multiplies BEFORE truncating: for the float-valued scissor math this keeps
// any sub-pixel rounding error at its shipped one-logical-pixel magnitude
// instead of magnifying it by the scale, and at scale 1 it truncates exactly
// like the implicit float-to-GLint conversion the shipped glScissor call did.
static inline int NativeRenderScale_ScissorCoord(float logicalCoord, int scale)
{
	return (int)(logicalCoord * (float)scale);
}

// Executable documentation of the pack-shader downsample (not called by the
// GPU path, which does this arithmetic in the shader): packing a source of
// srcDim texels through a destination viewport of dstDim pixels samples, for
// destination index i, the source texel floor((i + 0.5) / dstDim * srcDim).
// For a 2x source this decimates to the odd texels: 640 -> 320 picks 2i + 1.
static inline int NativeRenderScale_PackSampleIndex(int dstIndex, int dstDim, int srcDim)
{
	if ((dstDim <= 0) || (srcDim <= 0))
	{
		return 0;
	}
	// Integer form of floor((dstIndex + 0.5) * srcDim / dstDim).
	return (int)(((2 * (long)dstIndex + 1) * srcDim) / (2 * (long)dstDim));
}

// Executable documentation of the pack flipY: GL framebuffer row r of an
// h-row target lands in VRAM row (h - 1 - r), converting bottom-up GL storage
// to the top-left-origin VRAM layout.
static inline int NativeRenderScale_PackFlipRow(int row, int height)
{
	return height - 1 - row;
}

#endif

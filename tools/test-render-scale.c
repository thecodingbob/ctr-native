#include <stdio.h>

#include "../include/platform/native_render_scale.h"

static int checks;
static int failures;

static void expect(int condition, const char *name)
{
	checks++;
	if (!condition)
	{
		failures++;
		printf("FAIL: %s\n", name);
	}
}

int main(void)
{
	// Scale factor clamping.
	expect(NativeRenderScale_Factor() >= 1, "factor is at least 1");
	expect(NativeRenderScale_Factor() <= NATIVE_RENDER_SCALE_MAX, "factor is clamped");

	// Main target raster: 320x240 at scale 2 is 640x480; scale 1 is identity.
	expect(NativeRenderScale_MainTargetDim(320, 2) == 640, "320 scales to 640");
	expect(NativeRenderScale_MainTargetDim(240, 2) == 480, "240 scales to 480");
	expect(NativeRenderScale_MainTargetDim(320, 1) == 320, "scale 1 leaves width alone");
	expect(NativeRenderScale_MainTargetDim(512, 3) == 1536, "wide menu display scales too");
	expect(NativeRenderScale_MainTargetDim(0, 2) == 0, "degenerate dimension passes through");
	expect(NativeRenderScale_MainTargetDim(-1, 2) == -1, "negative dimension passes through");

	// VRAM source rect for the main-target seed: ALWAYS the logical display
	// dimension. Mutation proof: an implementation that hands the (scaled)
	// target dimension to the VRAM sampler -- the rejected black-screen class
	// of bug -- fails these.
	expect(NativeRenderScale_MainVramSourceDim(320, 2) == 320, "VRAM source stays 320 at scale 2");
	expect(NativeRenderScale_MainVramSourceDim(240, 2) == 240, "VRAM source stays 240 at scale 2");
	expect(NativeRenderScale_MainVramSourceDim(320, 2) != NativeRenderScale_MainTargetDim(320, 2),
	       "VRAM source differs from target raster at scale 2");
	expect(NativeRenderScale_MainVramSourceDim(320, 1) == NativeRenderScale_MainTargetDim(320, 1),
	       "VRAM source equals target raster at scale 1");

	// Scissor/clear magnification into target pixels. Mutation proof: leaving
	// the box unscaled fails these; the origin stays fixed so a scaled empty
	// box stays empty.
	expect(NativeRenderScale_ScissorCoord(8.0f, 2) == 16, "scissor x scales");
	expect(NativeRenderScale_ScissorCoord(37.0f, 3) == 111, "odd scissor extent scales exactly");
	expect(NativeRenderScale_ScissorCoord(0.0f, 4) == 0, "origin is preserved");
	expect(NativeRenderScale_ScissorCoord(240.0f, 1) == 240, "scale 1 is identity");
	expect(NativeRenderScale_ScissorCoord(100.0f, 2) != 100, "unscaled scissor is not accepted at scale 2");
	// Truncation-order proof: multiply first, then truncate. A truncate-first
	// mutation would give 79 * 2 = 158 and magnify the shipped sub-pixel error.
	expect(NativeRenderScale_ScissorCoord(79.75f, 2) == 159, "fractional coordinate truncates after scaling");
	// Scale 1 must truncate exactly like the shipped implicit float-to-GLint
	// conversion at glScissor.
	expect(NativeRenderScale_ScissorCoord(79.75f, 1) == 79, "scale 1 keeps shipped truncation");

	// Pack downsample characterization (executable documentation of the pack
	// quad's normalized-UV nearest sampling): a 640-wide source packed through
	// a 320-wide destination viewport samples the odd texels.
	expect(NativeRenderScale_PackSampleIndex(0, 320, 640) == 1, "first packed pixel samples texel 1");
	expect(NativeRenderScale_PackSampleIndex(1, 320, 640) == 3, "second packed pixel samples texel 3");
	expect(NativeRenderScale_PackSampleIndex(319, 320, 640) == 639, "last packed pixel samples texel 639");
	expect(NativeRenderScale_PackSampleIndex(5, 320, 320) == 5, "same-size pack is the identity");
	// Non-integer ratio: 480 destination pixels over a 640 source (4:3 of the
	// scaled width) walk the source without ever leaving its bounds.
	expect(NativeRenderScale_PackSampleIndex(0, 480, 640) == 0, "non-integer ratio starts in bounds");
	expect(NativeRenderScale_PackSampleIndex(479, 480, 640) == 639, "non-integer ratio ends in bounds");
	expect(NativeRenderScale_PackSampleIndex(240, 480, 640) == 320, "non-integer ratio midpoint maps to midpoint");

	// Vertical orientation of the pack (flipY): GL bottom-up rows land in
	// top-left-origin VRAM rows.
	expect(NativeRenderScale_PackFlipRow(0, 240) == 239, "bottom GL row becomes last VRAM row");
	expect(NativeRenderScale_PackFlipRow(239, 240) == 0, "top GL row becomes first VRAM row");
	expect(NativeRenderScale_PackFlipRow(120, 240) == 119, "flip is an involution around the centre");

	printf("%d checks, %d failures\n", checks, failures);
	return failures != 0;
}

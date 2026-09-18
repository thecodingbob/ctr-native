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
	// Mode ladder clamping: 0 is Native, everything else snaps into 1..4.
	expect(NativeRenderScale_ClampMode(0) == 0, "native mode survives the clamp");
	expect(NativeRenderScale_ClampMode(1) == 1, "original mode survives the clamp");
	expect(NativeRenderScale_ClampMode(4) == 4, "4x survives the clamp");
	expect(NativeRenderScale_ClampMode(7) == 4, "hand-edited 7 clamps to 4x");
	expect(NativeRenderScale_ClampMode(-3) == 1, "hand-edited negative clamps to original");

	// Present-path selection: only Original keeps the shipped VRAM roundtrip.
	expect(!NativeRenderScale_ModeUsesDirectPresent(1), "original presents through VRAM");
	expect(NativeRenderScale_ModeUsesDirectPresent(2), "2x presents directly");
	expect(NativeRenderScale_ModeUsesDirectPresent(4), "4x presents directly");
	expect(NativeRenderScale_ModeUsesDirectPresent(0), "native presents directly");
	expect(!NativeRenderScale_ModeUsesDirectPresent(-3), "clamped junk presents through VRAM");

	// Fixed-multiple target raster: 320x240 at mode 2 is 640x480; mode 1 is the
	// identity; the viewport argument is ignored outside Native mode.
	expect(NativeRenderScale_TargetDimForMode(320, 2, 9999) == 640, "320 scales to 640 at 2x");
	expect(NativeRenderScale_TargetDimForMode(240, 2, 9999) == 480, "240 scales to 480 at 2x");
	expect(NativeRenderScale_TargetDimForMode(512, 4, 0) == 2048, "512 scales to 2048 at 4x");
	expect(NativeRenderScale_TargetDimForMode(320, 1, 9999) == 320, "original leaves width alone");
	expect(NativeRenderScale_TargetDimForMode(0, 2, 9999) == 0, "degenerate dimension passes through");
	expect(NativeRenderScale_TargetDimForMode(-1, 2, 9999) == -1, "negative dimension passes through");

	// Native mode: raster follows the presentation viewport but never drops
	// below the logical raster (a smaller-than-PSX target would upsample into
	// the PSX-sized VRAM pack and corrupt feedback semantics).
	expect(NativeRenderScale_TargetDimForMode(512, 0, 1440) == 1440, "native follows the viewport");
	expect(NativeRenderScale_TargetDimForMode(216, 0, 1080) == 1080, "native follows the viewport height");
	expect(NativeRenderScale_TargetDimForMode(512, 0, 300) == 512, "tiny window clamps to the logical raster");
	expect(NativeRenderScale_TargetDimForMode(512, 0, 0) == 512, "unsized viewport clamps to the logical raster");

	// Scissor/clear magnification into target pixels, per axis. Mutation
	// proofs: leaving the box unscaled fails the ratio cases; the origin stays
	// fixed; target == logical is the exact identity (Original mode, offscreen
	// targets); a zero/unsized dimension falls back to shipped coordinates.
	expect(NativeRenderScale_ScaleAxisCoord(8.0f, 512, 1024) == 16, "x doubles against a 2x-wide target");
	expect(NativeRenderScale_ScaleAxisCoord(37.0f, 216, 648) == 111, "odd extent triples exactly");
	expect(NativeRenderScale_ScaleAxisCoord(0.0f, 512, 2048) == 0, "origin is preserved");
	expect(NativeRenderScale_ScaleAxisCoord(240.0f, 240, 240) == 240, "identity when target equals logical");
	expect(NativeRenderScale_ScaleAxisCoord(100.0f, 512, 1024) != 100, "unscaled scissor is not accepted");
	expect(NativeRenderScale_ScaleAxisCoord(100.0f, 512, 1440) == 281, "native-mode fractional ratio truncates after scaling");
	expect(NativeRenderScale_ScaleAxisCoord(100.0f, 512, 0) == 100, "unsized target falls back to shipped coordinates");
	expect(NativeRenderScale_ScaleAxisCoord(100.0f, 0, 1024) == 100, "degenerate logical falls back to shipped coordinates");
	// Truncation-order proof: multiply first, then truncate. A truncate-first
	// mutation would give 79 * 2 = 158 and magnify the shipped sub-pixel error.
	expect(NativeRenderScale_ScaleAxisCoord(79.75f, 512, 1024) == 159, "fractional coordinate truncates after scaling");
	// Identity ratio must truncate exactly like the shipped implicit
	// float-to-GLint conversion at glScissor.
	expect(NativeRenderScale_ScaleAxisCoord(79.75f, 512, 512) == 79, "identity ratio keeps shipped truncation");

	// Pack downsample characterization (executable documentation of the pack
	// quad's normalized-UV nearest sampling): a 640-wide source packed through
	// a 320-wide destination viewport samples the odd texels.
	expect(NativeRenderScale_PackSampleIndex(0, 320, 640) == 1, "first packed pixel samples texel 1");
	expect(NativeRenderScale_PackSampleIndex(1, 320, 640) == 3, "second packed pixel samples texel 3");
	expect(NativeRenderScale_PackSampleIndex(319, 320, 640) == 639, "last packed pixel samples texel 639");
	expect(NativeRenderScale_PackSampleIndex(5, 320, 320) == 5, "same-size pack is the identity");
	// Non-integer ratio (Native mode): destination pixels walk the source
	// without ever leaving its bounds.
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

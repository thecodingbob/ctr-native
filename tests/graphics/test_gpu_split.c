#include <shared/ctr_test.h>
#include <SDL3/SDL.h>
#include <common.h>
#include <platform/native_memtrack.h>

// Compile the dispatcher in its owning source, without extracting production code.
#include <platform/native_gpu.c>

struct sData sdata_static;
NativeConfig g_config;
int g_cfg_bilinearFiltering;

// The rest of the GPU facade is not exercised by this dispatch-only fixture.
int NativePgxp_Lookup(const void *addr, u32 value, NativePgxpVertex *out)
{
	(void)addr; (void)value; (void)out; CtrTest_Bail("unexpected vertex lookup"); return 0;
}
const struct PlatformMempackArena *Platform_GetMempackArena(void)
{
	CtrTest_Bail("unexpected memory lookup"); return NULL;
}
int NativeGpuLinks_IsRegisteredHostRange(const void *pointer, size_t size)
{
	(void)pointer; (void)size; CtrTest_Bail("unexpected OT lookup"); return 0;
}
int NativeGpuLinks_IsRegisteredHostPointer(const void *pointer)
{
	(void)pointer; CtrTest_Bail("unexpected OT lookup"); return 0;
}
int NativeGpuLinks_IsTerminator(u32 token) { (void)token; CtrTest_Bail("unexpected OT traversal"); return 0; }
void *NativeGpuLinks_ToHostPointer(u32 token) { (void)token; CtrTest_Bail("unexpected OT traversal"); return NULL; }
int NativeRenderer_GetVRAMStateSize(void) { CtrTest_Bail("unexpected VRAM snapshot"); return 0; }
int NativeRenderer_CaptureVRAMState(void *dst, int size)
{
	(void)dst; (void)size; CtrTest_Bail("unexpected VRAM snapshot"); return 0;
}
int NativeRenderer_RestoreVRAMState(const void *src, int size)
{
	(void)src; (void)size; CtrTest_Bail("unexpected VRAM restore"); return 0;
}
b32 NativeTextVertices_Set(const void *primitive, float left, float right)
{
	(void)primitive; (void)left; (void)right; CtrTest_Bail("unexpected text emission"); return 0;
}
b32 NativeTextVertices_Get(const void *primitive, float *left, float *right)
{
	(void)primitive; (void)left; (void)right; CtrTest_Bail("unexpected text parsing"); return 0;
}
void NativeRenderer_UpdateVRAM(void) { CtrTest_Bail("unexpected batch submission"); }
void NativeRenderer_UpdateVertexBuffer(const GrVertex *vertices, int count)
{
	(void)vertices; (void)count; CtrTest_Bail("unexpected vertex upload");
}
TextureID NativeRenderer_GetVRAMTexture(void) { CtrTest_Bail("unexpected primitive parsing"); return 0; }
TextureID NativeRenderer_GetWhiteTexture(void) { CtrTest_Bail("unexpected primitive parsing"); return 0; }
void NativeRenderer_StoreFrameBuffer(int x, int y, int w, int h)
{
	(void)x; (void)y; (void)w; (void)h; CtrTest_Bail("unexpected framebuffer copy");
}
int LoadImage(RECT16 *rect, void *src) { (void)rect; (void)src; CtrTest_Bail("unexpected VRAM upload"); return 0; }
int ClearImage(RECT16 *rect, u8 r, u8 g, u8 b)
{
	(void)rect; (void)r; (void)g; (void)b; CtrTest_Bail("unexpected VRAM clear"); return 0;
}
int MoveImage(RECT16 *rect, int x, int y)
{
	(void)rect; (void)x; (void)y; CtrTest_Bail("unexpected VRAM move"); return 0;
}
void Platform_LogError(const char *format, ...) { (void)format; CtrTest_Bail("unexpected GPU error"); }
void NativeRenderer_ApplyTimeStopPalette(void) { CtrTest_Bail("unexpected post-processing"); }

enum { WHITE_TEXTURE = 1, VRAM_TEXTURE = 2 };
static TextureID s_boundTexture;
static TextureID s_expectedTexture;
static int s_targetW;
static int s_targetH;
static int s_draws;
static int s_resizes;
static int s_semiTransPass;
static int s_debugDepth;
static int s_offscreen;

void NativeRenderer_SetStencilMode(int mode) { (void)mode; }
void NativeRenderer_SetupClipMode(const RECT16 *clip, const DISPENV *display, int enable)
{
	(void)clip; (void)display; (void)enable;
}
void NativeRenderer_SetOffscreenState(const RECT16 *rect, int enable)
{
	s_offscreen = enable;
	if (enable && (s_targetW != rect->w || s_targetH != rect->h))
	{
		// Model EnsureRenderTarget's glBindTexture/glTexImage2D/unbind and
		// binding-cache invalidation, followed by LoadRenderTargetFromVRAM.
		s_boundTexture = 0;
		s_targetW = rect->w;
		s_targetH = rect->h;
		s_resizes++;
	}
}
void NativeRenderer_SetTexture(TextureID texture, TexFormat format, int allowAnisotropic)
{
	(void)format; (void)allowAnisotropic;
	s_boundTexture = texture;
}
void NativeRenderer_SetOverrideTextureSize(int width, int height)
{
	CTR_TEST_CHECK(width == 128 && height == 64 && s_boundTexture == s_expectedTexture,
	               "native override uniforms follow target selection and texture binding");
}
void NativeRenderer_SetPSXDrawMaskSet(int enabled) { (void)enabled; }
void NativeRenderer_SetPSXTextureOutputSTP(int enabled) { (void)enabled; }
void NativeRenderer_SetProjection(const RECT16 *rect, const DISPENV *display, int offscreen)
{
	(void)rect; (void)display;
	CTR_TEST_CHECK_EQ(offscreen, s_offscreen, "projection uses the selected render target");
}
void NativeRenderer_SetBlendMode(BlendMode blend) { (void)blend; }
void NativeRenderer_SetPSXTextureSemiTransPass(int pass) { s_semiTransPass = pass; }
void NativeRenderer_DrawTriangles(int first, int triangles)
{
	CTR_TEST_CHECK(first == 6 && triangles == 2, "the split retains its vertex range");
	CTR_TEST_CHECK_EQ(s_boundTexture, s_expectedTexture,
	                  "target resizing cannot discard the sampler needed by the clear or model");
	s_draws++;
}
void NativeRenderer_PushDebugLabel(const char *label) { (void)label; s_debugDepth++; }
void NativeRenderer_PopDebugLabel(void) { s_debugDepth--; }

static GPUDrawSplit MakeSplit(int width, int height, TextureID texture)
{
	GPUDrawSplit split = {0};
	split.drawenv.clip = (RECT16){512, 256, width, height};
	split.dispenv.disp = (RECT16){0, 0, 512, 216};
	split.textureId = texture;
	split.texFormat = TF_16_BIT;
	split.blendMode = BM_NONE;
	split.startVertex = 6;
	split.numVerts = 6;
	return split;
}

static void DrawOffscreen(int width, int height)
{
	GPUDrawSplit clear = MakeSplit(width, height, WHITE_TEXTURE);
	s_expectedTexture = WHITE_TEXTURE;
	DrawSplit(&clear);
	GPUDrawSplit model = MakeSplit(width, height, VRAM_TEXTURE);
	s_expectedTexture = VRAM_TEXTURE;
	DrawSplit(&model);
}

int main(void)
{
	CTR_TEST_SECTION("Wumpa-to-kart resizing rebinds the sampler before clearing");
	for (int frame = 0; frame < 3; frame++)
	{
		DrawOffscreen(32, 24);
		DrawOffscreen(96, 64); // first refreshed kart in P1
		DrawOffscreen(96, 64); // later karts/viewports reuse the target size
	}
	CTR_TEST_CHECK_EQ(s_resizes, 6, "each Wumpa-to-kart frame exercises both target resizes");
	CTR_TEST_CHECK_EQ(s_draws, 18, "every clear and model is still drawn");

	CTR_TEST_SECTION("paused frames with no Wumpa resize preserve the same draw contract");
	DrawOffscreen(96, 64);
	CTR_TEST_CHECK_EQ(s_resizes, 6, "drawing more karts does not resize the target");

	CTR_TEST_SECTION("RGBA overrides and two-pass transparency keep their state");
	GPUDrawSplit override = MakeSplit(128, 64, 3);
	override.texFormat = TF_32_BIT_RGBA;
	override.drawenv.tw.w = 128;
	override.drawenv.tw.h = 64;
	override.debugText = "override";
	s_expectedTexture = 3;
	DrawSplit(&override);
	GPUDrawSplit transparent = MakeSplit(96, 64, VRAM_TEXTURE);
	transparent.psxTexturedSemiTrans = true;
	transparent.blendMode = BM_AVERAGE;
	s_expectedTexture = VRAM_TEXTURE;
	int before = s_draws;
	DrawSplit(&transparent);
	CTR_TEST_CHECK_EQ(s_draws - before, 2, "STP textures still draw both transparency passes");
	CTR_TEST_CHECK_EQ(s_semiTransPass, 0, "the transparency pass is reset after drawing");
	CTR_TEST_CHECK_EQ(s_debugDepth, 0, "debug labels stay balanced");

	CTR_TEST_SECTION("empty clips leave offscreen mode without drawing");
	transparent.drawenv.clip.w = 0;
	transparent.debugText = "empty";
	before = s_draws;
	DrawSplit(&transparent);
	CTR_TEST_CHECK_EQ(s_draws, before, "empty clips emit no geometry");
	CTR_TEST_CHECK_EQ(s_offscreen, 0, "empty clips drop stale offscreen mode");
	CTR_TEST_CHECK_EQ(s_debugDepth, 0, "empty-clip labels stay balanced");
	return CtrTest_Summary();
}

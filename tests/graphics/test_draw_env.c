#include <shared/ctr_test.h>
#include <common.h>

#include <platform/native_libgpu.c>
#include <game/PushBuffer.c>

DRAWENV activeDrawEnv;
DISPENV activeDispEnv;
int g_GPUDisabledState;
struct sData sdata_static;
NativeConfig g_config;
u8 *gCTRNativeScratchpadBase;
static const void *s_linkedPacket;

// The suite exercises environment emission, not the other PushBuffer geometry paths.
void CTC2(u32 value, int reg) { (void)value; (void)reg; CtrTest_Bail("unexpected GTE write"); }
void MTC2(u32 value, int reg) { (void)value; (void)reg; CtrTest_Bail("unexpected GTE write"); }
void MTC2_S(int value, int reg) { (void)value; (void)reg; CtrTest_Bail("unexpected GTE write"); }
u32 MFC2(int reg) { (void)reg; CtrTest_Bail("unexpected GTE read"); return 0; }
int MFC2_S(int reg) { (void)reg; CtrTest_Bail("unexpected GTE read"); return 0; }
int doCOP2(int op) { (void)op; CtrTest_Bail("unexpected GTE operation"); return 0; }
int SquareRoot0_stub(int value) { (void)value; CtrTest_Bail("unexpected frustum calculation"); return 0; }
int Widescreen_GetFactor(void) { CtrTest_Bail("unexpected projection calculation"); return 1000; }
void ConvertRotToMatrix(MATRIX *matrix, const SVec3 *rot)
{
	(void)matrix; (void)rot; CtrTest_Bail("unexpected matrix calculation");
}
void NativePgxp_LoadTransform(const void *key, const s16 *rotation, const s32 *translation, int bank)
{
	(void)key; (void)rotation; (void)translation; (void)bank; CtrTest_Bail("unexpected PGXP transform");
}
int NativePgxp_GetPosition(const void *key, const s16 *vector, float *out)
{
	(void)key; (void)vector; (void)out; CtrTest_Bail("unexpected PGXP position"); return 0;
}
void NativePgxp_CameraRotation(const float *angles, double *rotation)
{
	(void)angles; (void)rotation; CtrTest_Bail("unexpected PGXP rotation");
}
void NativePgxp_SetTransform(const void *key, const s16 *rotation, const s32 *translation, const double *preciseRotation,
                             const double *preciseTranslation)
{
	(void)key; (void)rotation; (void)translation; (void)preciseRotation; (void)preciseTranslation;
	CtrTest_Bail("unexpected PGXP transform");
}

// Packet construction must stay deferred: no renderer, window or VRAM I/O.
void NativeRenderer_Clear(int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	(void)x; (void)y; (void)w; (void)h; (void)r; (void)g; (void)b;
	CtrTest_Bail("draw environment construction must not clear immediately");
}
void NativeRenderer_ClearVRAM(int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	NativeRenderer_Clear(x, y, w, h, r, g, b);
}
void NativeRenderer_CopyVRAM(u16 *src, int x, int y, int w, int h, int dstX, int dstY)
{
	(void)src; (void)x; (void)y; (void)w; (void)h; (void)dstX; (void)dstY;
	CtrTest_Bail("packet construction must not copy VRAM");
}
void NativeRenderer_ReadVRAM(u16 *dst, int x, int y, int w, int h)
{
	(void)dst; (void)x; (void)y; (void)w; (void)h;
	CtrTest_Bail("packet construction must not read VRAM");
}
void NativeRenderer_UpdateVRAM(void) { CtrTest_Bail("packet construction must not upload VRAM"); }
int NativeGpu_HasPendingSplits(void) { return 0; }
void DrawAllSplits(void) { CtrTest_Bail("packet construction must not draw"); }
void ClearSplits(void) { CtrTest_Bail("packet construction must not reset splits"); }
int Platform_BeginScene(void) { CtrTest_Bail("packet construction must not open a scene"); return 0; }
void Platform_EndScene(void) { CtrTest_Bail("packet construction must not close a scene"); }
int GetVideoMode(void) { return MODE_NTSC; }
void ParsePrimitivesLinkedList(u32 *p, int single)
{
	(void)p; (void)single;
	CtrTest_Bail("packet construction must not submit primitives");
}
void NativePgxp_EndFrame(void) { CtrTest_Bail("packet construction must not end a frame"); }
void NativeTextVertices_EndFrame(void) { CtrTest_Bail("packet construction must not end a text frame"); }
uint32_t NativeGpuLinks_FromHostPointerAt(const void *pointer, const char *file, int line, const char *function)
{
	(void)file; (void)line; (void)function;
	s_linkedPacket = pointer;
	return 1;
}

static void CheckBackground(int x, int y, int width, int height, int offsetX, int offsetY)
{
	struct DB db = {0};
	union
	{
		DR_ENV packet;
		u8 allocation[0x40];
	} primitiveMemory = {0};
	u32 ot = 0;
	RECT viewport = {x, y, width, height};
	SetDefDrawEnv(&db.drawEnv, 0, 0, 512, 216);
	CTR_TEST_CHECK_EQ(db.drawEnv.dfe, 1, "the real main framebuffer starts with on-screen rendering enabled");
	db.primMem.cursor = &primitiveMemory;
	db.primMem.guardEnd = (u8 *)&primitiveMemory + sizeof(primitiveMemory);
	db.otMem.start = &ot;
	db.otMem.cursor = &ot + 1;
	s_linkedPacket = NULL;
	PushBuffer_SetDrawEnv_DecalMP(&ot, &db, &viewport, offsetX, offsetY, 0, 0, 0, 0, 1);
	const DR_ENV *packet = &primitiveMemory.packet;
	CTR_TEST_CHECK(s_linkedPacket == packet && ot == 1, "the actual offscreen environment is linked to its ordering table");
	// Inheriting dfe=1 sends this VRAM-only viewport to the main target and
	// clips away both the clear and the model. This must test the real producer.
	CTR_TEST_CHECK_EQ(packet->code[3] & 0x400, 0, "Wumpa and kart environments select the offscreen render target");
	CTR_TEST_CHECK_EQ(db.drawEnv.dfe, 1, "emitting an offscreen environment does not change the main target");
	CTR_TEST_CHECK_EQ(packet->tag >> 24, 9, "background environments contain the clear TILE");
	CTR_TEST_CHECK_EQ(packet->code[5], 0xe6000000u, "mask state is reset before the background clear");
	CTR_TEST_CHECK_EQ(packet->code[6], 0x60000000u, "black backgrounds use an opaque untextured TILE");
	CTR_TEST_CHECK_EQ((s16)packet->code[7] + offsetX, x, "clear left edge includes the drawing offset");
	CTR_TEST_CHECK_EQ((s16)(packet->code[7] >> 16) + offsetY, y, "clear top edge includes the drawing offset");
	CTR_TEST_CHECK_EQ((int)(packet->code[8] & 0xffff), width, "clear width matches the viewport");
	CTR_TEST_CHECK_EQ((int)(packet->code[8] >> 16), height, "clear height matches the viewport");
}

int main(void)
{
	CTR_TEST_SECTION("multiplayer Wumpa and kart offscreen backgrounds");
	CheckBackground(0x3e0, 0x180, 32, 24, 0x3e0 + 16 - 256, 0x180 + 12 - 108);
	CheckBackground(0x320, 0x100, 32, 24, 0x320 + 16 - 256, 0x100 + 12 - 108);
	CheckBackground(0x200, 0x180, 96, 64, 0x200 - 80, 0x180 - 40);

	CTR_TEST_SECTION("general draw environment packet contract");
	DRAWENV env = {0};
	DR_ENV packet = {0};
	env.clip = (RECT){0, 0, 512, 216};
	env.dtd = 1;
	env.dfe = 1;
	env.tpage = 0x123;
	SetDrawEnv(&packet, &env);
	CTR_TEST_CHECK(sizeof(packet) >= sizeof(u32) * 10, "the extended packet fits the draw environment allocation");
	CTR_TEST_CHECK_EQ(packet.tag >> 24, 6, "environments without a background do not emit a TILE");
	CTR_TEST_CHECK_EQ(packet.code[3], 0xe1000723u, "drawing mode precedes texture-window and mask state");
	CTR_TEST_CHECK_EQ(packet.code[4] >> 24, 0xe2, "texture window retains its GPU command");
	CTR_TEST_CHECK_EQ(packet.code[5], 0xe6000000u, "environments without a background also reset mask state");
	env.isbg = 1;
	env.r0 = 12;
	env.g0 = 34;
	env.b0 = 56;
	SetDrawEnv(&packet, &env);
	CTR_TEST_CHECK_EQ(packet.code[6], 0x6038220cu, "background TILE preserves all RGB components");
	env.clip.w = -1;
	env.clip.h = -1;
	SetDrawEnv(&packet, &env);
	CTR_TEST_CHECK_EQ(packet.code[8], 0, "negative background dimensions are clamped to zero");
	env.clip.w = VRAM_WIDTH;
	env.clip.h = VRAM_HEIGHT;
	SetDrawEnv(&packet, &env);
	CTR_TEST_CHECK_EQ(packet.code[8], (VRAM_WIDTH - 1) | ((VRAM_HEIGHT - 1) << 16),
	                  "background dimensions stay within GPU limits");
	return CtrTest_Summary();
}

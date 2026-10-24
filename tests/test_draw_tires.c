#include <shared/ctr_test.h>
#include <common.h>
#include <math.h>

NativeConfig g_config;
struct sData sdata_static;

#include "platform/native_pgxp.c"
#include "platform/native_gte_core.c"
#include "platform/native_inline_c.c"
#include "game/DrawTires.c"

// Asset/physics and OT allocation are outside this geometry fixture.
void TRIG_AngleSinCos_r9r8r10(u32 angle, s32 *sine, s32 *cosine)
{
	angle &= ANG_TWO_PI - 1;
	*sine = (int)(sin(angle * 6.283185307179586 / 4096.0) * 4096);
	*cosine = (int)(cos(angle * 6.283185307179586 / 4096.0) * 4096);
}

int VehPhysForce_ScaleValue(const struct Driver *driver, int value)
{
	(void)driver;
	return value;
}

u32 NativeGpuLinks_FromHostPointerAt(const void *pointer, const char *file, int line, const char *function)
{
	(void)pointer;
	(void)file;
	(void)line;
	(void)function;
	return 1;
}

static void CheckMatrixBank(const MATRIX *matrix, int bank)
{
	const u32 expected[] = {
		CTR_PackS16Pair(matrix->m[0][0], matrix->m[0][1]),
		CTR_PackS16Pair(matrix->m[0][2], matrix->m[1][0]),
		CTR_PackS16Pair(matrix->m[1][1], matrix->m[1][2]),
		CTR_PackS16Pair(matrix->m[2][0], matrix->m[2][1]),
		(u32)(s32)matrix->m[2][2],
	};
	for (int word = 0; word < 5; word++)
	{
		CTR_TEST_CHECK_EQ(CFC2(bank + word), expected[word], "signed matrix components reach their GTE bank intact");
	}
}

static void TestMatrixLoads(void)
{
	CTR_TEST_SECTION("solid and reflected wheels load all three matrix banks");
	struct Instance inst = {0};
	struct InstDrawPerPlayer idpp = {0};
	struct PushBuffer pb = {0};
	struct DrawTiresScratch scratch = {0};
	// Distinct signed elements catch missing halfwords, wrong offsets and aliasing
	// of the s16 writes with the packed word reads in optimized builds.
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			int value = (row * 3 + col + 1) * 211;
			inst.matrix.m[row][col] = (s16)value;
			idpp.m3x3.m[row][col] = (s16)-value;
			pb.matrix_ViewProj.m[row][col] = (s16)(value - 2000);
		}
	}
	inst.matrix.t[0] = 100;
	inst.matrix.t[1] = -200;
	inst.matrix.t[2] = 300;

	for (int reflected = 0; reflected <= 1; reflected++)
	{
		memset(&gteRegs, 0, sizeof(gteRegs));
		if (reflected)
		{
			DrawTiresReflection_SetupGteState(&scratch, &inst, &idpp, &pb);
		}
		else
		{
			DrawTiresSolid_SetupGteState(&scratch, &inst, &idpp, &pb);
		}
		CheckMatrixBank(&inst.matrix, 16);
		CheckMatrixBank(&idpp.m3x3, 8);
		CTR_TEST_CHECK_EQ(CFC2_S(5), 400, "camera-relative X translation survives");
		CTR_TEST_CHECK_EQ(CFC2_S(6), -800, "camera-relative Y translation survives");
		CTR_TEST_CHECK_EQ(CFC2_S(7), 1200, "camera-relative Z translation survives");
		if (reflected)
		{
			DrawTiresReflection_SetupProjectionState(&pb);
		}
		else
		{
			DrawTiresSolid_SetupProjectionState(&pb);
		}
		CheckMatrixBank(&pb.matrix_ViewProj, 0);
	}
}

static int AxesAndProjectionStayBounded(int reflected)
{
	struct Instance inst = {0};
	struct InstDrawPerPlayer idpp = {0};
	struct PushBuffer pb = {0};
	struct Icon sprite = {0};
	struct Icon *sprites[17];
	u32 ot = 0;
	POLY_FT4 primitives[5];
	for (int i = 0; i < 17; i++)
	{
		sprites[i] = &sprite;
	}
	inst.matrix.m[0][0] = inst.matrix.m[1][1] = inst.matrix.m[2][2] = 4096;
	idpp.m3x3.m[0][0] = idpp.m3x3.m[1][1] = idpp.m3x3.m[2][2] = 4096;
	pb.matrix_ViewProj.m[0][0] = pb.matrix_ViewProj.m[1][1] = pb.matrix_ViewProj.m[2][2] = 4096;
	pb.rect.w = 512;
	pb.rect.h = reflected ? 216 : 240;
	pb.distanceToScreen_PREV = 256;
	for (int z = 1000; z <= 30000; z += 1000)
	{
		for (int angle = 0; angle < 4096; angle += 17)
		{
			struct DrawTiresScratch scratch = {0};
			scratch.wheelSize.word = 0xccc;
			scratch.wheelSprites = sprites;
			scratch.otRangeNormal = scratch.otRangeSecondary = (int)(uintptr_t)&ot;
			struct PrimMem primMem = {0};
			primMem.cursor = primitives;
			primMem.guardEnd = primitives + 5;
			int primCount = 0;
			inst.matrix.t[0] = 100;
			inst.matrix.t[1] = 200;
			inst.matrix.t[2] = z / 4;
			for (int i = 0; i < 4; i++)
			{
				scratch.wheelLocal[i].rim.x = (s16)(cos(angle * 6.283185307179586 / 4096) * 4096);
				scratch.wheelLocal[i].rim.z.word = (int)(sin(angle * 6.283185307179586 / 4096) * 4096);
			}
			if (reflected)
			{
				DrawTiresReflection_SetupGteState(&scratch, &inst, &idpp, &pb);
				DrawTiresReflection_BuildWheelAxes(&scratch);
				DrawTiresReflection_SetupProjectionState(&pb);
				DrawTiresReflection_ProjectWheelQuads(&scratch, &primMem, &primCount);
			}
			else
			{
				DrawTiresSolid_SetupGteState(&scratch, &inst, &idpp, &pb);
				DrawTiresSolid_BuildWheelAxes(&scratch);
				DrawTiresSolid_SetupProjectionState(&pb);
				DrawTiresSolid_ProjectWheelQuads(&scratch, &primMem, &primCount);
			}
			SVec3Slot a = scratch.tireAxisA[0];
			SVec3Slot b = scratch.tireAxisB[0];
			int lengthA = a.x * a.x + a.y * a.y + a.z * a.z;
			int lengthB = b.x * b.x + b.y * b.y + b.z * b.z;
			if (lengthA < 25 * 25 || lengthA > 80 * 80 || lengthB < 25 * 25 || lengthB > 80 * 80)
			{
				printf("reflection=%d z=%d angle=%d A=(%d,%d,%d) B=(%d,%d,%d)\n", reflected, z, angle,
				       a.x, a.y, a.z, b.x, b.y, b.z);
				return 0;
			}
			// Check projection coordinates independently of primitive emission.
			for (int corner = 0; corner < 4; corner++)
			{
				int x = (s16)scratch.projectedSxy[corner];
				int y = (s16)((u32)scratch.projectedSxy[corner] >> 16);
				if (abs(x - pb.rect.w / 2) > 160 || abs(y - pb.rect.h / 2) > 300)
				{
					printf("reflection=%d z=%d angle=%d corner=(%d,%d)\n", reflected, z, angle, x, y);
					return 0;
				}
			}
		}
	}
	return 1;
}

static void CheckSmallQuad(const POLY_FT4 *p, const struct PushBuffer *pb)
{
	const int xs[] = {p->x0, p->x1, p->x2, p->x3};
	const int ys[] = {p->y0, p->y1, p->y2, p->y3};
	for (int corner = 0; corner < 4; corner++)
	{
		CTR_TEST_CHECK(abs(xs[corner] - pb->rect.w / 2) < 32, "distant wheel stays attached to the racer horizontally");
		CTR_TEST_CHECK(abs(ys[corner] - pb->rect.h / 2) < 32, "distant wheel stays attached to the racer vertically");
		CTR_TEST_CHECK(abs(xs[corner] - xs[0]) <= 4, "distant wheel does not stretch across the screen");
		CTR_TEST_CHECK(abs(ys[corner] - ys[0]) <= 4, "distant wheel keeps its projected height");
	}
}

static void TestIssue57Scene(void)
{
	CTR_TEST_SECTION("issue #57: distant racer from the 2026-10-24 quick state");
	// Scalar geometry extracted from the report, without save-state pointers or
	// disc assets. The old code emitted corners at (1023,1023) beside (245,116).
	struct Instance inst = {0};
	inst.matrix = (MATRIX){
		{{2439, 19, 3287}, {568, 4031, -446}, {-3237, 722, 2399}},
		{-7757, 185, 4752},
	};
	inst.scale.x = inst.scale.y = inst.scale.z = 3276;
	struct InstDrawPerPlayer idpp = {0};
	idpp.m3x3 = (MATRIX){
		{{256, 1, 387}, {59, 402, -53}, {-341, 72, 282}}, {0, 0, 0},
	};
	struct PushBuffer pb = {0};
	pb.matrix_ViewProj = (MATRIX){
		{{131, 18, -3068}, {-16, -2303, -15}, {-4092, 31, -175}}, {0, 0, 0},
	};
	pb.pos.x = 1266;
	pb.pos.y = 631;
	pb.pos.z = 4635;
	pb.distanceToScreen_PREV = 256;
	struct Driver driver = {0};
	driver.wheelSize = 3276;
	driver.hazardTimer = -17762;
	struct Icon sprite = {0};
	struct Icon *sprites[17];
	for (int i = 0; i < 17; i++)
	{
		sprites[i] = &sprite;
	}
	u32 ot = 0;
	idpp.otRangeNormal = idpp.otRangeSecondary = (int)(uintptr_t)&ot;
	const int viewportSizes[][2] = {{512, 216}, {512, 108}, {256, 108}};
	for (int mode = PGXP_MODE_OFF; mode <= PGXP_MODE_PERSPECTIVE; mode++)
	{
		for (int viewport = 0; viewport < 3; viewport++)
		{
			pb.rect.w = viewportSizes[viewport][0];
			pb.rect.h = viewportSizes[viewport][1];
			for (int reflected = 0; reflected <= 1; reflected++)
			{
				NativePgxp_EndFrame();
				g_config.pgxpMode = mode;
				NativePgxp_SetWorldPhase(1);
				struct DrawTiresScratch scratch = {0};
				scratch.wheelSprites = sprites;
				POLY_FT4 primitives[5] = {0};
				struct PrimMem primMem = {0};
				primMem.cursor = primitives;
				primMem.guardEnd = primitives + 5;
				int primCount = 0;
				if (reflected)
				{
					DrawTiresReflection_BuildWheelLocalPairs(&scratch, &driver, &inst, &idpp);
					DrawTiresReflection_SetupGteState(&scratch, &inst, &idpp, &pb);
					DrawTiresReflection_BuildWheelAxes(&scratch);
					DrawTiresReflection_SetupProjectionState(&pb);
					DrawTiresReflection_ProjectWheelQuads(&scratch, &primMem, &primCount);
				}
				else
				{
					DrawTiresSolid_BuildWheelLocalPairs(&scratch, &driver, &inst, &idpp);
					DrawTiresSolid_SetupGteState(&scratch, &inst, &idpp, &pb);
					DrawTiresSolid_BuildWheelAxes(&scratch);
					DrawTiresSolid_SetupProjectionState(&pb);
					DrawTiresSolid_ProjectWheelQuads(&scratch, &primMem, &primCount);
					CTR_TEST_CHECK_EQ(scratch.wheelCenters[0].x, -35896, "wheel center keeps the unsaturated transform result");
				}
				CTR_TEST_CHECK_EQ(primCount, 4, "all four wheels are emitted instead of hiding malformed quads");
				CTR_TEST_CHECK(primMem.cursor == primitives + 4, "wheel emission advances the primitive cursor exactly four packets");
				for (int wheel = 0; wheel < primCount && wheel < 4; wheel++)
				{
					POLY_FT4 *p = &primitives[wheel];
					CheckSmallQuad(p, &pb);
					if (mode != PGXP_MODE_OFF)
					{
						NativePgxpVertex precise;
						u32 xy = CTR_PackS16Pair(p->x0, p->y0);
						int found = NativePgxp_Lookup(&p->x0, xy, &precise);
						CTR_TEST_CHECK(found, "wide projection carries precise coordinates into the wheel packet");
						if (found)
						{
							CTR_TEST_CHECK(precise.w > 32768, "PGXP keeps the actual distant depth");
							CTR_TEST_CHECK(fabs(precise.x - p->x0) < 2 && fabs(precise.y - p->y0) < 2,
							               "precise wheel corners also stay beside the integer projection");
						}
					}
				}
			}
		}
	}
	NativePgxp_EndFrame();
	g_config.pgxpMode = PGXP_MODE_OFF;
}

static void TestSignedCoordinateBoundaries(void)
{
	CTR_TEST_SECTION("wheel corners cross either signed-16-bit limit on every world axis");
	const int distances[] = {32760, 32768, 32776, 40000};
	for (int mode = PGXP_MODE_OFF; mode <= PGXP_MODE_PERSPECTIVE; mode++)
	{
		g_config.pgxpMode = mode;
		for (int axis = 0; axis < 3; axis++)
		{
			for (int sign = -1; sign <= 1; sign += 2)
			{
				struct PushBuffer pb = {0};
				pb.rect.w = 512;
				pb.rect.h = 216;
				pb.distanceToScreen_PREV = 256;
				int screenAxis = (axis + 1) % 3;
				int verticalAxis = (axis + 2) % 3;
				pb.matrix_ViewProj.m[2][axis] = (s16)(sign * 4096);
				pb.matrix_ViewProj.m[0][screenAxis] = 4096;
				pb.matrix_ViewProj.m[1][verticalAxis] = 4096;
				DrawTiresSolid_SetupProjectionState(&pb);
				for (int distance = 0; distance < 4; distance++)
				{
					struct DrawTiresScratch scratch = {0};
					s32 center[3] = {0};
					s16 axisA[3] = {0};
					s16 axisB[3] = {0};
					center[axis] = sign * distances[distance];
					center[screenAxis] = 100;
					center[verticalAxis] = 200;
					axisA[axis] = 50;
					axisA[screenAxis] = 20;
					axisB[verticalAxis] = 40;
					scratch.wheelCenters[0] = (Vec3){center[0], center[1], center[2]};
					scratch.tireAxisA[0] = (SVec3Slot){axisA[0], axisA[1], axisA[2], 0};
					scratch.tireAxisB[0] = (SVec3Slot){axisB[0], axisB[1], axisB[2], 0};
					for (int a = -1; a <= 1; a += 2)
					{
						for (int b = -1; b <= 1; b += 2)
						{
							u32 xy = DrawTires_ProjectCorner(&scratch, 0, a, b);
							CTR_TEST_CHECK(abs((s16)xy - 256) <= 2 && abs((s16)(xy >> 16) - 108) <= 3,
							               "crossing a coordinate limit keeps every wheel corner near the racer");
							if (mode != PGXP_MODE_OFF)
							{
								NativePgxpVertex precise = {0};
								NativePgxp_StoreGteSXY(&xy, 14, xy);
								CTR_TEST_CHECK(NativePgxp_Lookup(&xy, xy, &precise), "boundary projection retains its precision shadow");
								CTR_TEST_CHECK_NEAR(precise.w, distances[distance] + sign * a * 50,
								                    "boundary projection uses the full signed input depth");
							}
						}
					}
				}
			}
		}
	}
	NativePgxp_EndFrame();
	g_config.pgxpMode = PGXP_MODE_OFF;
}

static void TestWideProjectionCompatibility(void)
{
	CTR_TEST_SECTION("wide projection preserves RTPS results inside the signed-16-bit range");
	const SVec3 positions[] = {{17, -23, 1000}, {1200, 2300, 30000}, {-32000, -2000, 1000}, {0, 0, -500}, {32767, -32768, 2000}};
	struct PushBuffer pb = {0};
	pb.rect.w = 512;
	pb.rect.h = 216;
	pb.distanceToScreen_PREV = 256;
	pb.matrix_ViewProj = (MATRIX){
		{{3996, 0, 899}, {0, 4096, 0}, {-899, 0, 3996}}, {0, 0, 0},
	};
	for (int mode = PGXP_MODE_OFF; mode <= PGXP_MODE_PERSPECTIVE; mode++)
	{
		for (int i = 0; i < (int)(sizeof(positions) / sizeof(positions[0])); i++)
		{
			NativePgxp_EndFrame();
			g_config.pgxpMode = mode;
			memset(&gteRegs, 0, sizeof(gteRegs));
			DrawTiresSolid_SetupProjectionState(&pb);
			CTC2(99, 5);
			CTC2((u32)-40, 6);
			CTC2(1500, 7);
			CTC2((u32)-98, 27);
			CTC2(0x100000, 28);
			CTR_GteLoadSVec3V0(&positions[i]);
			GTERegisters initial = gteRegs;
			gte_rtps_b();
			GTERegisters expected = gteRegs;
			u32 packed = MFC2(14);
			NativePgxpVertex precise = {0};
			if (mode != PGXP_MODE_OFF)
			{
				NativePgxp_StoreGteSXY(&packed, 14, packed);
				CTR_TEST_CHECK(NativePgxp_Lookup(&packed, packed, &precise), "ordinary RTPS has a precision shadow");
			}
			gteRegs = initial;
			NativePgxp_EndFrame();
			// A wide input is explicit: unrelated V0 precision must not replace it.
			const float stale[] = {5000, 6000, 7000};
			NativePgxp_GteSetInput(0, &positions[i].x, stale);
			NativeGte_ProjectWide(positions[i].x, positions[i].y, positions[i].z);
			for (int reg = 8; reg <= 27; reg++)
			{
				CTR_TEST_CHECK_EQ(gteRegs.CP2D.p[reg].d, expected.CP2D.p[reg].d, "wide RTPS preserves IR, screen/depth FIFOs and MAC results");
			}
			CTR_TEST_CHECK_EQ(CFC2(31), expected.CP2C.p[31].d, "wide RTPS preserves projection flags");
			if (mode != PGXP_MODE_OFF)
			{
				u32 result = MFC2(14);
				NativePgxpVertex actual = {0};
				NativePgxp_StoreGteSXY(&result, 14, result);
				CTR_TEST_CHECK(NativePgxp_Lookup(&result, result, &actual), "wide RTPS has a precision shadow");
				CTR_TEST_CHECK_NEAR(actual.x, precise.x, "wide RTPS preserves precise X");
				CTR_TEST_CHECK_NEAR(actual.y, precise.y, "wide RTPS preserves precise Y");
				CTR_TEST_CHECK_NEAR(actual.w, precise.w, "wide RTPS preserves perspective depth");
			}
		}
	}
	NativePgxp_EndFrame();
	g_config.pgxpMode = PGXP_MODE_OFF;
}

int main(void)
{
	setbuf(stdout, NULL);
	TestMatrixLoads();
	TestIssue57Scene();
	TestSignedCoordinateBoundaries();
	TestWideProjectionCompatibility();
	CTR_TEST_SECTION("wheel geometry stays bounded across 7230 distance/angle combinations per pass");
	CTR_TEST_CHECK(AxesAndProjectionStayBounded(0), "solid wheel axes and unfiltered projections stay bounded");
	CTR_TEST_CHECK(AxesAndProjectionStayBounded(1), "reflected wheel axes and unfiltered projections stay bounded");
	return CtrTest_Summary();
}

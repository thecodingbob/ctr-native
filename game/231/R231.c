#include <common.h>

// NOTE(aalhendi): Short-name reads span the following buffer; preserve their shared
// backing storage. The longer buffers have independent addresses in retail code.
char rb_namePrefix[52] CTR_PSX_MATCH_SECTION(".RB_names") = "nitro\0\0\0"
                                                            "tnt1\0\0\0\0"
                                                            "shieldbomb\0\0"
                                                            "cloud1";
char rb_nameShatter[28] CTR_PSX_MATCH_SECTION(".RB_names") = "shatter1";
char rb_nameExplosion[28] CTR_PSX_MATCH_SECTION(".RB_names") = "explosion1";
char rb_nameBlowup[24] CTR_PSX_MATCH_SECTION(".RB_names") = "blowup";
char rb_nameShockwave[28] CTR_PSX_MATCH_SECTION(".RB_names") = "shockwave1";
char rb_nameBurstExplosion[28] CTR_PSX_MATCH_SECTION(".RB_names") = "explosion2";

struct OverlayRDATA_231 R231 =
    {
        .emSet_Missile =
            {
                {
                    .flags = 1,
                    .initOffset = 0xc,
                    .InitTypes = {.FuncInit = {.particle_funcPtr = NULL, .particle_colorFlags = 0, .particle_lifespan = 5, .particle_Type = 1}},
                },
                {
                    .flags = 1,
                    .initOffset = 0,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}},
                },
                {
                    .flags = 1,
                    .initOffset = 1,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}},
                },
                {
                    .flags = 1,
                    .initOffset = 2,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}},
                },
                {
                    .flags = 1,
                    .initOffset = 7,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x8000}}},
                },
                {
                    .flags = 1,
                    .initOffset = 8,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x8000}}},
                },
                {
                    .flags = 1,
                    .initOffset = 9,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x8000}}},
                },
                {
                    .flags = 3,
                    .initOffset = 3,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x2000, .velocity = 0x666}}},
                },
                {
                    .flags = 3,
                    .initOffset = 4,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x1800, .velocity = 0x666}}},
                },
                {
                    .flags = 3,
                    .initOffset = 5,
                    .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x1000, .velocity = 0x6cc}}},
                },
                {0},
            },
        .warpballParticleHeight = {0, 27, 59, 92, 126, 160, 189, 216, 237, 251, 255},
        .warpballFadeScale =
            {
                {4505, 5120, 4096},
                {5226, 8192, 4096},
                {5600, 5501, 4096},
                {5272, 3183, 4096},
                {4242, 1411, 3337},
                {2878, 437, 1668},
            },
        .warpballFadeY = {-64, -256, -87, 57, 167, 228},
        .maskPosArr =
            {
                0, 0, -2, -4, -8, -12, -16, -19, -23, -27, -29, -31, -32, -31, -29, -27, -23, -19, -16, -12, -8, -4, -2, 0,
            },
        .shieldGrowScale =
            {
                {977, 1835},
                {1792, 2936},
                {2205, 2095},
                {2335, 1254},
                {1884, 1612},
                {1433, 1971},
                {1612, 1881},
                {1792, 1792},
            },
        .shieldPopScale =
            {
                {2150, 1612},
                {2419, 1433},
                {2508, 1344},
                {2329, 1523},
                {1792, 1792},
                {1254, 2150},
                {896, 2419},
                {716, 2508},
                {537, 2150},
                {358, 1254},
                {179, 537},
            },
        .shieldPulseScale =
            {
                {1845, 1756},
                {1899, 1720},
                {1971, 1684},
                {1899, 1720},
                {1845, 1756},
                {1792, 1792},
            },
        .emSet_PotionShatter =
            {
                {.flags = 1, .initOffset = 0xc, .InitTypes = {.FuncInit = {.particle_colorFlags = 0xa1, .particle_lifespan = 20}}},
                {.flags = 1, .initOffset = 0, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}}},
                {.flags = 1, .initOffset = 2, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}}},
                {.flags = 0x17,
                 .initOffset = 1,
                 .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1, .velocity = 0xed8, .accel = -280}, .rngSeed = {.velocity = 400}}}},
                {.flags = 1, .initOffset = 5, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0x1000}}}},
                {.flags = 1, .initOffset = 7, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}}},
                {.flags = 1, .initOffset = 8, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 0xc800}}}},
                {.flags = 1, .initOffset = 9, .InitTypes = {.AxisInit = {.baseValue = {.startVal = 1}}}},
                {0},
            },
};

#ifndef NATIVE_CONFIG_H
#define NATIVE_CONFIG_H

typedef struct {
    int skipIntro;  // 0 = off (default), 1 = skip all intros
    int skipHints;  // 0 = off (default), 1 = skipp all mask hints in adventure mode
} NativeConfig;

extern NativeConfig g_config;

void NativeConfig_Load(void);

#endif
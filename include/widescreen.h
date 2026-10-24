#ifndef WIDESCREEN_H
#define WIDESCREEN_H

int Widescreen_GetFactor(void);
int Widescreen_XShift(int width);

void Widescreen_CompressFT4(POLY_FT4 *p);
void Widescreen_CompressGT4(POLY_GT4 *p);
void Widescreen_CompressGT4Scaled(POLY_GT4 *p, int unscaledWidth, int scale);
void Widescreen_CompressNative(PolyFT4 *p);

#endif

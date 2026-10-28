#ifndef PS25_FONT_H
#define PS25_FONT_H
#include "gfx.h"

/* Own compact 5x7 glyph program; horizontal runs go straight to GS sprites. */
void font_text(Gfx *g, float x, float y, float size, u64 color, const char *str);
int font_width(const char *str, float size);
#endif

#ifndef PS25_GFX_H
#define PS25_GFX_H
#include <gsKit.h>

/* All layout coordinates are in a 1280x720 virtual 16:9 space. */
#define PS25_VW 1280.0f
#define PS25_VH 720.0f

typedef struct {
    GSGLOBAL *gs;
    float sx;
    float sy;
} Gfx;

int gfx_init(Gfx *g);
void gfx_begin(Gfx *g);
void gfx_end(Gfx *g);
u64 gfx_rgba(int r, int gg, int b, int alpha);
void gfx_rect(Gfx *g, float x, float y, float w, float h, u64 c);
void gfx_line(Gfx *g, float x1, float y1, float x2, float y2, float thickness, u64 c);
void gfx_round(Gfx *g, float x, float y, float w, float h, float radius, u64 c);
void gfx_circle(Gfx *g, float x, float y, float r, u64 c);
void gfx_gradient(Gfx *g, float x, float y, float w, float h, u64 top, u64 bottom);

#endif

#include "gfx.h"
#include <dmaKit.h>
#include <math.h>
#include <string.h>
#if PS25_HIRES
#include <gsHires.h>
#endif

#define PS25_PI 3.14159265358979323846f

int gfx_init(Gfx *g)
{
    memset(g, 0, sizeof(*g));

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
#if PS25_HIRES
    g->gs = gsKit_hires_init_global();
#else
    g->gs = gsKit_init_global();
#endif
    if (!g->gs) return 0;

#if PS25_HIRES
    /* Legacy HIRES three-pass mode kept for A/B diagnostics. */
    g->gs->Mode = GS_MODE_DTV_720P;
    g->gs->Width = 1280;
    g->gs->Height = 720;
    g->gs->PSM = GS_PSM_CT16S;
    g->gs->PSMZ = GS_PSMZ_16S;
#elif PS25_DIRECT720
    /* Standard GS double-buffered 720p: 2 * 1280 * aligned(720,64) * 2
       = 3.75 MiB. No texture RAM or Z-buffer required. */
    g->gs->Mode = GS_MODE_DTV_720P;
    g->gs->Width = 1280;
    g->gs->Height = 720;
    g->gs->PSM = GS_PSM_CT16;
    g->gs->PSMZ = GS_PSMZ_16;
#else
    /* Separate, conventional 480p build: useful for PCSX2 and recovery. */
    g->gs->Mode = GS_MODE_DTV_480P;
    g->gs->Width = 704;
    g->gs->Height = 480;
    g->gs->PSM = GS_PSM_CT16;
    g->gs->PSMZ = GS_PSMZ_16;
#endif
    g->gs->Interlace = GS_NONINTERLACED;
    g->gs->Field = GS_FRAME;
    g->gs->Dithering = GS_SETTING_ON;
    g->gs->DoubleBuffering = PS25_SINGLE720 ? GS_SETTING_OFF : GS_SETTING_ON;
    g->gs->ZBuffering = GS_SETTING_OFF;
    g->gs->PrimAlphaEnable = GS_SETTING_ON;
#if PS25_HIRES
    /* gsKit's high-resolution multipass scanout; REAL PS2 only. */
    gsKit_hires_init_screen(g->gs, 3);
#else
    gsKit_init_screen(g->gs);
#endif
    gsKit_set_test(g->gs, GS_ZTEST_OFF);
    /* Standard source-over alpha, with PS2's 0x80 = opaque convention. */
    gsKit_set_primalpha(g->gs, GS_SETREG_ALPHA(0, 1, 0, 1, 128), 0);
    g->sx = (float)g->gs->Width / PS25_VW;
    g->sy = (float)g->gs->Height / PS25_VH;
    return 1;
}

u64 gfx_rgba(int r, int g, int b, int a)
{
    return GS_SETREG_RGBAQ(r, g, b, a, 0);
}

void gfx_begin(Gfx *g)
{
#if PS25_HIRES
    /* gsKit HIRES pass queues already clear each pass. Avoid queuing
       another full-screen clear in its timing-critical draw queue. */
    (void)g;
#else
    gsKit_clear(g->gs, gfx_rgba(3, 9, 23, 128));
#endif
}

void gfx_end(Gfx *g)
{
#if PS25_HIRES
    gsKit_hires_sync(g->gs);
    gsKit_hires_flip(g->gs);
#else
    gsKit_queue_exec(g->gs);
    gsKit_sync_flip(g->gs);
#endif
}

void gfx_rect(Gfx *g, float x, float y, float w, float h, u64 c)
{
    if (w <= 0 || h <= 0) return;
    gsKit_prim_sprite(g->gs, x*g->sx, y*g->sy,
                      (x+w)*g->sx, (y+h)*g->sy, 1, c);
}

void gfx_line(Gfx *g, float x1, float y1, float x2, float y2, float thickness, u64 c)
{
    gsKit_prim_line(g->gs, x1*g->sx, y1*g->sy,
                    x2*g->sx, y2*g->sy, thickness*g->sy, c);
}

void gfx_circle(Gfx *g, float x, float y, float r, u64 c)
{
    int i;
    for (i=0; i<16; ++i) {
        float a = (2.0f*PS25_PI*i)/16.0f;
        float b = (2.0f*PS25_PI*(i+1))/16.0f;
        gsKit_prim_triangle(g->gs,
            x*g->sx, y*g->sy,
            (x+cosf(a)*r)*g->sx, (y+sinf(a)*r)*g->sy,
            (x+cosf(b)*r)*g->sx, (y+sinf(b)*r)*g->sy, 1, c);
    }
}

void gfx_round(Gfx *g, float x, float y, float w, float h, float r, u64 c)
{
    if (w <= 0 || h <= 0) return;
    if (r < 1) { gfx_rect(g,x,y,w,h,c); return; }
    if (r > w/2) r = w/2;
    if (r > h/2) r = h/2;
    gfx_rect(g,x+r,y,w-2*r,h,c);
    gfx_rect(g,x,y+r,r,h-2*r,c);
    gfx_rect(g,x+w-r,y+r,r,h-2*r,c);
    /* Corners made from triangles, never sampled from texture RAM. */
    for (int corner=0; corner<4; corner++) {
        float cx=x+(corner&1?w-r:r);
        float cy=y+(corner&2?h-r:r);
        float base=(corner==0?PS25_PI:corner==1?1.5f*PS25_PI:
                    corner==2?0.5f*PS25_PI:0.0f);
        for (int n=0; n<5; n++) {
            float a=base+(PS25_PI*0.5f*n)/5.0f;
            float b=base+(PS25_PI*0.5f*(n+1))/5.0f;
            gsKit_prim_triangle(g->gs,cx*g->sx,cy*g->sy,
                (cx+cosf(a)*r)*g->sx,(cy+sinf(a)*r)*g->sy,
                (cx+cosf(b)*r)*g->sx,(cy+sinf(b)*r)*g->sy,1,c);
        }
    }
}

void gfx_gradient(Gfx *g,float x,float y,float w,float h,u64 top,u64 bottom)
{
    gsKit_prim_quad_gouraud(g->gs,
        x*g->sx,y*g->sy,
        x*g->sx,(y+h)*g->sy,
        (x+w)*g->sx,y*g->sy,
        (x+w)*g->sx,(y+h)*g->sy,1,
        top,bottom,top,bottom);
}

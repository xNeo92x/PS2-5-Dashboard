/* PS2-5 standalone GS diagnostic.
   No controller, SIF/IOP module, filesystem or disk access.
   Stays on-screen indefinitely, never calls reset/reboot. */
#include <gsKit.h>
#include <dmaKit.h>

#ifndef PS25_DIAG_VIDEO
#define PS25_DIAG_VIDEO 0
#endif

int main(int argc,char **argv)
{
    GSGLOBAL *gs;
    u64 background, accent, white, dark;
    int w, h;
    (void)argc;
    (void)argv;
    dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF,D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF,D_CTRL_RCYC_8,1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    gs=gsKit_init_global();
    if(!gs)for(;;){}

#if PS25_DIAG_VIDEO == 0
    gs->Mode=GS_MODE_NTSC;
    gs->Interlace=GS_INTERLACED;
    gs->Field=GS_FIELD;
    w=640;h=448;
#elif PS25_DIAG_VIDEO == 1
    gs->Mode=GS_MODE_PAL;
    gs->Interlace=GS_INTERLACED;
    gs->Field=GS_FIELD;
    w=640;h=512;
#else
    gs->Mode=GS_MODE_DTV_480P;
    gs->Interlace=GS_NONINTERLACED;
    gs->Field=GS_FRAME;
    w=704;h=480;
#endif

#if PS25_DIAG_VIDEO == 2
    background=GS_SETREG_RGBAQ(5,58,52,0x80,0);
    accent=GS_SETREG_RGBAQ(40,235,155,0x80,0);
#else
    background=GS_SETREG_RGBAQ(14,47,125,0x80,0);
    accent=GS_SETREG_RGBAQ(248,154,25,0x80,0);
#endif
    white=GS_SETREG_RGBAQ(255,255,255,0x80,0);
    dark=GS_SETREG_RGBAQ(4,15,32,0x80,0);
    gs->Width=w;
    gs->Height=h;
    gs->PSM=GS_PSM_CT16;
    gs->Dithering=GS_SETTING_OFF;
    gs->DoubleBuffering=GS_SETTING_OFF;
    gs->ZBuffering=GS_SETTING_OFF;
    gs->PrimAlphaEnable=GS_SETTING_OFF;
    gsKit_init_screen(gs);
    gsKit_mode_switch(gs,GS_ONESHOT);
    gsKit_set_test(gs,GS_ZTEST_OFF);
    for(;;) {
        gsKit_clear(gs,background);
        gsKit_prim_sprite(gs,28,26,w-28,40,1,white);
        gsKit_prim_sprite(gs,28,h-40,w-28,h-26,1,white);
        gsKit_prim_sprite(gs,58,98,w-58,h-91,1,dark);
        gsKit_prim_sprite(gs,83,123,w-83,h-118,1,accent);
        gsKit_prim_sprite(gs,w/2-35,h/2-35,w/2+35,h/2+35,1,white);
        gsKit_queue_exec(gs);
        gsKit_sync_flip(gs);
    }
}

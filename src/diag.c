/* PS2-5 standalone GS signal/memory diagnostic.
   No controller, SIF/IOP modules, filesystem, disk reads or disk writes.
   Continues drawing forever; terminate with the PS2 reset button.
   Modes 3-7 test 720p output with different framebuffer configurations. */
#include <gsKit.h>
#include <dmaKit.h>
#include <stdio.h>

#ifndef PS25_DIAG_VIDEO
#define PS25_DIAG_VIDEO 0
#endif

int main(int argc,char **argv)
{
    GSGLOBAL *gs;
    int w,h;
    u64 colors[6],white,black;
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
    w=640; h=448;
#elif PS25_DIAG_VIDEO == 1
    gs->Mode=GS_MODE_PAL;
    gs->Interlace=GS_INTERLACED;
    gs->Field=GS_FIELD;
    w=640; h=512;
#elif PS25_DIAG_VIDEO == 2
    gs->Mode=GS_MODE_DTV_480P;
    gs->Interlace=GS_NONINTERLACED;
    gs->Field=GS_FRAME;
    w=704; h=480;
#else
    gs->Mode=GS_MODE_DTV_720P;
    gs->Interlace=GS_NONINTERLACED;
    gs->Field=GS_FRAME;
#if PS25_DIAG_VIDEO == 3
    /* Deliberate 640x360 framebuffer magnified 2x by the 720p CRTC. */
    w=640; h=360;
#else
    w=1280; h=720;
#endif
#endif

    gs->Width=w;
    gs->Height=h;
#if PS25_DIAG_VIDEO == 6
    gs->PSM=GS_PSM_CT32;
#elif PS25_DIAG_VIDEO == 7
    gs->PSM=GS_PSM_CT16S;
#else
    gs->PSM=GS_PSM_CT16;
#endif
    gs->Dithering=GS_SETTING_OFF;
#if PS25_DIAG_VIDEO == 5
    gs->DoubleBuffering=GS_SETTING_ON;
#else
    gs->DoubleBuffering=GS_SETTING_OFF;
#endif
    gs->ZBuffering=GS_SETTING_OFF;
    gs->PrimAlphaEnable=GS_SETTING_OFF;

    gsKit_init_screen(gs);
    gsKit_mode_switch(gs,GS_ONESHOT);
    gsKit_set_test(gs,GS_ZTEST_OFF);

    printf("PS25 video diagnostic %d: %dx%d PSM=%d double=%d VRAM=%lu / 4194304 bytes\n",
           PS25_DIAG_VIDEO,w,h,gs->PSM,gs->DoubleBuffering,
           (unsigned long)gs->CurrentPointer);

    colors[0]=GS_SETREG_RGBAQ(21,59,129,0x80,0);  /* dark blue */
    colors[1]=GS_SETREG_RGBAQ(34,106,213,0x80,0); /* saturated blue */
    colors[2]=GS_SETREG_RGBAQ(13,158,151,0x80,0); /* teal */
    colors[3]=GS_SETREG_RGBAQ(234,148,28,0x80,0); /* orange */
    colors[4]=GS_SETREG_RGBAQ(119,53,188,0x80,0); /* purple */
    colors[5]=GS_SETREG_RGBAQ(31,176,65,0x80,0);  /* green */
    white=GS_SETREG_RGBAQ(251,252,255,0x80,0);
    black=GS_SETREG_RGBAQ(5,14,33,0x80,0);
    for(;;) {
        int i;
        /* Six colored horizontal bars must all be visible, including
           the GREEN bar at the VERY BOTTOM. No text or complex assets. */
        gsKit_clear(gs,black);
        for(i=0;i<6;i++) {
            float top=(float)(i*h)/6;
            float bottom=(float)((i+1)*h)/6;
            gsKit_prim_sprite(gs,0,top,w,bottom,1,colors[i]);
        }
        /* Known white vertical rails and a center square. */
        gsKit_prim_sprite(gs,18,18,32,h-18,1,white);
        gsKit_prim_sprite(gs,w-32,18,w-18,h-18,1,white);
        gsKit_prim_sprite(gs,w/2-26,h/2-26,w/2+26,h/2+26,1,white);
        gsKit_queue_exec(gs);
        gsKit_sync_flip(gs);
    }
}

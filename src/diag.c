/* PS2-5 standalone GS signal/memory diagnostic.
   No controller, SIF/IOP modules, filesystem, disk reads or disk writes.
   Continues drawing forever; terminate with the PS2 reset button.
   Modes 3-11 test 720p output with different framebuffer configurations. */
#include <gsKit.h>
#include <dmaKit.h>
#include <stdio.h>
#if PS25_DIAG_PAD
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
static unsigned char s_pad_dma[256] __attribute__((aligned(64)));
static int s_pad_open;
static int s_pad_read;
static void pad_boot(void)
{
    SifInitRpc(0);
    /* Driver names match the known working PS25 dashboard. No MC access. */
    SifLoadModule("rom0:SIO2MAN",0,NULL);
    SifLoadModule("rom0:PADMAN",0,NULL);
    if (padInit(0))
        s_pad_open=padPortOpen(0,0,s_pad_dma)!=0;
}
static void pad_poll(void)
{
    struct padButtonStatus report;
    int state;
    if (!s_pad_open) return;
    state=padGetState(0,0);
    if (state==PAD_STATE_STABLE || state==PAD_STATE_FINDCTP1)
        if (padRead(0,0,&report)) s_pad_read=1;
}
#endif

#ifndef PS25_DIAG_VIDEO
#define PS25_DIAG_VIDEO 0
#endif

int main(int argc,char **argv)
{
    GSGLOBAL *gs;
    int w,h;
#if PS25_DIAG_PAD
    int frames=0;
#endif
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
#elif PS25_DIAG_VIDEO == 13
    /* RIPPS2-style 1080i, GS FRAME -> only 540 active buffer rows. */
    gs->Mode=GS_MODE_DTV_1080I;
    gs->Interlace=GS_INTERLACED;
    gs->Field=GS_FRAME;
    w=640; h=540;
#else
    gs->Mode=GS_MODE_DTV_720P;
    gs->Interlace=GS_NONINTERLACED;
    gs->Field=GS_FRAME;
#if PS25_DIAG_VIDEO == 3
    /* Deliberate 640x360 framebuffer magnified 2x by the 720p CRTC. */
    w=640; h=360;
#elif PS25_DIAG_VIDEO == 12 || PS25_DIAG_VIDEO == 14
    /* RIPPS2 one-pass 720p: horizontal 2x CRTC MAGH, full 720 rows. */
    w=640; h=720;
#elif PS25_DIAG_VIDEO == 8
    /* The official gsKit modetest uses a visible 698-line 720p image. */
    w=1280; h=698;
#elif PS25_DIAG_VIDEO == 9
    /* Last complete 16-bit 64-pixel GS page row before 720p. */
    w=1280; h=704;
#elif PS25_DIAG_VIDEO == 10
    /* Progressive reduction to locate the first failing scanline region. */
    w=1280; h=640;
#elif PS25_DIAG_VIDEO == 11
    /* Force 480 active lines at native 1280 horizontal size. */
    w=1280; h=480;
#else
    w=1280; h=720;
#endif
#endif

    gs->Width=w;
    gs->Height=h;
#if PS25_DIAG_VIDEO == 6
    gs->PSM=GS_PSM_CT32;
#elif PS25_DIAG_VIDEO == 7 || PS25_DIAG_VIDEO == 12 || PS25_DIAG_VIDEO == 13
    gs->PSM=GS_PSM_CT16S;
#else
    gs->PSM=GS_PSM_CT16;
#endif
    gs->Dithering=GS_SETTING_OFF;
#if PS25_DIAG_VIDEO == 5 || PS25_DIAG_VIDEO == 12 || PS25_DIAG_VIDEO == 13 || PS25_DIAG_VIDEO == 14
    gs->DoubleBuffering=GS_SETTING_ON;
#else
    gs->DoubleBuffering=GS_SETTING_OFF;
#endif
    gs->ZBuffering=GS_SETTING_OFF;
    gs->PrimAlphaEnable=GS_SETTING_OFF;

    gsKit_init_screen(gs);
    gsKit_mode_switch(gs,GS_ONESHOT);
    gsKit_set_test(gs,GS_ZTEST_OFF);

    printf("PS25 diag %d: framebuffer %dx%d, output %dx%d, MagH=%d MagV=%d, PSM=%d double=%d VRAM=%lu\n",
           PS25_DIAG_VIDEO,w,h,gs->DW,gs->DH,gs->MagH,gs->MagV,
           gs->PSM,gs->DoubleBuffering,(unsigned long)gs->CurrentPointer);

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
#if PS25_DIAG_PAD
        /* The Retro GEM observes PS2 controller communication. Our original
           video-only diagnostics NEVER polled the pad, so their OSD hotkey
           might not work. Give the GS two displayed frames first; then
           initialize PADMAN once and poll the controller every frame. */
        if (frames==2) pad_boot();
        if (frames>=2) pad_poll();
        if (frames<3) frames++;
#endif
        /* Six colored horizontal bars occupy the configured *active* FB height.
           On <720 configurations GS leaves some outside lines unfilled.
           Green must be visible at the bottom of the colored active area. */
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
#if PS25_DIAG_PAD
        /* Pad indicator at top: orange = no valid read yet, green = valid
           controller reports received. Diagnostic colored stripes unaffected. */
        gsKit_prim_sprite(gs,48,20,110,50,1,
            s_pad_read?colors[5]:colors[3]);
#endif
        gsKit_queue_exec(gs);
        gsKit_sync_flip(gs);
    }
}

/*
 * PS2-5 Dashboard - independent PS2 homebrew proof of concept.
 * No writes to PSBBN or to the HDD/SSD are performed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include "gfx.h"
#include "ui.h"

static unsigned char pad_buffer[256] __attribute__((aligned(64)));
static int pad_available;

static void init_input(void)
{
    SifInitRpc(0);
    /* Module may already be resident; failures are not fatal to video. */
    SifLoadModule("rom0:SIO2MAN",0,NULL);
    SifLoadModule("rom0:PADMAN",0,NULL);
    if(padInit(0)) pad_available=padPortOpen(0,0,pad_buffer)!=0;
}

static void init_memory_cards(void)
{
    SifLoadModule("rom0:MCMAN",0,NULL);
    SifLoadModule("rom0:MCSERV",0,NULL);
}

static unsigned int read_input(void)
{
    struct padButtonStatus b;
    int state;
    if(!pad_available)return 0;
    state=padGetState(0,0);
    if(state!=PAD_STATE_STABLE && state!=PAD_STATE_FINDCTP1)return 0;
    if(!padRead(0,0,&b))return 0;
    return ((unsigned int)b.btns)^0xffffu;
}

/* Read only; never touch partitions. POSIX open goes through PS2SDK. */
static int load_catalog_file(Dashboard *d)
{
    const char *paths[]={"mc0:/PS25/APPS.CFG","mc1:/PS25/APPS.CFG"};
    char buf[8192];
    int i;
    for(i=0;i<2;i++) {
        int fd=open(paths[i],O_RDONLY);
        if(fd>=0){
            int n=read(fd,buf,sizeof(buf)-1);
            close(fd);
            if(n<0)continue;
            buf[n]=0;
            catalog_from_text(&d->catalog,buf);
            d->config_found=1;
            return 1;
        }
    }
    return 0;
}
static int path_is_elf(const char *path)
{
    unsigned char hdr[4];
    int fd;
    if(!path)return 0;
    fd=open(path,O_RDONLY);
    if(fd<0)return 0;
    int count=read(fd,hdr,4);
    close(fd);
    return count==4 && hdr[0]==0x7f && hdr[1]=='E' &&
           hdr[2]=='L' && hdr[3]=='F';
}

int main(int argc,char **argv)
{
    Gfx gfx;
    Dashboard dash;
    unsigned int previous=0, held=0, pressed=0;
    int directional_frames=0;
    int quit=0;
    const char *launch=0;
    (void)argc;(void)argv;

    ui_init(&dash);
    if(!gfx_init(&gfx)) {
        printf("PS25: GS initialization failed\n");
        return 1;
    }
    /* Render before touching the IOP. A visible screen isolates pad/MC issues. */
    ui_toast(&dash,"VIDEO INITIALIZED");
    ui_render(&gfx,&dash);
    dash.frame++;
#if !PS25_DEMO
    ui_toast(&dash,"INITIALIZING CONTROLLER");
    ui_render(&gfx,&dash);
    dash.frame++;
    init_input();
    ui_toast(&dash,"SQUARE: LOAD MEMORY CARD CATALOG");
#else
    ui_toast(&dash,"DEMO: NO IOP OR MEMORY CARD ACCESS");
#endif
#if PS25_HIRES
    printf("PS25: legacy experimental HIRES 720p multipass\n");
#elif PS25_MAGH_INTERLACED
    printf("PS25: RIPPS2-style 640x540 GS frame, 1080i output MAGH=3\n");
#elif PS25_MAGH_HD
    printf("PS25: RIPPS2-style 640x720 GS framebuffer, 720p MAGH=2\n");
#elif PS25_DIRECT720
    printf("PS25: direct double-buffered 720p 16-bit (no HIRES)\n");
#else
    printf("PS25: standard 480p video mode\n");
#endif
    while(1){
        held=PS25_DEMO ? 0 : read_input();
        pressed=held & ~previous;
        if((held & (PAD_LEFT|PAD_RIGHT)) &&
           ((held & (PAD_LEFT|PAD_RIGHT))==
            (previous & (PAD_LEFT|PAD_RIGHT)))) {
            directional_frames++;
            if(directional_frames>=18 && (directional_frames%7)==0)
                pressed|=(held & (PAD_LEFT|PAD_RIGHT));
        } else directional_frames=0;
        previous=held;
        if(!PS25_DEMO && (pressed & PAD_SQUARE)) {
            ui_toast(&dash,"READING MEMORY CARD");
            ui_render(&gfx,&dash);
            dash.frame++;
            init_memory_cards();
            if(load_catalog_file(&dash)) ui_toast(&dash,"CATALOG LOADED");
            else ui_toast(&dash,"NO CATALOG CONFIG ON MC0 / MC1");
        }
        ui_action(&dash,pressed,held,&quit,&launch);
        if(quit) {
            ExecOSD(0,NULL);
        }
        if(launch){
            if(!path_is_elf(launch)){
                ui_toast(&dash,"ELF FILE NOT FOUND OR INVALID");
            } else {
                printf("PS25: attempting to execute %s\n",launch);
                /* Experimental: depends on the PS2's available IOP filesystem. */
                LoadExecPS2(launch,0,NULL);
            }
        }
        if(dash.message_frames>0)dash.message_frames--;
        ui_render(&gfx,&dash);
        dash.frame++;
    }
    return 0;
}

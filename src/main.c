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
    SifLoadModule("rom0:MCMAN",0,NULL);
    SifLoadModule("rom0:MCSERV",0,NULL);
    if(padInit(0)) pad_available=padPortOpen(0,0,pad_buffer)!=0;
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

    init_input();
    ui_init(&dash);
    load_catalog_file(&dash);
    if(!gfx_init(&gfx)) {
        printf("PS25: GS initialization failed\n");
        return 1;
    }
#if PS25_HIRES
    printf("PS25: experimental REAL-HARDWARE 720p hires multipass\n");
#else
    printf("PS25: safe 480p video mode\n");
#endif
    while(1){
        held=read_input();
        pressed=held & ~previous;
        if((held & (PAD_LEFT|PAD_RIGHT)) &&
           ((held & (PAD_LEFT|PAD_RIGHT))==
            (previous & (PAD_LEFT|PAD_RIGHT)))) {
            directional_frames++;
            if(directional_frames>=18 && (directional_frames%7)==0)
                pressed|=(held & (PAD_LEFT|PAD_RIGHT));
        } else directional_frames=0;
        previous=held;
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

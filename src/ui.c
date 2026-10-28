#include "ui.h"
#include "font.h"
#include <libpad.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define COL(r,g,b,a) gfx_rgba(r,g,b,a)
#define WHITE COL(239,247,255,128)
#define MUTED COL(157,181,214,128)
#define SOFT COL(83,116,159,128)
#define CYAN COL(89,197,255,128)
#define BG COL(3,9,23,128)

static const char *const tabs[3]={"GAMES","APPS","SYSTEM"};
static const char *const sys_items[3]={"ABOUT","CONTROLS","EXIT TO BROWSER"};
static const unsigned char shades[6][3]={
    {21,105,211},{29,148,199},{45,92,205},
    {27,148,160},{75,87,192},{33,111,163}
};

static const char *label_for(const Dashboard *d,int tab,int index)
{
    const CatalogEntry *e;
    if(tab==2) return sys_items[index%3];
    e=catalog_nth(&d->catalog,tab==1,index);
    return e?e->title:"";
}
static int count_for(const Dashboard *d,int tab)
{
    return tab==2?3:catalog_count_kind(&d->catalog,tab==1);
}
static void upper_short(char *out,size_t cap,const char *in,size_t max)
{
    size_t i=0;
    if(!in||cap==0)return;
    while(in[i]&&i<max&&i+1<cap) {out[i]=in[i];i++;}
    if(in[i]&&i+3<cap&&i>=3){out[i-3]='.';out[i-2]='.';out[i-1]='.';}
    out[i]=0;
}
static void label(Gfx *g,float x,float y,float sz,u64 color,const char *str)
{
    font_text(g,x,y,sz,color,str);
}
static void outline(Gfx *g,float x,float y,float w,float h,u64 c)
{
    gfx_rect(g,x,y,w,2,c);gfx_rect(g,x,y+h-2,w,2,c);
    gfx_rect(g,x,y,2,h,c);gfx_rect(g,x+w-2,y,2,h,c);
}
static void glyph_card(Gfx *g,float x,float y,int id,u64 c)
{
    int k=id%5;
    if(k==0){ /* four rings: icon rendered as line/primitive geometry */
        for(int n=0;n<3;n++)gfx_circle(g,x+20+n*20,y+22,9,c);
    } else if(k==1){
        gfx_round(g,x,y,66,48,12,c);
        gfx_rect(g,x+15,y+13,36,22,COL(6,26,49,128));
    } else if(k==2){
        for(int n=0;n<4;n++)gfx_line(g,x+n*17,y+44,x+28+n*12,y+8,3,c);
    } else if(k==3){
        gfx_circle(g,x+20,y+23,17,c);
        gfx_circle(g,x+46,y+23,17,c);
        gfx_rect(g,x+17,y+15,34,16,c);
    } else {
        gfx_line(g,x+5,y+43,x+34,y+6,4,c);
        gfx_line(g,x+34,y+6,x+63,y+43,4,c);
        gfx_line(g,x+14,y+34,x+54,y+34,4,c);
    }
}
static void draw_bg(Gfx *g,int frame)
{
    gfx_gradient(g,0,0,1280,720,COL(3,12,31,128),COL(6,24,54,128));
    /* Slow, almost static geometric wave: entirely GS triangles/lines. */
    for(int i=0;i<20;i++) {
        float x=(float)i*80.0f-40.0f;
        float yy=545.0f+34.0f*sinf((float)i*0.48f+(float)frame*0.0025f);
        gfx_line(g,x,yy,x+280,yy-68,1.2f,COL(30,100,172,13));
    }
    gfx_circle(g,1090,234,160,COL(15,65,132,9));
    gfx_circle(g,1140,204,105,COL(22,86,161,10));
    gfx_gradient(g,0,580,1280,140,COL(4,15,36,0),COL(2,9,23,96));
}
static void draw_header(Gfx *g,const Dashboard *d)
{
    gfx_circle(g,83,72,24,COL(48,133,227,128));
    gfx_circle(g,83,72,17,COL(4,20,48,128));
    gfx_rect(g,69,67,28,10,COL(230,246,255,128));
    gfx_rect(g,78,58,10,28,COL(230,246,255,128));
    label(g,125,58,4.4f,WHITE,"PS2 5");
    label(g,125,100,2.2f,SOFT,"DASHBOARD");
    for(int t=0;t<3;t++) {
        int x=442+t*160;
        label(g,(float)x,69,3.2f,t==d->tab?WHITE:MUTED,tabs[t]);
        if(t==d->tab)gfx_round(g,(float)x,111,91,4,2,CYAN);
    }
#if PS25_HIRES
    label(g,1061,72,2.6f,MUTED,"720P TEST");
#else
    label(g,1046,72,2.6f,MUTED,"480P SAFE");
#endif
    gfx_line(g,70,133,1210,133,1,COL(138,185,244,31));
}
static void draw_hero(Gfx *g,const Dashboard *d)
{
    int tab=d->tab;
    int count=count_for(d,tab);
    int selection=d->selection[tab];
    char title[48];
    const char *name;
    if(!count)name=tab==0?"YOUR GAME LIBRARY IS EMPTY":"YOUR APP LIBRARY IS EMPTY";
    else name=label_for(d,tab,selection);
    upper_short(title,sizeof(title),name,25);
    label(g,76,167,2.7f,CYAN,tab==0?"YOUR COLLECTION":tab==1?"YOUR APPLICATIONS":"CONSOLE");
    label(g,76,204,5.8f,WHITE,title);
    if(tab==2) {
        label(g,78,277,2.6f,MUTED,"CONFIGURATION AND SYSTEM INFORMATION");
    } else if(!count) {
        label(g,78,278,2.6f,MUTED,"ADD ELF ENTRIES TO MC0:/PS25/APPS.CFG");
        label(g,78,305,2.2f,SOFT,"NO AUTOMATIC PSBBN OR ISO DISCOVERY YET");
    } else {
        label(g,78,281,2.6f,MUTED,tab==0?"SELECT A GAME TO LAUNCH":"SELECT AN APPLICATION TO LAUNCH");
        label(g,78,309,2.3f,SOFT,"SUPPORTED: MC0 / MC1 / HOST ELF FILES");
    }
    /* Right-side abstract hero mark, inspired by PS5's spacious layout. */
    gfx_circle(g,1069,240,81,COL(14,64,139,38));
    gfx_circle(g,1069,240,66,COL(43,111,214,28));
    gfx_round(g,1028,199,82,82,21,COL(28,98,199,92));
    gfx_circle(g,1069,241,24,COL(7,27,58,128));
    gfx_line(g,1047,240,1090,240,5,WHITE);
    gfx_line(g,1069,219,1069,263,5,WHITE);
}
static void draw_card(Gfx *g,const Dashboard *d,int tab,int nth,int cardpos)
{
    float x=74.0f+cardpos*229.0f,y=377.0f,w=208.0f,h=186.0f;
    int selected=nth==d->selection[tab];
    int palette=nth%6;
    const unsigned char *tone=shades[palette];
    u64 base=COL(12+tone[0]/5,28+tone[1]/6,50+tone[2]/6,128);
    char title[48],number[16];
    const char *name=label_for(d,tab,nth);
    gfx_round(g,x+3,y+6,w,h,16,COL(0,0,8,58));
    gfx_round(g,x,y,w,h,15,base);
    gfx_gradient(g,x+8,y+10,w-16,92,
        COL(tone[0],tone[1],tone[2],48),
        COL(tone[0]/3,tone[1]/3,tone[2]/3,8));
    if(selected){
        outline(g,x-3,y-3,w+6,h+6,WHITE);
        gfx_rect(g,x+5,y+h-7,w-10,4,CYAN);
    } else {
        outline(g,x,y,w,h,COL(107,144,188,48));
    }
    glyph_card(g,x+70,y+31,nth+(tab==2?2:0),COL(151,204,255,98));
    snprintf(number,sizeof(number),"%02d",(nth+1));
    label(g,x+17,y+120,2.2f,COL(190,211,240,115),number);
    upper_short(title,sizeof(title),name,15);
    label(g,x+17,y+149,2.9f,WHITE,title);
}
static void draw_cards(Gfx *g,const Dashboard *d)
{
    int n=count_for(d,d->tab);
    int focus=d->selection[d->tab];
    int start=focus>2?focus-2:0;
    if(start>n-5 && n>5)start=n-5;
    if(n==0){
        gfx_round(g,75,374,1130,189,20,COL(21,41,71,128));
        outline(g,75,374,1130,189,COL(65,111,174,70));
        label(g,113,414,4,WHITE,"START HERE");
        label(g,113,463,2.7f,MUTED,"CREATE THE FILE MC0:/PS25/APPS.CFG");
        label(g,113,497,2.3f,SOFT,"EXAMPLE: GAME TITLE | GAME | MC0:/BOOT/APP.ELF");
        return;
    }
    for(int p=0;p<5&&start+p<n;p++)
        draw_card(g,d,d->tab,start+p,p);
}
static void draw_footer(Gfx *g,const Dashboard *d)
{
    gfx_line(g,73,622,1207,622,1,COL(100,152,203,36));
    label(g,79,653,2.4f,COL(229,245,255,128),"X  OPEN / SELECT");
    label(g,356,653,2.4f,MUTED,"L1 R1  TABS");
    label(g,640,653,2.4f,MUTED,"TRIANGLE  SETTINGS");
    label(g,1000,653,2.4f,MUTED,"O  BACK");
    if(d->message_frames>0){
        gfx_round(g,318,582,644,43,12,COL(8,44,83,128));
        label(g,347,598,2.1f,WHITE,d->message);
    }
}
static void draw_overlay(Gfx *g,const Dashboard *d)
{
    const char *title, *l1, *l2, *l3;
    if(!d->overlay)return;
    gfx_rect(g,0,0,1280,720,COL(1,5,14,91));
    gfx_round(g,251,192,778,326,21,COL(21,44,76,128));
    outline(g,251,192,778,326,COL(110,179,240,100));
    if(d->overlay==1){
        title="ABOUT PS2 5";
        l1="PROCEDURAL PS2 HOMEBREW DASHBOARD";
        l2="NO SONY ASSETS OR BITMAP TEXTURES";
        l3="EXPERIMENTAL - KEEP PSBBN UNCHANGED";
    }else if(d->overlay==2){
        title="EXIT TO BROWSER?";
        l1="THIS ENDS THE DASHBOARD SESSION";
        l2="X CONFIRMS - O CANCELS";
        l3="YOUR SSD DATA IS NOT MODIFIED";
    }else{
        title="CONTROLS";
        l1="LEFT / RIGHT: SELECT   L1 / R1: TABS";
        l2="X: OPEN    TRIANGLE: SYSTEM";
        l3="O: BACK    SELECT: HELP";
    }
    label(g,294,241,4.8f,WHITE,title);
    label(g,294,323,2.6f,MUTED,l1);
    label(g,294,371,2.6f,MUTED,l2);
    label(g,294,420,2.6f,MUTED,l3);
    label(g,294,478,2.4f,CYAN,"O  CLOSE");
}
void ui_init(Dashboard *d)
{
    memset(d,0,sizeof(*d));
    catalog_clear(&d->catalog);
}
void ui_toast(Dashboard *d,const char *msg)
{
    snprintf(d->message,sizeof(d->message),"%s",msg?msg:"");
    d->message_frames=200;
}
void ui_render(Gfx *g,const Dashboard *d)
{
    gfx_begin(g);
    draw_bg(g,d->frame);
    draw_header(g,d);
    draw_hero(g,d);
    draw_cards(g,d);
    draw_footer(g,d);
    draw_overlay(g,d);
    gfx_end(g);
}
void ui_action(Dashboard *d,unsigned int pressed,unsigned int held,
               int *exit_to_browser,const char **launch)
{
    int n;
    (void)held;
    *exit_to_browser=0;
    *launch=0;
    if(d->overlay){
        if(pressed & PAD_CIRCLE){d->overlay=0;return;}
        if(d->overlay==2&&(pressed&(PAD_CROSS|PAD_START))){
            *exit_to_browser=1;
        }
        return;
    }
    if(pressed & PAD_SELECT){d->overlay=3;return;}
    if(pressed & PAD_TRIANGLE){d->tab=2;return;}
    if(pressed & PAD_CIRCLE){d->tab=0;return;}
    if(pressed & PAD_L1)d->tab=(d->tab+2)%3;
    if(pressed & PAD_R1)d->tab=(d->tab+1)%3;
    n=count_for(d,d->tab);
    if(n>0){
        if(pressed & PAD_LEFT)
            d->selection[d->tab]=(d->selection[d->tab]+n-1)%n;
        if(pressed & PAD_RIGHT)
            d->selection[d->tab]=(d->selection[d->tab]+1)%n;
    }
    if(pressed & (PAD_CROSS|PAD_START)){
        if(d->tab==2){
            if(d->selection[2]==0)d->overlay=1;
            else if(d->selection[2]==1)d->overlay=3;
            else d->overlay=2;
        }else if(n>0){
            const CatalogEntry *entry=catalog_nth(&d->catalog,d->tab==1,
                                                  d->selection[d->tab]);
            if(entry)*launch=entry->path;
        }else ui_toast(d,"NO ENTRIES - SEE README FOR CONFIGURATION");
    }
}

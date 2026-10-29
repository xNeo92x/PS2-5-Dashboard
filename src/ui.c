/* PS2-5 Dashboard UI v0.2.
 * All surfaces are hardware primitives; labels use bounded procedural glyphs.
 * 1280x720 virtual coordinates, scaled to the current GS framebuffer.
 */
#include "ui.h"
#include "font.h"
#include "text_layout.h"
#include <libpad.h>
#include <stdio.h>
#include <string.h>

#define C(r,g,b,a) gfx_rgba(r,g,b,a)
#define WHITE C(237,243,255,128)
#define DIM C(144,165,197,128)
#define SUBTLE C(96,129,172,128)
#define ACCENT C(83,180,250,128)
#define NAVY C(3,11,28,128)

#if PS25_PROBE
static int probe_hide_text;
#endif

static const char *tabs[3]={"GAMES","APPS","SYSTEM"};
static const char *sysitems[3]={"ABOUT","CONTROLS","EXIT TO BROWSER"};
static const unsigned char tones[6][3]={
    {28,94,187},{16,124,181},{62,72,155},
    {27,119,144},{45,76,178},{39,94,157}
};

static int count_for(const Dashboard *d,int tab)
{
    return tab==2?3:catalog_count_kind(&d->catalog,tab==1);
}
static const char *title_for(const Dashboard *d,int tab,int pos)
{
    const CatalogEntry *e;
    if(tab==2) return (pos>=0 && pos<3)?sysitems[pos]:"";
    e=catalog_nth(&d->catalog,tab==1,pos);
    return e?e->title:"";
}
static void txt(Gfx *g,float x,float y,float sz,u64 c,const char *s,float max_width)
{
    char b[100];
#if PS25_PROBE
    if(probe_hide_text) return;
#endif
    ps25_fit_text(b,sizeof(b),s,sz,max_width);
    font_text(g,x,y,sz,c,b);
}
static void outline(Gfx *g,float x,float y,float w,float h,float z,u64 c)
{
    gfx_rect(g,x,y,w,z,c);
    gfx_rect(g,x,y+h-z,w,z,c);
    gfx_rect(g,x,y,z,h,c);
    gfx_rect(g,x+w-z,y,z,h,c);
}
static void symbol(Gfx *g,float x,float y,float size,int i,u64 c)
{
    /* Compact nontrademark geometric symbols; no textures or image files. */
    float a=size*0.23f;
    switch(i%5) {
        case 0:
            gfx_circle(g,x+a,y+a,a,c);
            gfx_circle(g,x+3*a,y+a,a,c);
            gfx_circle(g,x+2*a,y+3*a,a,c);
            break;
        case 1:
            gfx_round(g,x,y,size,size,12,c);
            gfx_rect(g,x+size*.20f,y+size*.29f,size*.60f,size*.42f,C(12,42,82,128));
            break;
        case 2:
            gfx_line(g,x,y+size,x+size*.5f,y,4,c);
            gfx_line(g,x+size*.5f,y,x+size,y+size,4,c);
            gfx_line(g,x+size*.25f,y+size*.7f,x+size*.75f,y+size*.7f,4,c);
            break;
        case 3:
            gfx_circle(g,x+size*.28f,y+size*.45f,size*.27f,c);
            gfx_circle(g,x+size*.72f,y+size*.45f,size*.27f,c);
            gfx_rect(g,x+size*.25f,y+size*.34f,size*.5f,size*.32f,c);
            break;
        default:
            gfx_round(g,x+size*.1f,y+size*.1f,size*.8f,size*.8f,9,c);
            gfx_line(g,x+size*.25f,y+size*.5f,x+size*.75f,y+size*.5f,4,C(10,36,74,128));
            gfx_line(g,x+size*.5f,y+size*.25f,x+size*.5f,y+size*.75f,4,C(10,36,74,128));
            break;
    }
}
static void background(Gfx *g)
{
    gfx_gradient(g,0,0,1280,720,C(3,13,36,128),C(4,11,30,128));
    gfx_gradient(g,0,0,1280,350,C(8,35,78,66),C(3,17,44,0));
    gfx_circle(g,1122,360,245,C(24,76,138,9));
    gfx_circle(g,1020,360,165,C(35,101,179,10));
    gfx_line(g,0,338,1280,338,1,C(101,146,209,15));
}
static void header(Gfx *g,const Dashboard *d)
{
    gfx_round(g,75,52,44,44,13,C(36,113,221,128));
    gfx_rect(g,94,61,7,27,WHITE);
    gfx_rect(g,84,71,27,7,WHITE);
    txt(g,134,59,4.8f,WHITE,"PS2 / 5",260);
    for(int i=0;i<3;i++) {
        float x=445.0f+i*160.0f;
        txt(g,x,69,3.0f,i==d->tab?WHITE:DIM,tabs[i],144);
        if(i==d->tab)gfx_round(g,x,105,112,4,2,ACCENT);
    }
#if PS25_HIRES
    txt(g,1061,73,2.4f,DIM,"HIRES TEST",153);
#elif PS25_DIRECT720
    txt(g,1033,73,2.4f,DIM,"720P DIRECT",177);
#else
    txt(g,1065,73,2.4f,DIM,"480P",115);
#endif
    gfx_rect(g,72,124,1136,1,C(123,151,196,25));
}
static void tile(Gfx *g,const Dashboard *d,int idx,int display_pos)
{
    float x=76.0f+(float)display_pos*200.0f;
    float y=162.0f,w=184.0f,h=171.0f;
    int selected=d->selection[d->tab]==idx;
    const unsigned char *rgb=tones[idx%6];
    u64 card=C(rgb[0],rgb[1],rgb[2],90);
    const char *name=title_for(d,d->tab,idx);
    gfx_round(g,x,y,w,h,14,C(14,34,66,128));
    gfx_gradient(g,x+6,y+6,w-12,h-54,card,C(rgb[0]/3,rgb[1]/3,rgb[2]/3,20));
    gfx_round(g,x+8,y+116,w-16,45,6,C(8,24,50,128));
    if(selected) {
        outline(g,x-3,y-3,w+6,h+6,3,WHITE);
        gfx_rect(g,x+9,y+h-5,w-18,3,ACCENT);
    } else {
        outline(g,x,y,w,h,1,C(84,123,184,48));
    }
    symbol(g,x+62,y+28,58,idx+(d->tab==2?1:0),C(157,209,255,120));
    txt(g,x+17,y+133,2.35f,WHITE,name,w-34);
}
static void tile_placeholder(Gfx *g)
{
    gfx_round(g,76,162,184,171,15,C(14,31,61,128));
    outline(g,73,159,190,177,2,C(96,159,240,105));
    symbol(g,137,192,60,4,C(95,153,209,110));
    txt(g,94,297,2.45f,WHITE,"EMPTY LIBRARY",148);
}
static void rail(Gfx *g,const Dashboard *d)
{
    int count=count_for(d,d->tab);
    int selection=d->selection[d->tab];
    int start=selection>3?selection-3:0;
    if(start>count-5 && count>5) start=count-5;
    txt(g,77,138,2.0f,SUBTLE,d->tab==2?"CONSOLE OPTIONS":"YOUR COLLECTION",298);
    if(count==0){tile_placeholder(g);return;}
    for(int i=0;i<5 && start+i<count;i++)tile(g,d,start+i,i);
    if(start>0)txt(g,77,350,2.3f,SUBTLE,"< PREVIOUS",188);
    if(start+5<count)txt(g,1010,350,2.3f,SUBTLE,"NEXT >",190);
}
static void hero(Gfx *g,const Dashboard *d)
{
    int tab=d->tab, count=count_for(d,tab);
    int selected=d->selection[tab];
    const char *headline,*detail,*hint;
    gfx_round(g,75,372,1131,228,17,C(11,29,59,124));
    gfx_gradient(g,84,382,1109,205,C(17,53,106,70),C(8,23,52,32));
    gfx_rect(g,100,406,4,35,ACCENT);

    if(tab==2) {
        headline=sysitems[selected];
        if(selected==0) {
            detail="ORIGINAL PS2 HOMEBREW DASHBOARD";
            hint="BUILT WITH GS PRIMITIVES - NO IMAGE ASSETS";
        } else if(selected==1) {
            detail="LEFT / RIGHT: SELECT     L1 / R1: SWITCH TABS";
            hint="X: OPEN     CIRCLE: BACK     SQUARE: LOAD LIST";
        } else {
            detail="RETURN TO THE PLAYSTATION 2 BROWSER";
            hint="PRESS X TO CONFIRM";
        }
    } else if(count==0) {
        headline=tab==0?"NO GAMES ADDED":"NO APPS ADDED";
        detail="ADD HOME BREW ELF ENTRIES TO THE CATALOG";
        hint="MC0:/PS25/APPS.CFG     PRESS SQUARE TO LOAD";
    } else {
        headline=title_for(d,tab,selected);
        detail=tab==0?"READY TO LAUNCH":"APPLICATION";
        hint="PRESS X TO START  /  L1 R1 TO CHANGE CATEGORY";
    }
    txt(g,122,407,2.35f,ACCENT,tab==0?"GAMES":tab==1?"APPLICATIONS":"SYSTEM",390);
    txt(g,122,450,5.0f,WHITE,headline,735);
    txt(g,122,522,2.35f,DIM,detail,790);
    txt(g,122,561,2.1f,SUBTLE,hint,790);

    /* Abstract artwork: a shape rather than copied PS5 iconography. */
    gfx_circle(g,1040,474,92,C(38,112,208,30));
    gfx_circle(g,1040,474,66,C(48,139,230,36));
    symbol(g,1009,443,67,selected+(tab==2?1:0),C(194,233,255,125));
}
static void footer(Gfx *g,const Dashboard *d)
{
    gfx_rect(g,75,629,1130,1,C(140,169,218,34));
    txt(g,79,661,2.2f,WHITE,"X OPEN",138);
    txt(g,278,661,2.2f,DIM,"L1 R1 TABS",185);
    txt(g,532,661,2.2f,DIM,"SQUARE LOAD LIST",270);
    txt(g,915,661,2.2f,DIM,"O BACK",135);
    if(d->message_frames>0) {
        gfx_round(g,292,606,700,34,9,C(16,63,111,128));
        txt(g,316,615,1.9f,WHITE,d->message,650);
    }
}
static void overlay(Gfx *g,const Dashboard *d)
{
    const char *headline,*l1,*l2,*l3;
    if(!d->overlay)return;
    gfx_rect(g,0,0,1280,720,C(2,6,18,105));
    gfx_round(g,243,202,795,315,18,C(19,42,79,128));
    outline(g,243,202,795,315,2,C(115,173,245,95));
    if(d->overlay==1) {
        headline="ABOUT PS2 / 5";
        l1="ORIGINAL PROCEDURAL HOME BREW DASHBOARD";
        l2="NO SONY ASSETS - NO LARGE BITMAP TEXTURES";
        l3="EXPERIMENTAL - KEEP PSBBN UNCHANGED";
    } else if(d->overlay==2) {
        headline="EXIT TO BROWSER?";
        l1="THIS ENDS THE DASHBOARD SESSION";
        l2="X CONFIRM   O CANCEL";
        l3="YOUR SATA SSD IS NOT MODIFIED";
    } else {
        headline="CONTROLS";
        l1="LEFT RIGHT SELECT   L1 R1 SWITCH CATEGORY";
        l2="X OPEN   TRIANGLE SYSTEM   O BACK";
        l3="SQUARE LOAD LIST   SELECT HELP";
    }
    txt(g,285,239,4.6f,WHITE,headline,720);
    txt(g,286,322,2.5f,DIM,l1,709);
    txt(g,286,366,2.5f,DIM,l2,709);
    txt(g,286,411,2.5f,DIM,l3,709);
    txt(g,286,475,2.2f,ACCENT,"O CLOSE",270);
}
void ui_init(Dashboard *d)
{
    memset(d,0,sizeof(*d));
    catalog_clear(&d->catalog);
}
void ui_toast(Dashboard *d,const char *message)
{
    snprintf(d->message,sizeof(d->message),"%s",message?message:"");
    d->message_frames=200;
}
void ui_render(Gfx *g,const Dashboard *d)
{
#if PS25_PROBE
    /* Self-advancing hardware isolation test. One mode per 420 frames.
       0 = geometry-free six-color scanout; 1 = all dashboard primitives
       without text; 2 = complete dashboard with procedural glyphs.
       No pad, IOP, MC or SSD access when built with PS25_DEMO=1.
       Visual marker in the center top: white/orange/cyan. */
    int stage=(d->frame/420)%3;
    if(stage==0) {
        static const int color[6][3]={
            {17,52,118},{33,112,213},{34,157,146},
            {233,143,27},{112,50,189},{32,189,67}
        };
        gfx_begin(g);
        for(int i=0;i<6;i++)
            gfx_rect(g,0,i*120,1280,120,C(color[i][0],color[i][1],color[i][2],128));
        gfx_rect(g,15,15,10,690,WHITE);
        gfx_rect(g,1255,15,10,690,WHITE);
        gfx_rect(g,610,24,60,60,WHITE);
        gfx_end(g);
        return;
    }
    probe_hide_text=(stage==1);
#endif
    gfx_begin(g);
    background(g);
    header(g,d);
    rail(g,d);
    hero(g,d);
    footer(g,d);
    overlay(g,d);
#if PS25_PROBE
    /* Diagnostic mode marker: orange square = shapes without text.
       Cyan square = full scene with font draw calls. */
    gfx_rect(g,610,30,60,60,stage==1?C(255,145,12,128):ACCENT);
#endif
    gfx_end(g);
}
void ui_action(Dashboard *d,unsigned int pressed,unsigned int held,
               int *exit_to_browser,const char **launch)
{
    int count;
    (void)held;
    *exit_to_browser=0;
    *launch=NULL;
    if(d->overlay) {
        if(pressed & PAD_CIRCLE){d->overlay=0;return;}
        if(d->overlay==2 && (pressed & (PAD_CROSS|PAD_START)))
            *exit_to_browser=1;
        return;
    }
    if(pressed & PAD_SELECT){d->overlay=3;return;}
    if(pressed & PAD_TRIANGLE){d->tab=2;return;}
    if(pressed & PAD_CIRCLE){d->tab=0;return;}
    if(pressed & PAD_L1)d->tab=(d->tab+2)%3;
    if(pressed & PAD_R1)d->tab=(d->tab+1)%3;
    count=count_for(d,d->tab);
    if(count>0) {
        if(pressed & PAD_LEFT)d->selection[d->tab]=(d->selection[d->tab]+count-1)%count;
        if(pressed & PAD_RIGHT)d->selection[d->tab]=(d->selection[d->tab]+1)%count;
    }
    if(pressed & (PAD_CROSS|PAD_START)) {
        if(d->tab==2) {
            if(d->selection[2]==0)d->overlay=1;
            else if(d->selection[2]==1)d->overlay=3;
            else d->overlay=2;
        } else if(count>0) {
            const CatalogEntry *e=catalog_nth(&d->catalog,d->tab==1,d->selection[d->tab]);
            if(e)*launch=e->path;
        } else ui_toast(d,"NO ENTRIES IN CATALOG");
    }
}

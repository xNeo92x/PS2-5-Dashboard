#include "catalog.h"
#include <string.h>
#include <ctype.h>

static char *trim(char *p)
{
    char *e;
    while(*p==' '||*p=='\t'||*p=='\r')++p;
    e=p+strlen(p);
    while(e>p&&(e[-1]==' '||e[-1]=='\t'||e[-1]=='\r'))*--e=0;
    return p;
}
static int has_elf_extension(const char *p)
{
    size_t n=strlen(p);
    return n>=4 && p[n-4]=='.' &&
           toupper((unsigned char)p[n-3])=='E' &&
           toupper((unsigned char)p[n-2])=='L' &&
           toupper((unsigned char)p[n-1])=='F';
}
static int allowed_path(const char *p)
{
    /* Explicit allowlist. ISO, PFS and USB launches need separate loaders. */
    return (!strncmp(p,"mc0:/",5) || !strncmp(p,"mc1:/",5) ||
            !strncmp(p,"host:",5)) && has_elf_extension(p);
}
void catalog_clear(Catalog *c) { memset(c,0,sizeof(*c)); }

int catalog_parse_line(Catalog *c,const char *line)
{
    char buf[260],*title,*kind,*path,*a,*b;
    size_t n;
    int is_app;
    CatalogEntry *entry;
    if(!c||!line||c->count>=PS25_MAX_ITEMS)return 0;
    n=strlen(line);
    if(n==0||n>=sizeof(buf))return 0;
    memcpy(buf,line,n+1);
    title=trim(buf);
    if(!*title||*title=='#'||*title==';')return 0;
    a=strchr(title,'|');
    if(!a)return 0;
    *a++=0; kind=trim(a);
    b=strchr(kind,'|');
    if(!b)return 0;
    *b++=0; path=trim(b);
    title=trim(title);kind=trim(kind);
    if(strchr(path,'|')||!title[0]||strlen(title)>=PS25_TITLE_LEN||
       strlen(path)>=PS25_PATH_LEN||!allowed_path(path))return 0;
    for(n=0;kind[n];n++)kind[n]=(char)toupper((unsigned char)kind[n]);
    if(!strcmp(kind,"GAME"))is_app=0;
    else if(!strcmp(kind,"APP"))is_app=1;
    else return 0;
    entry=&c->entries[c->count++];
    strcpy(entry->title,title);
    strcpy(entry->path,path);
    entry->is_app=is_app;
    return 1;
}
int catalog_from_text(Catalog *c,const char *text)
{
    char line[260];
    int used=0, accepted=0, overflow=0;
    if(!c||!text)return 0;
    while(*text) {
        char ch=*text++;
        if(ch=='\r')continue;
        if(ch=='\n') {
            if(!overflow) {line[used]=0;accepted+=catalog_parse_line(c,line);}
            used=0;overflow=0;
        } else if(!overflow) {
            if(used+1<(int)sizeof(line))line[used++]=ch;
            else overflow=1;
        }
    }
    if(used>0&&!overflow){line[used]=0;accepted+=catalog_parse_line(c,line);}
    return accepted;
}
int catalog_count_kind(const Catalog *c,int is_app)
{
    int i,n=0;
    for(i=0;i<c->count;i++)if(c->entries[i].is_app==is_app)n++;
    return n;
}
const CatalogEntry *catalog_nth(const Catalog *c,int is_app,int n)
{
    int i;
    if(n<0)return 0;
    for(i=0;i<c->count;i++)
        if(c->entries[i].is_app==is_app) {
            if(n--==0)return &c->entries[i];
        }
    return 0;
}

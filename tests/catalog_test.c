#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "catalog.h"

int main(void)
{
    Catalog c;
    const CatalogEntry *e;
    catalog_clear(&c);
    assert(catalog_from_text(&c,
        "# demo config\n"
        "Gran Turismo 4 | GAME | mc0:/BOOT/GT4.ELF\r\n"
        "LaunchELF| APP | mc1:/APPS/ULE.elf\n"
        "not allowed | GAME | mass:/USB/START.ELF\n"
        "not an elf|GAME|mc0:/FILE.ISO\n"
        "malformed line\n")==2);
    assert(c.count==2);
    assert(catalog_count_kind(&c,0)==1);
    assert(catalog_count_kind(&c,1)==1);
    e=catalog_nth(&c,0,0);
    assert(e && strcmp(e->title,"Gran Turismo 4")==0);
    assert(strcmp(e->path,"mc0:/BOOT/GT4.ELF")==0);
    assert(catalog_nth(&c,1,0)!=NULL);
    assert(catalog_nth(&c,1,1)==NULL);
    assert(!catalog_parse_line(&c,"too many|pipes|mc0:/A.ELF|BAD"));
    assert(!catalog_parse_line(&c,"invalid|MUSIC|mc0:/A.ELF"));
    assert(!catalog_parse_line(&c,"unsafe|GAME|hdd0:/A.ELF"));
    assert(catalog_from_text(&c,"Good|APP|host:run.ELF")==1);
    assert(c.count==3);
    printf("catalog tests passed (3 accepted entries)\n");
    return 0;
}

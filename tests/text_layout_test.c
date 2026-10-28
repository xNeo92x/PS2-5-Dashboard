#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "text_layout.h"

int main(void)
{
    char b[32];
    ps25_fit_text(b,sizeof(b),"EXIT TO BROWSER",2.9f,145.f);
    assert(strlen(b)*6.f*2.9f <= 145.f);
    assert(strcmp(b,"EXIT...")==0);
    ps25_fit_text(b,sizeof(b),"ABOUT",3.0f,145.f);
    assert(!strcmp(b,"ABOUT"));
    ps25_fit_text(b,sizeof(b),"ONE TWO THREE",2.f,48.f);
    assert(!strcmp(b,"O..."));
    ps25_fit_text(b,sizeof(b),"abc",0.f,100.f);
    assert(!*b);
    ps25_fit_text(b,4,"TEST LONG NAME",1.f,1000.f);
    assert(!strcmp(b,"TES"));
    ps25_fit_text(b,sizeof(b),NULL,3.f,100.f);
    assert(!*b);
    printf("text layout tests passed\n");
    return 0;
}

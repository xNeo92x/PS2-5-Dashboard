#ifndef PS25_UI_H
#define PS25_UI_H
#include "gfx.h"
#include "catalog.h"

typedef struct {
    Catalog catalog;
    int tab; /* 0 games, 1 apps, 2 system */
    int selection[3];
    int frame;
    int overlay; /* 0 none, 1 about, 2 exit confirmation, 3 help */
    int config_found;
    char message[96];
    int message_frames;
} Dashboard;

void ui_init(Dashboard *d);
void ui_render(Gfx *g, const Dashboard *d);
void ui_action(Dashboard *d, unsigned int pressed, unsigned int held, int *exit_to_browser, const char **launch);
void ui_toast(Dashboard *d, const char *msg);
#endif

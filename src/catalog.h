#ifndef PS25_CATALOG_H
#define PS25_CATALOG_H

#define PS25_MAX_ITEMS 32
#define PS25_TITLE_LEN 48
#define PS25_PATH_LEN 128

typedef struct {
    char title[PS25_TITLE_LEN];
    char path[PS25_PATH_LEN];
    int is_app; /* 0 = game launcher entry, 1 = app launcher entry */
} CatalogEntry;
typedef struct {
    CatalogEntry entries[PS25_MAX_ITEMS];
    int count;
} Catalog;

/* No SDK dependencies: parsing is host-unit-tested. */
void catalog_clear(Catalog *c);
int catalog_parse_line(Catalog *c, const char *line);
int catalog_from_text(Catalog *c, const char *text);
int catalog_count_kind(const Catalog *c, int is_app);
const CatalogEntry *catalog_nth(const Catalog *c, int is_app, int n);

#endif

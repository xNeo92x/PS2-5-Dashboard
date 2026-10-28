#ifndef PS25_TEXT_LAYOUT_H
#define PS25_TEXT_LAYOUT_H
#include <stddef.h>

/* Fixed-width procedural font: each glyph occupies 6 * scale units.
   Fits ASCII source into a bounded UI region, adding '...' if needed.
   Used to prevent labels from crossing card boundaries. */
void ps25_fit_text(char *dst, size_t capacity, const char *src,
                   float scale, float max_width);
#endif

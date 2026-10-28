#include "text_layout.h"
#include <string.h>

void ps25_fit_text(char *dst, size_t capacity, const char *src,
                   float scale, float max_width)
{
    size_t limit, len, copy;
    if (!dst || !capacity) return;
    dst[0] = 0;
    if (!src || scale <= 0 || max_width <= 0) return;
    /* Saturate to dst size before floating-point to integer conversion. */
    if (max_width >= (float)(capacity - 1) * 6.0f * scale)
        limit = capacity - 1;
    else
        limit = (size_t)(max_width / (6.0f * scale));
    if (limit > capacity - 1) limit = capacity - 1;
    if (limit == 0) return;
    len = strlen(src);
    if (len <= limit) {
        memcpy(dst, src, len + 1);
        return;
    }
    if (limit <= 3) {
        memset(dst, '.', limit);
        dst[limit] = 0;
        return;
    }
    copy = limit - 3;
    while (copy > 0 && src[copy-1] == ' ') copy--;
    memcpy(dst, src, copy);
    memcpy(dst + copy, "...", 3);
    dst[copy+3]=0;
}

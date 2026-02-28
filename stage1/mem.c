#include "mem.h"

void *memcpy(void *dst, const void *src, size_t num) {
    /* TODO: faster memcpy */
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (num--) {
        *d++ = *s++;
    }
    return dst;
}

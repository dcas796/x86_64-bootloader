#include "mem.h"

#include "math.h"
#include "memory_map.h"
#include "print.h"

static void *ext_stack_top = (void*)EXTENDED_STACK_BASE;

void *memcpy(void *dst, const void *src, size_t num) {
    /* TODO: faster memcpy */
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (num--) {
        *d++ = *s++;
    }
    return dst;
}

void *push(size_t n) {
    if ((uint32_t)ext_stack_top - EXTENDED_STACK_LIMIT < n) {
        panic("Overgrown stack");
    }
    ext_stack_top -= n;
    return ext_stack_top;
}

void pop(size_t n) {
    if (EXTENDED_STACK_BASE - (uint32_t)ext_stack_top < n) {
        panic("Stack overflow");
    }
    ext_stack_top += n;
}

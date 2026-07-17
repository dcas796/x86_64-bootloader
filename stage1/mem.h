#ifndef MEM_H
#define MEM_H

#include <types.h>

void *memset(void *dst, uint8_t b, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *push(size_t n);
void pop(size_t n);
void *static_alloc(size_t n);

#endif

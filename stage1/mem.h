#ifndef MEM_H
#define MEM_H

#include <types.h>

void *memcpy(void *dst, const void *src, size_t n);
void *push(size_t n);
void *leak(size_t n);
void pop(size_t n);

#endif

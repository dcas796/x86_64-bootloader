#ifndef SYSINFO_H
#define SYSINFO_H

#include <types.h>

typedef struct __attribute__((aligned(4))) {
    uint8_t boot_drive;
} sysinfo_t;

#endif
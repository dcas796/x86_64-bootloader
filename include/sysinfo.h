#ifndef SYSINFO_H
#define SYSINFO_H

#include <types.h>

typedef enum {
    SYSINFO_MT_USABLE,
    SYSINFO_MT_RESERVED,
    SYSINFO_MT_RECLAIMABLE,
    SYSINFO_MT_ACPI_NVS,
    SYSINFO_MT_BAD,
} sysinfo_mem_type_t;

typedef struct {
    uint64_t base_addr;
    uint64_t size;
    sysinfo_mem_type_t type;
    bool is_volatile;
} sysinfo_memregion_t;

typedef struct __attribute__((aligned(4))) {
    uint8_t boot_drive;
    sysinfo_memregion_t *mem_regions;
    size_t mem_regions_count;
} sysinfo_t;

#endif
#ifndef SYSINFO_IMPL_H
#define SYSINFO_IMPL_H

#include <types.h>
#include <sysinfo.h>

typedef enum {
    SYSINFO_SUCCESS,
    SYSINFO_EMPTY_MEMORY_LAYOUT,
} sysinfo_result_t;

sysinfo_result_t get_sysinfo(sysinfo_t *info, uint8_t boot_drive);
const char *sysinfo_result_to_str(sysinfo_result_t result);

#endif

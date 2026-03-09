#include "sysinfo_impl.h"

void get_sysinfo(sysinfo_t *info, uint8_t boot_drive) {
    *info = (sysinfo_t){
        .boot_drive = boot_drive,
    };
}

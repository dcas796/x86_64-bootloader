#include "sysinfo_impl.h"

#include "mem.h"

#define MREGION_MAGIC 0x534D4150

typedef struct PACKED {
    uint64_t base_addr;
    uint64_t size;
    uint32_t type;
    uint32_t ext_attr;
} mregion_t;

static bool get_next_mregion(mregion_t *region, uint32_t *index) {
    bool has_error;
    uint32_t function = 0xE820;

    __asm__ volatile(
        "clc\n\t"
        "int $0x15\n\t"
        : "=@ccc"(has_error), "+a"(function), "+b"(*index)
        : "d"(MREGION_MAGIC), "c"(24), "D"(region)
        : "memory"
    );

    return !has_error && *index != 0 && function == MREGION_MAGIC;
}

static void convert_region(mregion_t region, sysinfo_memregion_t *sysinfo_region) {
    sysinfo_mem_type_t type;

    switch (region.type) {
        case 1:
            type = SYSINFO_MT_USABLE;
            break;
        case 2:
            type = SYSINFO_MT_RESERVED;
            break;
        case 3:
            type = SYSINFO_MT_RECLAIMABLE;
            break;
        case 4:
            type = SYSINFO_MT_ACPI_NVS;
            break;
        case 5:
            type = SYSINFO_MT_BAD;
            break;
        default:
            type = SYSINFO_MT_RESERVED;
    }

    *sysinfo_region = (sysinfo_memregion_t){
        .base_addr = region.base_addr,
        .size = region.size,
        .type = type,
        .is_volatile = (region.ext_attr >> 1) & 1,
    };
}

sysinfo_result_t get_sysinfo(sysinfo_t *info, uint8_t boot_drive) {
    sysinfo_memregion_t *mem_regions = nullptr;

    uint32_t count = 0;
    uint32_t index = 0;
    auto mregion = (mregion_t){ 0 };

    while (get_next_mregion(&mregion, &index)) {
        mem_regions = push(sizeof(sysinfo_memregion_t));
        convert_region(mregion, mem_regions);
        mregion = (mregion_t){ 0 };
        ++count;
    }

    if (mem_regions == nullptr) return SYSINFO_EMPTY_MEMORY_LAYOUT;

    *info = (sysinfo_t){
        .boot_drive = boot_drive,
        .mem_regions = mem_regions,
        .mem_regions_count = count,
    };

    return SYSINFO_SUCCESS;
}

const char *sysinfo_result_to_str(sysinfo_result_t result) {
    switch (result) {
        case SYSINFO_SUCCESS:
            return "Success";
        case SYSINFO_EMPTY_MEMORY_LAYOUT:
            return "Memory layout reported by BIOS is empty";
        default:
            return "Unknown";
    }
}

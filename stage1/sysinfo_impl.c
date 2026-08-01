#include "sysinfo_impl.h"

#include "mem.h"
#include "memory_map.h"

#define MREGION_MAGIC               0x534D4150
#define RSDP_SIGNATURE              "RSD PTR "
#define EBDA_SEGMENT_FIELD          (uint16_t*)0x40E
#define EBDA_RSDP_SEARCH_SIZE       1024
#define BIOS_READ_ONLY_REGION_START (uint8_t*)0xE0000
#define BIOS_READ_ONLY_REGION_END   (uint8_t*)0xFFFFF

typedef struct PACKED {
    uint64_t base_addr;
    uint64_t size;
    uint32_t type;
    uint32_t ext_attr;
} mregion_t;

static bool get_next_mregion(const mregion_t *region, uint32_t *index) {
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

static bool is_left_more_restrictive(sysinfo_mem_type_t left, sysinfo_mem_type_t right) {
    return left > right;
}

static void add_padding_right(sysinfo_memregion_t *this_region) {
    if (this_region->next != nullptr) {
        uint64_t limit_right = this_region->base_addr + this_region->size;
        if (this_region->type == this_region->next->type) {
            this_region->size = this_region->next->base_addr + this_region->next->size - this_region->base_addr;
            this_region->next = this_region->next->next;
        } else if (this_region->next->type == SYSINFO_MT_RESERVED) {
            this_region->next->size += this_region->next->base_addr - limit_right;
            this_region->next->base_addr = limit_right;
        } else if (limit_right < this_region->next->base_addr) {
            sysinfo_memregion_t *padding = static_alloc(sizeof(sysinfo_memregion_t));
            *padding = (sysinfo_memregion_t){
                .next = this_region->next,
                .base_addr = limit_right,
                .size = this_region->next->base_addr - limit_right,
                .type = SYSINFO_MT_RESERVED,
                .is_volatile = false
            };
            this_region->next = padding;
        }
    }
}

static void fix_right(sysinfo_memregion_t *this_region) {
    if (this_region == nullptr) return;

    uint64_t limit_right = this_region->base_addr + this_region->size;

    while (this_region->next != nullptr && limit_right > this_region->next->base_addr) {
        if (!is_left_more_restrictive(this_region->next->type, this_region->type)) {
            uint64_t next_region_limit_right = this_region->next->base_addr + this_region->next->size;
            if (next_region_limit_right <= limit_right) {
                this_region->next = this_region->next->next;
            } else {
                this_region->next->size -= limit_right - this_region->next->base_addr;
                this_region->next->base_addr = limit_right;
            }
        } else {
            this_region->size = this_region->next->base_addr - this_region->base_addr;

            if (limit_right > this_region->next->base_addr + this_region->next->size) {
                sysinfo_memregion_t *split_region = static_alloc(sizeof(sysinfo_memregion_t));
                memcpy(split_region, this_region, sizeof(sysinfo_memregion_t));
                split_region->base_addr = this_region->next->base_addr + this_region->next->size;
                split_region->size = limit_right - split_region->base_addr;
                split_region->next = this_region->next->next;
                this_region->next->next = split_region;
                this_region = split_region;
            }
        }
    }

    add_padding_right(this_region);
}

void insert_sorted(sysinfo_memregion_t **mem_regions, sysinfo_memregion_t *this_region) {
    if (this_region == nullptr || this_region->size == 0) return;

    sysinfo_memregion_t *prev2 = nullptr;
    sysinfo_memregion_t *prev = nullptr;
    sysinfo_memregion_t *current = *mem_regions;
    while (current != nullptr && current->base_addr < this_region->base_addr) {
        prev2 = prev;
        prev = current;
        current = current->next;
    }

    if (prev == nullptr) {
        if (*mem_regions == nullptr) {
            *mem_regions = this_region;
            (*mem_regions)->next = nullptr;
        } else {
            this_region->next = *mem_regions;
            *mem_regions = this_region;
            fix_right(*mem_regions);
        }
    } else {
        this_region->next = current;
        prev->next = this_region;
        fix_right(prev2);
        fix_right(prev);
    }
}

static sysinfo_result_t get_mem_regions(sysinfo_memregion_t **mem_regions) {
    uint32_t count = 0;
    uint32_t index = 0;
    auto mregion = (mregion_t){ 0 };

    while (get_next_mregion(&mregion, &index)) {
        sysinfo_memregion_t *this_region = static_alloc(sizeof(sysinfo_memregion_t));
        convert_region(mregion, this_region);
        insert_sorted(mem_regions, this_region);
        ++count;
    }

    if (count == 0) return SYSINFO_EMPTY_MEMORY_LAYOUT;

    sysinfo_memregion_t *reserved = static_alloc(sizeof(sysinfo_memregion_t));
    *reserved = (sysinfo_memregion_t){
        .next = nullptr,
        .base_addr = 0x0,
        .size = REAL_MODE_LIMIT,
        .type = SYSINFO_MT_RESERVED,
        .is_volatile = false,
    };
    insert_sorted(mem_regions, reserved);

    return SYSINFO_SUCCESS;
}

static bool is_rsdp_valid(const uint8_t *p) {
    typedef struct __attribute__ ((packed)) {
        char signature[8];
        uint8_t checksum;
        char oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;
    } rsdp_t;

    typedef struct __attribute__ ((packed)) {
        char signature[8];
        uint8_t checksum;
        char oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
        uint8_t reserved[3];
    } xsdp_t;

    auto rsdp = (const rsdp_t *)p;

    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(rsdp_t); i++) {
        sum += p[i];
    }
    if (sum != 0) return false;

    if (rsdp->revision > 0) {
        auto xsdp = (const xsdp_t *)p;

        for (size_t i = sizeof(rsdp_t); i < xsdp->length; i++) {
            sum += p[i];
        }
        if (sum != 0) return false;
    }

    return true;
}

static sysinfo_result_t get_rsdp(uint8_t **rdsp) {
    *rdsp = nullptr;

    const uint16_t ebda_segment = *EBDA_SEGMENT_FIELD;
    auto const ebda_base = (uint8_t*)(ebda_segment * 16);

    for (uint8_t *p = ebda_base; p < ebda_base + EBDA_RSDP_SEARCH_SIZE; p += 16) {
        if (memeq(RSDP_SIGNATURE, (const char *)p, sizeof(RSDP_SIGNATURE) - 1) && is_rsdp_valid(p)) {
            *rdsp = p;
            return SYSINFO_SUCCESS;
        }
    }

    for (uint8_t *p = BIOS_READ_ONLY_REGION_START; p < BIOS_READ_ONLY_REGION_END; p += 16) {
        if (memeq(RSDP_SIGNATURE, (const char *)p, sizeof(RSDP_SIGNATURE) - 1) && is_rsdp_valid(p)) {
            *rdsp = p;
            return SYSINFO_SUCCESS;
        }
    }

    return SYSINFO_RSDP_NOT_FOUND;
}

sysinfo_result_t get_sysinfo(sysinfo_t *info, uint8_t boot_drive) {
    sysinfo_memregion_t *mem_regions = nullptr;
    sysinfo_result_t result = get_mem_regions(&mem_regions);
    if (result != SYSINFO_SUCCESS) return result;

    uint8_t *rsdp = nullptr;
    result = get_rsdp(&rsdp);
    if (result != SYSINFO_SUCCESS) return result;

    *info = (sysinfo_t){
        .boot_drive = boot_drive,
        .mem_regions = mem_regions,
        .rsdp = rsdp,
    };

    return SYSINFO_SUCCESS;
}

const char *sysinfo_result_to_str(sysinfo_result_t result) {
    switch (result) {
        case SYSINFO_SUCCESS:
            return "Success";
        case SYSINFO_EMPTY_MEMORY_LAYOUT:
            return "Memory layout reported by BIOS is empty";
        case SYSINFO_RSDP_NOT_FOUND:
            return "Could not find a valid RSDP";
        default:
            return "Unknown";
    }
}

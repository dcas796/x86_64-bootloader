#include "disk.h"

#include "math.h"
#include "memory_map.h"

typedef struct PACKED {
    uint8_t  size;      /* must always be 0x10 */
    uint8_t  zero;      /* must always be 0 */
    uint16_t num_sect;
    uint16_t buf_offset;
    uint16_t buf_segment;
    uint64_t lba;
} dap_t;

disk_status_t disk_read(uint8_t drive_number, uint64_t lba, uint8_t *buffer, uint16_t sector_count) {
    uint32_t addr = (uint32_t)buffer;
    if (addr > REAL_MODE_LIMIT) return DISK_ADDR_OVER_REAL_BOUNDARY;
    uint16_t segment = (uint16_t)min(addr >> 4, 0xffff);
    uint16_t offset = (uint16_t)(addr - segment * 16);
    dap_t dap = {
        .size = 0x10,
        .zero = 0,
        .num_sect = sector_count,
        .buf_offset = offset,
        .buf_segment = segment,
        .lba = lba
    };

    bool has_error;
    uint16_t return_code;

    __asm__ volatile (
        "clc\n\t"
        "int $0x13\n"
        : "=@ccc" (has_error), "=a" (return_code)
        : "a" (0x42 << 8), "d" (drive_number), "S" ((uint16_t)((uint32_t)&dap & 0xffff)) /* this is to shut up the compiler */
        : "memory"
    );

    return has_error ? (disk_status_t)(return_code >> 8) : DISK_SUCCESS;
}

const char *disk_status_to_str(disk_status_t status) {
    switch (status) {
        case DISK_SUCCESS:
            return "Success";
        case DISK_INVALID_COMMAND:
            return "Invalid command";
        case DISK_ADDR_MARK_NOT_FOUND:
            return "Address mark not found";
        case DISK_WRITE_ON_PROT_DISK:
            return "Write on protected disk";
        case DISK_SECTOR_NOT_FOUND:
            return "Sector not found";
        case DISK_RESET_FAILED:
            return "Reset failed";
        case DISK_CHANGE_LINE_ACTIVE:
            return "Change line active";
        case DISK_PARAM_ACTIVITY_FAILED:
            return "Parameter activity failed";
        case DISK_DMA_OVERRUN:
            return "DMA overrun";
        case DISK_DMA_OVER_BOUNDARY:
            return "DMA over 64K boundary";
        case DISK_BAD_SECTOR:
            return "Bad sector";
        case DISK_BAD_CYLINDER:
            return "Bad cylinder";
        case DISK_MEDIA_TYPE_NOT_FOUND:
            return "Media type not found";
        case DISK_INVALID_NUM_SECTORS:
            return "Invalid number of sectors";
        case DISK_CTRL_DATA_ADDR_MARK:
            return "Control data address mark detected";
        case DISK_DMA_OUT_OF_RANGE:
            return "DMA out of range";
        case DISK_CRC_ECC_DATA_ERR:
            return "CRC/ECC data error";
        case DISK_ECC_CORRECTED_DATA_ERR:
            return "ECC corrected data error";
        case DISK_CONTROLLER_FAILURE:
            return "Controller failure";
        case DISK_SEEK_FAILURE:
            return "Seek failure";
        case DISK_DRIVE_TIMED_OUT:
            return "Drive timed out";
        case DISK_DRIVE_NOT_READY:
            return "Drive not ready";
        case DISK_UNDEFINED_ERR:
            return "Undefined error";
        case DISK_WRITE_FAULT:
            return "Write fault";
        case DISK_STATUS_ERR:
            return "Status error";
        case DISK_SENSE_OPERATION_FAILED:
            return "Sense operation failed";
        default:
            return "Unknown disk status";
    }
}

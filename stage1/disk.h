#ifndef DISK_H
#define DISK_H

#include <types.h>

typedef enum {
    DISK_SUCCESS                = 0x00,
    DISK_INVALID_COMMAND        = 0x01,
    DISK_ADDR_MARK_NOT_FOUND    = 0x02,
    DISK_WRITE_ON_PROT_DISK     = 0x03,
    DISK_SECTOR_NOT_FOUND       = 0x04,
    DISK_RESET_FAILED           = 0x05,
    DISK_CHANGE_LINE_ACTIVE     = 0x06,
    DISK_PARAM_ACTIVITY_FAILED  = 0x07,
    DISK_DMA_OVERRUN            = 0x08,
    DISK_DMA_OVER_BOUNDARY      = 0x09,
    DISK_BAD_SECTOR             = 0x0a,
    DISK_BAD_CYLINDER           = 0x0b,
    DISK_MEDIA_TYPE_NOT_FOUND   = 0x0c,
    DISK_INVALID_NUM_SECTORS    = 0x0d,
    DISK_CTRL_DATA_ADDR_MARK    = 0x0e,
    DISK_DMA_OUT_OF_RANGE       = 0x0f,
    DISK_CRC_ECC_DATA_ERR       = 0x10,
    DISK_ECC_CORRECTED_DATA_ERR = 0x11,
    DISK_CONTROLLER_FAILURE     = 0x20,
    DISK_SEEK_FAILURE           = 0x40,
    DISK_DRIVE_TIMED_OUT        = 0x80,
    DISK_DRIVE_NOT_READY        = 0xaa,
    DISK_UNDEFINED_ERR          = 0xbb,
    DISK_WRITE_FAULT            = 0xcc,
    DISK_STATUS_ERR             = 0xe0,
    DISK_SENSE_OPERATION_FAILED = 0xff,
} disk_status_t;

const char *disk_status_to_str(disk_status_t status);
disk_status_t disk_read(uint8_t drive_number, uint64_t lba, uint8_t *buffer, uint32_t sector_count);


#endif

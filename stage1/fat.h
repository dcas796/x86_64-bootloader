#ifndef FAT_H
#define FAT_H

#include <types.h>
#include "disk.h"

/* === DEFINITIONS === */

#define FAT_MAX_LENGTH_SHORT_NAME 8
#define FAT_MAX_LENGTH_SHORT_EXTENSION 3

/* === STRUCTURES & ENUMS === */

typedef struct PACKED {
    uint8_t  jmp[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
} fat_bpb_common_t;

typedef struct PACKED {
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  file_system_type[8];
} fat_bpb_extended_fat12_16_t;

typedef struct PACKED {
    uint32_t  fat_size_32;
    uint16_t  extended_flags;
    uint16_t  fs_version;
    uint32_t  root_cluster;
    uint16_t  fs_info;
    uint16_t  backup_boot_sector;
    uint8_t   reserved[12];
    uint8_t   drive_number;
    uint8_t   reserved1;
    uint8_t   boot_signature;
    uint32_t  volume_id;
    uint8_t   volume_label[11];
    uint8_t   file_system_type[8];
} fat_bpb_extended_fat32_t;

typedef struct PACKED {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  ntres;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;
} fat_dirent_t;

/* LFN entry layout (attribute 0x0F) */
typedef struct PACKED {
    uint8_t  ord;         /* sequence number (1..n), last has 0x40 set */
    uint16_t name1[5];    /* 5 UTF-16 chars */
    uint8_t  attr;        /* 0x0F */
    uint8_t  type;        /* 0 */
    uint8_t  chksum;      /* checksum of associated short name */
    uint16_t name2[6];    /* 6 UTF-16 chars */
    uint16_t fst_clus_lo; /* 0 */
    uint16_t name3[2];    /* 2 UTF-16 chars */
} fat_lfn_t;

typedef enum {
    FAT_ATTR_READ_ONLY  = 0x01,
    FAT_ATTR_HIDDEN     = 0x02,
    FAT_ATTR_SYSTEM     = 0x04,
    FAT_ATTR_VOLUME_ID  = 0x08,
    FAT_ATTR_DIRECTORY  = 0x10,
    FAT_ATTR_ARCHIVE    = 0x20,
    FAT_ATTR_LONG_NAME  = FAT_ATTR_READ_ONLY |
                            FAT_ATTR_HIDDEN |
                            FAT_ATTR_SYSTEM |
                            FAT_ATTR_VOLUME_ID,
} fat_attr_t;

typedef enum {
    FAT_TYPE_12,
    FAT_TYPE_16,
    FAT_TYPE_32
} fat_type_t;

typedef struct PACKED {
    fat_type_t type;
    fat_bpb_common_t bpb;
    union {
        fat_bpb_extended_fat12_16_t _12_16;
        fat_bpb_extended_fat32_t _32;
    } bpb_ext;
} fat_t;

/* === PUBLIC API === */

bool is_dir_free(const fat_dirent_t *dir);
void get_short_name(const fat_dirent_t *dir, char name[FAT_MAX_LENGTH_SHORT_NAME + 1]);
void get_short_extension(const fat_dirent_t *dir, char ext[FAT_MAX_LENGTH_SHORT_EXTENSION + 1]);
disk_status_t fat_mount(fat_t *fat, uint8_t drive_num);

#endif

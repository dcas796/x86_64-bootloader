#include "fat.h"
#include <memory_map.h>
#include "mem.h"

/* === PRIVATE FUNCTIONS === */

static uint32_t get_count_of_clusters(const fat_t *fat) {
    if (fat->bpb.bytes_per_sector == 0) return 0;

    uint32_t root_dir_sectors = ((fat->bpb.root_entry_count * 32) + (fat->bpb.bytes_per_sector - 1)) / fat->bpb.bytes_per_sector;

    uint32_t fat_size;
    if (fat->bpb.fat_size_16 != 0) {
        fat_size = fat->bpb.fat_size_16;
    } else {
        fat_size = fat->bpb_ext_32.fat_size_32;
    }

    uint32_t total_sectors;
    if (fat->bpb.total_sectors_16 != 0) {
        total_sectors = fat->bpb.total_sectors_16;
    } else {
        total_sectors = fat->bpb.total_sectors_32;
    }

    uint32_t data_sectors = total_sectors - (fat->bpb.reserved_sectors + (fat->bpb.num_fats * fat_size) + root_dir_sectors);

    if (fat->bpb.sectors_per_cluster == 0) return 0;
    return data_sectors / fat->bpb.sectors_per_cluster;
}

static uint32_t get_max_valid_cluster_num(const fat_t *fat) {
    return get_count_of_clusters(fat) + 1;
}

static fat_type_t get_fat_type(const fat_t *fat) {
    uint32_t count_of_clusters = get_count_of_clusters(fat);

    if (count_of_clusters < 4085) {
        return FAT_TYPE_12;
    } else if (count_of_clusters < 65525) {
        return FAT_TYPE_16;
    } else {
        return FAT_TYPE_32;
    }
}

/* === PUBLIC API === */

/*
 * Check if a directory entry is free.
 *
 * Parameters:
 *   dir: Pointer to the directory entry.
 *
 * Returns:
 *   True if the directory entry is free, false otherwise.
 */
bool is_dir_free(const fat_dirent_t *dir) {
    return dir->name[0] == 0xe5 || dir->name[0] == 0x00;
}

/*
 * Get the short name of a directory entry.
 *
 * Parameters:
 *   dir: Pointer to the directory entry.
 *   name: Buffer to store the short name (null-terminated).
 */
void get_short_name(const fat_dirent_t *dir, char name[9]) {
    int i = 0;
    while (i <= 7 && dir->name[i] != 0x20) {
        name[i] = dir->name[i];
        ++i;
    }
    name[i] = '\0';
    return;
}

/*
 * Get the short extension of a directory entry.
 *
 * Parameters:
 *   dir: Pointer to the directory entry.
 *   ext: Buffer to store the short extension (null-terminated).
 */
void get_short_extension(const fat_dirent_t *dir, char ext[4]) {
    int i = 8;
    while (i <= 11 && dir->name[i] != 0x20) {
        ext[i - 8] = dir->name[i];
        ++i;
    }
    ext[i - 8] = '\0';
    return;
}

/*
 * Initialize a FAT filesystem from a drive.
 *
 * Parameters:
 *   drive_num: The drive number.
 *   fat: Pointer to the FAT filesystem.
 *
 * Returns:
 *   The status of the operation.
 */
disk_status_t fat_from_drive(uint8_t drive_num, fat_t *fat) {
    /* Store the buffer temporarily in this address */
    uint8_t *buffer = (uint8_t*)FREE_MEM_ADDR;
    disk_status_t status = disk_read(drive_num, 0, 0, buffer, 1);
    if (status != DISK_SUCCESS) return status;
    memcpy(&fat->bpb, buffer, sizeof(fat_bpb_common_t) + sizeof(fat_bpb_extended_fat32_t)); /* the size of the fat32 extended bpb is the largest, so copy that */
    fat->type = get_fat_type(fat);
    return DISK_SUCCESS;
}

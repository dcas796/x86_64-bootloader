#include "fat.h"
#include <memory_map.h>

/* === PRIVATE FUNCTIONS === */

static uint32_t get_max_valid_cluster_num(uint32_t cluster_count) {
    return cluster_count + 1;
}

static fat_type_t get_fat_type(uint32_t cluster_count) {
    if (cluster_count < 4085) {
        return FAT_TYPE_12;
    }
    if (cluster_count < 65525) {
        return FAT_TYPE_16;
    }
    return FAT_TYPE_32;
}

static uint32_t get_count_of_clusters(const fat_bpb_t *bpb, uint32_t total_sectors, uint32_t first_data_sector) {
    uint32_t data_sectors = total_sectors - first_data_sector;

    if (bpb->sectors_per_cluster == 0) return 0;
    return data_sectors / bpb->sectors_per_cluster;
}

static uint32_t get_first_data_sector(const fat_bpb_t *bpb, uint32_t fat_size) {
    if (bpb->bytes_per_sector == 0) return 0;

    uint32_t root_dir_sectors = (bpb->root_entry_count * 32 + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    return bpb->reserved_sectors + bpb->num_fats * fat_size + root_dir_sectors;
}

static uint32_t get_total_sectors(const fat_bpb_t *bpb) {
    if (bpb->total_sectors_16 != 0) {
        return bpb->total_sectors_16;
    }
    return bpb->total_sectors_32;
}

static uint32_t get_fat_size(const fat_bpb_t *bpb, const fat_bpb_ext_t *bpb_ext) {
    if (bpb->fat_size_16 != 0) {
        return bpb->fat_size_16;
    }
    return bpb_ext->_32.fat_size_32;
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
    while (i < 11 && dir->name[i] != 0x20) {
        ext[i - 8] = dir->name[i];
        ++i;
    }
    ext[i - 8] = '\0';
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
disk_status_t fat_mount(fat_t *fat_out, uint8_t drive_num) {
    /* Store the buffer temporarily in this address */
    uint8_t *buffer = (uint8_t*)FREE_MEM_ADDR;
    disk_status_t status = disk_read(drive_num, 0, 0, buffer, 1);
    if (status != DISK_SUCCESS) return status;

    typedef struct PACKED {
        fat_bpb_t bpb;
        fat_bpb_ext_t bpb_ext;
    } _bpb_complete_t;

    auto temp = (const _bpb_complete_t*)buffer;

    uint32_t fat_size = get_fat_size(&temp->bpb, &temp->bpb_ext);
    uint32_t total_sectors = get_total_sectors(&temp->bpb);
    uint32_t first_data_sector = get_first_data_sector(&temp->bpb, fat_size);
    uint32_t cluster_count = get_count_of_clusters(&temp->bpb, total_sectors, first_data_sector);
    uint32_t type = get_fat_type(cluster_count);

    *fat_out = (fat_t){
        .type = type,
        .first_data_sector = first_data_sector,
        .total_sectors = total_sectors,
        .cluster_count = cluster_count,
        .fat_size = fat_size,
        .bpb = temp->bpb,
        .bpb_ext = temp->bpb_ext,
    };

    return DISK_SUCCESS;
}

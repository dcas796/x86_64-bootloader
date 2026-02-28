#include "fat.h"
#include <memory_map.h>

#include "math.h"
#include "mem.h"
#include "string.h"

#define MAX_NAME_LEN 255
#define MAX_LFN_LEN  255

/* === PRIVATE STRUCTS === */
typedef enum {
    FAT_CLUSTER_VALID,
    FAT_CLUSTER_END,
    FAT_CLUSTER_BAD,
    FAT_CLUSTER_DISK_FAILURE,
    FAT_CLUSTER_UNKNOWN_TYPE,
} fat_cluster_result_t;

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

static uint32_t get_first_fat_sector(const fat_bpb_t *bpb) {
    return bpb->reserved_sectors;
}

static uint16_t get_root_dir_sectors(const fat_bpb_t *bpb) {
    if (bpb->bytes_per_sector == 0) return 0;

    return (uint16_t)(((uint32_t)bpb->root_entry_count * 32 + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector);
}

static uint32_t get_first_data_sector(const fat_bpb_t *bpb, uint32_t fat_size, uint16_t root_dir_sectors) {
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

static uint32_t cluster_to_lba(const fat_t *fat, uint32_t cluster) {
    return fat->first_data_sector + (cluster - 2) * fat->bpb.sectors_per_cluster;
}

static fat_cluster_result_t fat_next(const fat_t *fat, uint32_t cluster, uint32_t *next) {
    uint32_t fat_offset;

    switch (fat->type) {
        case FAT_TYPE_12:
            fat_offset = cluster + cluster / 2;
            break;
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            break;
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            break;
        default:
            return FAT_CLUSTER_UNKNOWN_TYPE;
    }

    uint32_t fat_sector = fat->first_fat_sector + fat_offset / fat->bpb.bytes_per_sector;
    uint32_t ent_offset = fat_offset % fat->bpb.bytes_per_sector;

    auto buffer = (uint8_t*)FREE_MEM_ADDR;
    /* TODO: cache the FAT? */
    disk_status_t status = disk_read(fat->drive_number, fat_sector, buffer, 2);
    if (status != DISK_SUCCESS) return FAT_CLUSTER_DISK_FAILURE;

    fat_cluster_result_t result = FAT_CLUSTER_VALID;
    switch (fat->type) {
        case FAT_TYPE_12:
            uint16_t table_value_12 = *(uint16_t*)&buffer[ent_offset];
            *next = cluster & 1 ? table_value_12 >> 4 : table_value_12 & 0xFFF;

            if (*next >= 0xFF8) result = FAT_CLUSTER_END;
            if (*next == 0xFF7) result = FAT_CLUSTER_BAD;

            break;
        case FAT_TYPE_16:
            uint16_t table_value_16 = *(uint16_t*)&buffer[ent_offset];
            *next = table_value_16;

            if (*next >= 0xFFF8) result = FAT_CLUSTER_END;
            if (*next == 0xFFF7) result = FAT_CLUSTER_BAD;

            break;
        case FAT_TYPE_32:
            uint32_t table_value_32 = *(uint32_t*)&buffer[ent_offset];
            *next = table_value_32 & 0x0FFFFFFF;

            if (*next >= 0x0FFFFFF8) result = FAT_CLUSTER_END;
            if (*next == 0x0FFFFFF7) result = FAT_CLUSTER_BAD;

            break;
        default:
            return FAT_CLUSTER_UNKNOWN_TYPE;
    }

    return result;
}

static fat_cluster_result_t fat_next_not_bad(const fat_t *fat, uint32_t *cluster) {
    fat_cluster_result_t result;
    do {
        result = fat_next(fat, *cluster, cluster);
    } while (result == FAT_CLUSTER_BAD);
    return result;
}

static bool convert_to_83_filename_with_terminator(const char *name, char filename_out[FAT_NAME_LEN + 1]) {
    for (uint8_t i = 0; i < FAT_NAME_LEN; ++i) {
        filename_out[i] = ' ';
    }
    filename_out[FAT_NAME_LEN] = '\0';

    uint8_t i = 0;
    while (i < FAT_MAX_LENGTH_SHORT_NAME && name[i] != '.' && name[i] != '\0') {
        filename_out[i] = toupper(name[i]);
        ++i;
    }

    if (name[i] != '.' && name[i] != '\0') return false;

    if (name[i] != '\0') {
        uint8_t j = 0;
        while (j < FAT_MAX_LENGTH_SHORT_EXTENSION && name[i + j + 1] != '\0') {
            filename_out[FAT_MAX_LENGTH_SHORT_NAME + j] = toupper(name[i + j + 1]);
            ++j;
        }

        if (j == FAT_MAX_LENGTH_SHORT_EXTENSION && name[i + j + 1] != '\0') return false;
    }

    return true;
}

static bool search_for_entry(
    [[maybe_unused]] const fat_t *fat,
    const fat_dirent_t *entries,
    uint32_t entries_size,
    const char *name,
    fat_dirent_t *dir_out)
{
    char converted_name[FAT_NAME_LEN + 1];
    bool is_valid_name = convert_to_83_filename_with_terminator(name, converted_name);
    if (!is_valid_name) return false;

    uint32_t i = 0;
    while (i < entries_size && entries[i].name[0] != 0) {
        if (entries[i].name[0] == 0xE5 || (entries[i].attr & FAT_ATTR_VOLUME_ID) != 0) {
            ++i;
            continue;
        }

        if (entries[i].attr == FAT_ATTR_LONG_NAME) {
            /* TODO: implement LFN */
            // fat_lfn_t lfn = *(fat_lfn_t*)&entries[i];
            // memcpy(&lfn_buffer[lfn_index], &lfn.name1, sizeof(lfn.name1));
            // lfn_index += sizeof(lfn.name1) / sizeof(uint16_t);
            // memcpy(&lfn_buffer[lfn_index], &lfn.name2, sizeof(lfn.name2));
            // lfn_index += sizeof(lfn.name2) / sizeof(uint16_t);
            // memcpy(&lfn_buffer[lfn_index], &lfn.name3, sizeof(lfn.name3));
            // lfn_index += sizeof(lfn.name3) / sizeof(uint16_t);
        } else {
            char entry_name[sizeof(entries[i].name) + 1];
            memcpy(entry_name, entries[i].name, sizeof(entries[i].name));
            entry_name[sizeof(entries[i].name)] = '\0';
            if (streq(entry_name, converted_name)) {
                *dir_out = entries[i];
                return true;
            }
        }
        ++i;
    }

    return false;
}

static bool search_root_for_entry(
    const fat_t *fat,
    const char *name,
    fat_dirent_t *dir_out) {
    uint32_t first_root_dir_sector = fat->first_data_sector - fat->root_dir_sectors;
    uint8_t *buffer = (uint8_t*)FREE_MEM_ADDR;

    disk_status_t status = disk_read(fat->drive_number, first_root_dir_sector, buffer, fat->root_dir_sectors);
    if (status != DISK_SUCCESS) return false;

    const fat_dirent_t *entries = (fat_dirent_t*)buffer;
    return search_for_entry(fat, entries, fat->bpb.root_entry_count, name, dir_out);
}

static void get_next_segment(char **path, char segment[MAX_NAME_LEN + 1]) {
    if ((*path)[0] != '/') {
        segment[0] = '\0';
        return;
    }

    uint32_t i = 0;
    while (i < MAX_NAME_LEN && (*path)[i + 1] != '\0' && (*path)[i + 1] != '/') {
        segment[i] = (*path)[i + 1];
        ++i;
    }
    segment[i] = '\0';
    *path += i + 1;
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
    disk_status_t status = disk_read(drive_num, 0, buffer, 1);
    if (status != DISK_SUCCESS) return status;

    typedef struct PACKED {
        fat_bpb_t bpb;
        fat_bpb_ext_t bpb_ext;
    } _bpb_complete_t;

    auto temp = (const _bpb_complete_t*)buffer;

    uint32_t fat_size = get_fat_size(&temp->bpb, &temp->bpb_ext);
    uint32_t total_sectors = get_total_sectors(&temp->bpb);
    uint32_t first_fat_sector = get_first_fat_sector(&temp->bpb);
    uint16_t root_dir_sectors = get_root_dir_sectors(&temp->bpb);
    uint32_t first_data_sector = get_first_data_sector(&temp->bpb, fat_size, root_dir_sectors);
    uint32_t cluster_count = get_count_of_clusters(&temp->bpb, total_sectors, first_data_sector);
    uint32_t type = get_fat_type(cluster_count);

    *fat_out = (fat_t){
        .drive_number = drive_num,
        .type = type,
        .first_fat_sector = first_fat_sector,
        .first_data_sector = first_data_sector,
        .root_dir_sectors = root_dir_sectors,
        .total_sectors = total_sectors,
        .cluster_count = cluster_count,
        .fat_size = fat_size,
        .bpb = temp->bpb,
        .bpb_ext = temp->bpb_ext,
    };

    return DISK_SUCCESS;
}

fat_result_t fat_open(const fat_t *fat, char *path, fat_file_t* file_out) {
    fat_dirent_t current_directory;

    char path_segment[MAX_NAME_LEN + 1];
    get_next_segment(&path, path_segment);

    if (path_segment[0] == '\0') return FAT_FILE_NOT_FOUND;

    switch (fat->type) {
        case FAT_TYPE_12:
        case FAT_TYPE_16:
            bool found = search_root_for_entry(fat, path_segment, &current_directory);
            if (!found) return FAT_FILE_NOT_FOUND;
            get_next_segment(&path, path_segment);
            break;
        case FAT_TYPE_32:
            current_directory = (fat_dirent_t){
                .fst_clus_hi = fat->bpb_ext._32.root_cluster >> 16,
                .fst_clus_lo = fat->bpb_ext._32.root_cluster & 0xffff,
            };
            break;
        default:
            return FAT_UNKNOWN_FAT_TYPE;
    }

    uint32_t entry_count_per_cluster = fat->bpb.sectors_per_cluster * fat->bpb.bytes_per_sector / sizeof(fat_dirent_t);

    while (path_segment[0] != '\0') {
        bool found = false;
        uint32_t current_dir_cluster = (uint32_t)current_directory.fst_clus_hi << 16 | current_directory.fst_clus_lo;
        fat_cluster_result_t cluster_result = FAT_CLUSTER_VALID;
        while (!found && cluster_result == FAT_CLUSTER_VALID) {
            uint8_t *buffer = (uint8_t*)FREE_MEM_ADDR;
            disk_status_t status = disk_read(
                fat->drive_number,
                cluster_to_lba(fat, current_dir_cluster),
                buffer,
                fat->bpb.sectors_per_cluster);
            if (status != DISK_SUCCESS) return FAT_DISK_ERROR;

            const fat_dirent_t *entries = (fat_dirent_t*)buffer;
            // uint16_t lfn_buffer[MAX_LFN_LEN + 1];
            // uint16_t lfn_index = 0;

            found = search_for_entry(fat, entries, entry_count_per_cluster, path_segment, &current_directory);

            if (!found) {
                cluster_result = fat_next_not_bad(fat, &current_dir_cluster);
            }
        }

        if (!found) return FAT_FILE_NOT_FOUND;

        get_next_segment(&path, path_segment);
    }

    if ((current_directory.attr & FAT_ATTR_DIRECTORY) != 0) return FAT_IS_DIRECTORY;

    *file_out = (fat_file_t){
        .entry = current_directory,
    };

    return FAT_SUCCESS;
}

fat_result_t fat_read(const fat_t *fat, const fat_file_t *file, uint32_t offset, uint32_t length, void *buffer) {
    uint32_t bytes_per_cluster = fat->bpb.sectors_per_cluster * fat->bpb.bytes_per_sector;

    uint32_t cluster_offset = offset / bytes_per_cluster;
    uint32_t start_cluster = (file->entry.fst_clus_hi << 16) | file->entry.fst_clus_lo;

    for (uint32_t i = 0; i < cluster_offset; ++i) {
        fat_cluster_result_t cluster_result = fat_next_not_bad(fat, &start_cluster);
        if (cluster_result != FAT_CLUSTER_VALID) return FAT_OUT_OF_FILE_BOUNDS;
    }

    uint32_t current_cluster = start_cluster;

    while (length != 0) {
        uint32_t sector_offset = (offset % bytes_per_cluster) / fat->bpb.bytes_per_sector;
        uint32_t length_to_read = min(length, bytes_per_cluster);
        uint16_t sectors_to_read = (uint16_t)((length_to_read + fat->bpb.bytes_per_sector - 1) / fat->bpb.bytes_per_sector);
        uint32_t byte_offset = offset % fat->bpb.bytes_per_sector;

        bool use_temp_buffer = byte_offset != 0 || length_to_read % fat->bpb.bytes_per_sector != 0;
        uint8_t *temp_buffer = use_temp_buffer ? (uint8_t*)FREE_MEM_ADDR : buffer;
        disk_status_t status = disk_read(
            fat->drive_number,
            cluster_to_lba(fat, current_cluster) + sector_offset,
            temp_buffer,
            sectors_to_read
            );
        if (status != DISK_SUCCESS) return FAT_DISK_ERROR;

        if (use_temp_buffer) {
            memcpy(buffer, temp_buffer + byte_offset, length_to_read);
        }

        offset = 0;
        buffer += length_to_read;
        length -= length_to_read;

        if (length != 0) {
            fat_cluster_result_t cluster_result = fat_next_not_bad(fat, &start_cluster);
            if (cluster_result != FAT_CLUSTER_VALID) return FAT_OUT_OF_FILE_BOUNDS;
        }
    }

    return FAT_SUCCESS;
}

const char *fat_result_to_str(fat_result_t result) {
    switch (result) {
        case FAT_SUCCESS:
            return "Success";
        case FAT_UNKNOWN_FAT_TYPE:
            return "Unknown FAT type";
        case FAT_DISK_ERROR:
            return "Disk Error";
        case FAT_NOT_ABSOLUTE_PATH:
            return "Not absolute path";
        case FAT_FILE_NOT_FOUND:
            return "File not found";
        case FAT_IS_DIRECTORY:
            return "Is a directory";
        case FAT_UNKNOWN:
            return "Unknown error";
        default:
            return "Unknown (not a recognized error)";
    }
}


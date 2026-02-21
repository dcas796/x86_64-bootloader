#include <types.h>
#include "print.h"
#include "string.h"
#include "fat.h"
#include "disk.h"

extern uint8_t boot_drive;

void main() {
    puts("stage2\r\n");
    char boot_drive_str[3];
    puts("Boot drive: 0x");
    itoa(boot_drive, boot_drive_str, 16);
    puts(boot_drive_str);
    puts("\r\n");

    fat_t fat;
    puts("Reading FAT filesystem...\r\n");
    disk_status_t status = fat_from_drive(boot_drive, &fat);
    if (status != DISK_SUCCESS) {
        puts("Error loading FAT filesystem. Reason: ");
        puts(disk_status_to_str(status));
        puts("\r\n");
        return;
    }
    puts("Loaded FAT filesystem.\r\n");
    puts("Detected FAT type: ");
    switch (fat.type) {
        case FAT_TYPE_12:
            puts("FAT12");
            break;
        case FAT_TYPE_16:
            puts("FAT16");
            break;
        case FAT_TYPE_32:
            puts("FAT32");
            break;
        default:
            puts("Unknown");
            break;
    }
    puts("\r\n");
}

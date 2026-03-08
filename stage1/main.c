#include <types.h>
#include "print.h"
#include "string.h"
#include "fat.h"
#include "disk.h"
#include "elf.h"
#include "mem.h"
#include "options_parser.h"

extern uint8_t boot_drive;

#define BOOT_OPTIONS_TXT            "/boot/options.txt"
#define BOOT_OPTIONS_TXT_MAX_LEN    1024

void main() {
    puts("stage2\r\n");
    char boot_drive_str[3];
    puts("Boot drive: 0x");
    itoa(boot_drive, boot_drive_str, 16);
    puts(boot_drive_str);
    puts("\r\n");

    fat_t fat;
    puts("Reading FAT filesystem...\r\n");
    disk_status_t status = fat_mount(&fat, boot_drive);
    if (status != DISK_SUCCESS) {
        puts("Error loading FAT filesystem. Reason: ");
        puts(disk_status_to_str(status));
        puts("\r\n");
        return;
    }
    puts("Loaded FAT filesystem.\r\n");
    puts("Detected FAT type: ");
    char volume_label[12];
    switch (fat.type) {
        case FAT_TYPE_12:
            puts("FAT12");
            memcpy(volume_label, &fat.bpb_ext._12_16.volume_label, sizeof(volume_label));
            break;
        case FAT_TYPE_16:
            puts("FAT16");
            memcpy(volume_label, &fat.bpb_ext._12_16.volume_label, sizeof(volume_label));
            break;
        case FAT_TYPE_32:
            puts("FAT32");
            memcpy(volume_label, &fat.bpb_ext._32.volume_label, sizeof(volume_label));
            break;
        default:
            puts("Unknown");
            break;
    }
    volume_label[11] = '\0';
    puts("\r\n");
    puts("Reading from: ");
    puts(volume_label);
    puts("\r\n");

    fat_file_t options_txt;
    puts("Opening file: " BOOT_OPTIONS_TXT "\r\n");
    fat_result_t fat_result = fat_open(&fat, BOOT_OPTIONS_TXT, &options_txt);
    if (fat_result != FAT_SUCCESS) {
        puts("Error opening file. Reason: ");
        puts(fat_result_to_str(fat_result));
        puts("\r\n");
        return;
    }

    if (options_txt.entry.file_size > BOOT_OPTIONS_TXT_MAX_LEN) {
        puts("/boot/options.txt is larger than 1024 bytes\r\n");
        return;
    }

    puts("Reading file: " BOOT_OPTIONS_TXT "\r\n");
    char options_txt_contents[BOOT_OPTIONS_TXT_MAX_LEN + 1];
    fat_result = fat_read(&options_txt, 0, options_txt.entry.file_size, options_txt_contents);
    if (fat_result != FAT_SUCCESS) {
        puts("Error reading file. Reason: ");
        puts(fat_result_to_str(fat_result));
        puts("\r\n");
        return;
    }
    options_txt_contents[options_txt.entry.file_size] = '\0';

    puts("Parsing file...\r\n");
    boot_options_t boot_options;
    parser_result_t parser_result = parse(options_txt_contents, &boot_options);
    if (parser_result != PARSER_SUCCESS) {
        puts("Error parsing file. Reason: ");
        puts(parser_result_to_str(parser_result));
        puts("\r\n");
        return;
    }
    puts("Boot binary path: ");
    puts(boot_options.boot_binary);
    puts("\r\nLoad offset: 0x");
    char load_offset_str[9];
    puts(itoa(boot_options.load_offset, load_offset_str, 16));
    puts("\r\n");

    puts("Opening boot binary...\r\n");
    fat_file_t boot_binary_file;
    fat_result = fat_open(&fat, boot_options.boot_binary, &boot_binary_file);
    if (fat_result != FAT_SUCCESS) {
        puts("Error opening file. Reason: ");
        puts(fat_result_to_str(fat_result));
        puts("\r\n");
        return;
    }

    puts("Parsing ELF...\r\n");
    elf_t elf;
    elf_result_t elf_result = elf_open(&elf, &boot_binary_file);
    if (elf_result != ELF_SUCCESS) {
        puts("Error opening file. Reason: ");
        puts(elf_result_to_str(elf_result));
        puts("\r\n");
        return;
    }

    puts("Loading ELF...\r\n");
    elf_result = elf_load(&elf);
    if (elf_result != ELF_SUCCESS) {
        puts("Error loading file. Reason: ");
        puts(elf_result_to_str(elf_result));
        puts("\r\n");
        return;
    }

    puts("Switching to protected mode & transferring control to boot binary...\r\n");
    ((void(*)(void(*)(const char *)))elf.header.entry_point)(puts);
    puts("Returned from boot\r\n");
}

#ifndef ELF_H
#define ELF_H

#include <types.h>
#include "fat.h"

typedef struct PACKED {
    uint8_t  magic[4];
    uint8_t  efi_class;
    uint8_t  endianness;
    uint8_t  header_version;
    uint8_t  abi;
    uint8_t  unused[8];
    uint16_t type;
    uint16_t machine;
    uint32_t elf_version;
    uint32_t entry_point;
    uint32_t program_header_table;
    uint32_t section_header_table;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_entry_size;
    uint16_t program_entry_count;
    uint16_t section_entry_size;
    uint16_t section_entry_count;
    uint16_t string_table_section_index;
} elf_header_t;

typedef enum {
    ELF_CLASS_NONE,
    ELF_CLASS_32,
    ELF_CLASS_64,
} elf_class_t;

typedef enum {
    ELF_DATA_NONE,
    ELF_DATA_2_LSB,
    ELF_DATA_2_MSB,
} elf_data_t;

typedef enum {
    ELF_VER_NONE,
    ELF_VER_CURRENT,
} elf_version_t;

typedef enum {
    ELF_OSABI_NONE,
    ELF_OSABI_SYSV,
    ELF_OSABI_HPUX,
    EFL_OSABI_NETBSD,
    ELF_OSABI_LINUX,
    ELF_OSABI_SOLARIS,
    ELF_OSABI_IRIX,
    ELF_OSABI_FREEBSD,
    ELF_OSABI_TRU64,
    ELF_OSABI_ARM,
    ELF_OSABI_STANDALONE,
} elf_osabi_t;

typedef enum {
    ELF_TYPE_NONE,
    ELF_TYPE_REL,
    ELF_TYPE_EXEC,
    ELF_TYPE_DYN,
    ELF_TYPE_CORE,
} elf_type_t;

typedef enum {
    ELF_MACHINE_NONE,
    ELF_MACHINE_M32,
    ELF_MACHINE_SPARC,
    ELF_MACHINE_386,
    ELF_MACHINE_68K,
    ELF_MACHINE_88K,
    ELF_MACHINE_880,
    ELF_MACHINE_860,
    ELF_MACHINE_MIPS,
    ELF_MACHINE_PARISC,
    ELF_MACHINE_SPARC32PLUS,
    ELF_MACHINE_PPC,
    ELF_MACHINE_PPC64,
    ELF_MACHINE_S390,
    ELF_MACHINE_ARM,
    ELF_MACHINE_SH,
    ELF_MACHINE_SPARCV9,
    ELF_MACHINE_IA_64,
    ELF_MACHINE_X86_64,
    ELF_MACHINE_VAX
} elf_machine_t;

typedef struct PACKED {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} elf_program_header_t;

typedef enum {
    ELF_PT_NULL,
    ELF_PT_LOAD,
    ELF_PT_DYNAMIC,
    ELF_PT_INTERP,
    ELF_PT_NOTE,
    ELF_PT_SHLIB,
    ELF_PT_PHDR
} elf_program_header_type_t;

typedef enum {
    ELF_SUCCESS,
    ELF_NOT_RECOGNIZED,
    ELF_UNSUPPORTED_ARCH,
    ELF_NOT_EXECUTABLE,
    ELF_FAT_ERROR,
    ELF_LOAD_UNDER_BOUNDARY,
} elf_result_t;

typedef struct {
    const fat_file_t *file;
    elf_header_t header;
} elf_t;

elf_result_t elf_open(elf_t *elf, const fat_file_t *file);
elf_result_t elf_load(const elf_t *elf);
const char *elf_result_to_str(elf_result_t result);

#endif

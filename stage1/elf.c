#include "elf.h"

#include "mem.h"
#include "memory_map.h"
#include "string.h"
#include "sysinfo_impl.h"

/* === CONSTANTS === */

static const char ELF_MAGIC[] = "\x7F" "ELF";

/* === PRIVATE FUNCTIONS === */

static elf_result_t validate_header(const elf_header_t *header) {
    char magic[sizeof(header->magic) + 1];
    memcpy(magic, header->magic, sizeof(header->magic));
    magic[sizeof(header->magic)] = '\0';

    if (!streq(ELF_MAGIC, magic)) return ELF_NOT_RECOGNIZED;

    if (header->efi_class != ELF_CLASS_32) return ELF_UNSUPPORTED_ARCH;
    if (header->machine != ELF_MACHINE_386 && header->machine != ELF_MACHINE_X86_64) return ELF_UNSUPPORTED_ARCH;
    if (header->type != ELF_TYPE_EXEC ||
        header->program_entry_count == 0 ||
        header->program_header_table == 0 ||
        header->entry_point == 0) return ELF_NOT_EXECUTABLE;

    return ELF_SUCCESS;
}

static bool is_intersecting_used_memregions(
    const elf_program_header_t *prog_header,
    const sysinfo_memregion_t* memory_regions)
{
    if (prog_header->p_memsz == 0) return true;

    for (const sysinfo_memregion_t *memregion = memory_regions; memregion != nullptr; memregion = memregion->next) {
        if (memregion->type == SYSINFO_MT_USABLE) continue;
        if (memregion->base_addr < prog_header->p_vaddr) {
            if (memregion->base_addr + memregion->size > prog_header->p_vaddr) {
                return true;
            }
        } else {
            if (prog_header->p_vaddr + prog_header->p_memsz > memregion->base_addr) {
                return true;
            }
        }
    }

    return false;
}

static void append_memregion(const elf_program_header_t *prog_header, sysinfo_memregion_t **memory_regions) {
    sysinfo_memregion_t *memregion = leak(sizeof(sysinfo_memregion_t));
    *memregion = (sysinfo_memregion_t){
        .next = nullptr,
        .base_addr = prog_header->p_vaddr,
        .size = prog_header->p_memsz,
        .type = SYSINFO_MT_ELF_EXECUTABLE,
        .is_volatile = false,
    };
    insert_sorted(memory_regions, memregion);
}

/* === PUBLIC API === */

elf_result_t elf_open(elf_t *elf, const fat_file_t *file) {
    elf->file = file;

    fat_result_t result = fat_read(file, 0, sizeof(elf->header), &elf->header);
    if (result != FAT_SUCCESS) return ELF_FAT_ERROR;

    return validate_header(&elf->header);
}

elf_result_t elf_load(const elf_t *elf, sysinfo_memregion_t **memory_regions) {
    uint16_t entry_count = elf->header.program_entry_count;
    uint16_t entry_size = elf->header.program_entry_size;
    uint32_t offset = elf->header.program_header_table;

    for (uint16_t i = 0; i < entry_count; ++i) {
        elf_program_header_t prog_header;
        fat_result_t result = fat_read(elf->file, offset + entry_size * i, entry_size, &prog_header);
        if (result != FAT_SUCCESS) return ELF_FAT_ERROR;

        if (prog_header.p_type != ELF_PT_LOAD) continue;
        if (prog_header.p_vaddr < REAL_MODE_LIMIT ||
            is_intersecting_used_memregions(&prog_header, *memory_regions))
            return ELF_LOAD_ON_RESERVED_MEMORY;

        append_memregion(&prog_header, memory_regions);

        result = fat_read(elf->file, prog_header.p_offset, prog_header.p_filesz, (void*)prog_header.p_vaddr);
        if (result != FAT_SUCCESS) return ELF_FAT_ERROR;
    }

    return ELF_SUCCESS;
}

const char *elf_result_to_str(elf_result_t result) {
    switch (result) {
        case ELF_SUCCESS:
            return "Success";
        case ELF_NOT_RECOGNIZED:
            return "Not recognized as valid ELF";
        case ELF_UNSUPPORTED_ARCH:
            return "Unsupported architecture";
        case ELF_NOT_EXECUTABLE:
            return "Not executable";
        case ELF_FAT_ERROR:
            return "FAT error";
        case ELF_LOAD_ON_RESERVED_MEMORY:
            return "Load on a reserved region of memory";
        default:
            return "Unknown";
    }
}

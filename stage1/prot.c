#include "prot.h"

#include "memory_map.h"

static control_result_t get_stack_pointer(
    const sysinfo_memregion_t *mem_regions,
    uint8_t alignment,
    uint32_t reserve_space,
    uint32_t *stack_pointer)
{
    sysinfo_memregion_t const *largest = mem_regions;
    sysinfo_memregion_t const *current = mem_regions;
    while (current != nullptr) {
        if (current->size >= largest->size) {
            largest = current;
        }
        current = current->next;
    }

    if (largest == nullptr) return CONTROL_NO_SPACE_FOR_STACK;

    *stack_pointer = (uint32_t)(largest->base_addr + largest->size - 1 - reserve_space);
    *stack_pointer &= 0xffffffff << alignment;
    if (*stack_pointer < largest->base_addr) return CONTROL_NO_SPACE_FOR_STACK;
    *stack_pointer += reserve_space;

    return CONTROL_SUCCESS;
}

control_result_t transfer_control(void *entry_point, const sysinfo_t *info) {
    uint32_t stack_pointer;
    control_result_t result = get_stack_pointer(
        info->mem_regions,
        ELF_STACK_ALIGNMENT,
        sizeof(const sysinfo_t *) + sizeof(long),
        &stack_pointer);
    if (result != CONTROL_SUCCESS) return result;

    __asm__ volatile (
        "cli\n\t"
        "lgdt gdtr\n\t"
        "mov %%cr0, %%eax\n\t"
        "or $1, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        "ljmp $0x08, $1f\n\t"

        ".code32\n\t"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"

        "mov %2, %%esp\n\t"

        /* Push const sysinfo *info as an argument */
        "push %1\n\t"

        /* Push sentinel return address */
        "pushl $0\n\t"

        /* Jump to entry point */
        "jmp *%0\n\t"

        :
        : "r"((uint32_t)entry_point), "r"((uint32_t)info), "r"(stack_pointer)
        : "eax", "memory"
    );
    __builtin_unreachable();
}

const char *control_result_to_str(control_result_t result) {
    switch (result) {
        case CONTROL_SUCCESS:
            return "Success (something really bad happened).";
        case CONTROL_NO_SPACE_FOR_STACK:
            return "Not enough space in memory to place stack pointer";
        default:
            return "Unknown";
    }
}


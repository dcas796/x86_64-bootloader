#include "prot.h"

#include "memory_map.h"

void __attribute__((noreturn)) transfer_control(void *entry_point, const sysinfo_t *info) {
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

        "push %1\n\t"

        /* Push sentinel return address */
        "pushl $0\n\t"

        /* Jump to entry point */
        "jmp *%0\n\t"

        :
        : "r"((uint32_t)entry_point), "r"((uint32_t)info), "i"(PROTECTED_MODE_STACK_TOP + sizeof(sysinfo_t*) + sizeof(long))
        : "eax", "ecx", "esi", "edi", "memory"
    );
    __builtin_unreachable();
}

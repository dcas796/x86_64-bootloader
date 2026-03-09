#include "prot.h"

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

        /* Copy sysinfo_t by value onto the stack */
        "mov %2, %%ecx\n\t"        /* ecx = sizeof(sysinfo_t) in dwords */
        "sub %%ecx, %%esp\n\t"      /* make room on the stack */
        "mov %%esp, %%edi\n\t"      /* dst = new stack top */
        "mov %1, %%esi\n\t"         /* src = info pointer */
        "rep movsd\n\t"             /* copy ecx dwords from esi to edi */

        /* Push sentinel return address */
        "push $0\n\t"

        /* Jump to entry point */
        "jmp *%0\n\t"

        :
        : "r"((uint32_t)entry_point), "r"((uint32_t)info), "i"(sizeof(sysinfo_t) / 4)
        : "eax", "ecx", "esi", "edi", "memory"
    );
    __builtin_unreachable();
}

// x86_64-elf-gcc -m32 -Wall -Wextra -nostdlib -nostdinc -ffreestanding -I../../include -c test.c -o test.o && x86_64-elf-ld -m elf_i386 -Tlinker.ld test.o -o example.elf

#include <sysinfo.h>

void __attribute__((noreturn)) entry(sysinfo_t info) {
    auto vga = (unsigned short *)0xb8000;
    vga[0] = (0x0f << 8) | '#';
    vga[1] = (0x0f << 8) | (info.boot_drive - 2);

    while (true) {
        __asm__ volatile (
            "hlt"
            : :
        );
    }
}

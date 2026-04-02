#include "print.h"

void putc(char c) {
    __asm__ volatile (
        "int $0x10\n"
        :
        : "a" (0x0e << 8 | c), "b" (0)
        : "memory"
    );
}

void puts(const char *s) {
    char c;
    while ((c = *s++))
        putc(c);
}

extern void __attribute__((noreturn)) loop();

void __attribute__((noreturn)) panic(const char *s) {
    puts("PANIC: ");
    puts(s);
    loop();
}


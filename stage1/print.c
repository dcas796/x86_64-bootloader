#include "print.h"

void putc(char c) {
    __asm__ volatile (
        "movb $0x0e, %%ah\n"
        "movb %0, %%al\n"
        "xorb %%bh, %%bh\n"
        "int $0x10\n"
        :
        : "r" (c)
        : "ax", "bx"
    );
}

void puts(const char *s) {
    char c;
    while ((c = *s++))
        putc(c);
}

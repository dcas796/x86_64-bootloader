#include "print.h"

void putc(char c) {
    __asm__ volatile (
        "int $0x10\n"
        :
        : "a" (0x0e << 8 | c), "b" (0)
    );
}

void puts(const char *s) {
    char c;
    while ((c = *s++))
        putc(c);
}

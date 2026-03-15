#ifndef PRINT_H
#define PRINT_H

void putc(char c);
void puts(const char *s);
void __attribute__((noreturn)) panic(const char *s);

#endif

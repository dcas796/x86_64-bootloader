#ifndef STRING_H
#define STRING_H

#include <types.h>

size_t strlen(const char *str);
char *itoa(int value, char *str, uint8_t base);
bool atoi(const char *str, uint8_t base, uint32_t *out);
char *strnrev(char *str, size_t n);
bool streq(const char *s1, const char *s2);
char toupper(char c);

#endif

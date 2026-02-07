#ifndef STRING_H
#define STRING_H

unsigned int strlen(const char *str);
char *itoa(int value, char *str, unsigned int base);
char *strnrev(char *str, unsigned int size);

#endif

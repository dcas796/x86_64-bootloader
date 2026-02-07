#include "string.h"
#include <types.h>

unsigned int strlen(const char *str) {
    unsigned int len = 0;
    while (*str++) ++len;
    return len;
}

static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *itoa(int value, char *str, unsigned int base) {
    if (str == NULL || base > sizeof(alphabet)) return NULL;

    bool is_neg = false;
    if (value < 0) {
        is_neg = true;
        value = -value;
    }
    int i = 0;
    while (value > 0) {
        str[i] = alphabet[value % base];
        value /= base;
        ++i;
    }

    if (i == 0) {
        str[i++] = '0';
    }

    if (is_neg) {
        str[i++] = '-';
    }

    /* terminate the string */
    str[i] = '\x00';

    /* reverse the string */
    strnrev(str, i);

    return str;
}

char *strnrev(char *str, unsigned int size) {
    if (size == 0) return str;

    int i = 0;
    int j = size - 1;
    while (i < j) {
        /* xor swap */
        str[i] ^= str[j];
        str[j] ^= str[i];
        str[i] ^= str[j];
        i++;
        j--;
    }

    return str;
}

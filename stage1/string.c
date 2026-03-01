#include "string.h"
#include <types.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (*str++) ++len;
    return len;
}

static const char alphabet[] = "0123456789ABCDEF";
char *itoa(int value, char *str, uint8_t base) {
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

static int32_t find_in_alphabet(char d) {
    d = toupper(d);
    uint8_t i = 0;
    while (i < sizeof(alphabet)) {
        if (d == alphabet[i]) {
            return i;
        }
        ++i;
    }
    return -1;
}

bool atoi(const char *str, uint8_t base, uint32_t *out) {
    uint32_t i = 0;
    *out = 0;

    while (str[i] != '\0') {
        *out *= base;
        int32_t digit = find_in_alphabet(str[i]);
        if (digit < 0 || base <= digit) return false;
        *out += digit;
        ++i;
    }

    return true;
}

char *strnrev(char *str, size_t n) {
    if (n == 0) return str;

    size_t i = 0;
    size_t j = n - 1;
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

bool streq(const char *s1, const char *s2) {
    uint32_t i = 0;
    while (s1[i] != '\0' && s2[i] != '\0' && s1[i] == s2[i]) {
        ++i;
    }
    return s1[i] == s2[i];
}

char toupper(char c) {
    return 'a' <= c && c <= 'z' ? c - 32 : c;
}

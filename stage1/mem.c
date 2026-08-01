#include "mem.h"

#include "math.h"
#include "memory_map.h"
#include "print.h"

static void *ext_stack_top = (void*)EXTENDED_STACK_BASE;
static void *static_stack_top = (void*)STATIC_STACK_BASE;

/* Encoding instructions manually because GCC doesn't want to do its job and insists on using the 16-bit version of
 * `rep movsl` and `rep movsb`, resulting in truncation of the addresses into 16-bit.
 */
#define REP_MOVSL ".byte 0xF3, 0x67, 0x66, 0xA5\n\t"
#define REP_MOVSB ".byte 0xF3, 0x67, 0xA4\n\t"

#define REP_STOSL ".byte 0xF3, 0x67, 0x66, 0xAB\n\t"
#define REP_STOSB ".byte 0xF3, 0x67, 0xAA\n\t"

#define REP_CMPSL ".byte 0xF3, 0x67, 0x66, 0xA7\n\t"

void *memset(void *dst, uint8_t b, size_t n) {
    register uint32_t dst32 __asm__("edi") = (uint32_t)dst;
    register uint32_t dwords __asm__("ecx") = n / 4;
    register uint32_t val __asm__("eax") = b * 0x01010101u; // replicate byte across all 4 bytes
    size_t remaining = n % 4;

    __asm__ volatile (
        "cld\n\t"
        REP_STOSL
        "mov %[rem], %%ecx\n\t"
        REP_STOSB
        : "+D"(dst32), "+c"(dwords), "+a"(val)
        : [rem]"r"(remaining)
        : "memory"
    );

    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
    register uint32_t src32 __asm__("esi") = (uint32_t)src;
    register uint32_t dst32 __asm__("edi") = (uint32_t)dst;
    register uint32_t dwords __asm__("ecx") = n / 4;
    size_t remaining = n % 4;

    __asm__ volatile (
        "cld\n\t"
        REP_MOVSL
        "mov %[rem], %%ecx\n\t"
        REP_MOVSB
        : "+S"(src32), "+D"(dst32), "+c"(dwords)
        : [rem]"r"(remaining)
        : "memory"
    );

    return dst;
}

bool memeq(const void *p1, const void *p2, size_t n) {
    register uint32_t addr1 __asm__("esi") = (uint32_t)p1;
    register uint32_t addr2 __asm__("edi") = (uint32_t)p2;
    register uint32_t dwords __asm__("ecx") = n / 4;
    register uint32_t equal __asm__("eax");

    __asm__ volatile (
        "cld\n\t"
        "xor %%eax, %%eax\n\t"
        REP_CMPSL
        "sete %%al\n\t"
        "movzbl %%al, %%eax\n\t"
        : "+S"(addr1), "+D"(addr2), "+c"(dwords), "=a"(equal)
        :
        : "cc", "memory"
    );

    if (equal == 0) return false;

    auto s1 = (const uint8_t *)addr1;
    auto s2 = (const uint8_t *)addr2;
    size_t remaining = n % 4;
    for (size_t i = 0; i < remaining; i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }

    return true;
}

void *push(size_t n) {
    if ((uint32_t)ext_stack_top - EXTENDED_STACK_LIMIT < n) {
        panic("Overgrown extended stack");
    }
    ext_stack_top -= n;
    return ext_stack_top;
}

void *static_alloc(size_t n) {
    if ((uint32_t)static_stack_top - STATIC_STACK_LIMIT < n) {
        panic("Overgrown static allocations stack");
    }
    static_stack_top -= n;
    return static_stack_top;
}

void pop(size_t n) {
    if (EXTENDED_STACK_BASE - (uint32_t)ext_stack_top < n) {
        panic("Stack overflow");
    }
    ext_stack_top += n;
}

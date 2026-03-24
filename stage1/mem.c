#include "mem.h"

#include "math.h"
#include "memory_map.h"
#include "print.h"

static void *ext_stack_top = (void*)EXTENDED_STACK_BASE;
static void *leak_top = nullptr;

/* Encoding instructions manually because GCC doesn't want to do its job and insists on using the 16-bit version of
 * `rep movsl` and `rep movsb`, resulting in truncation of the addresses into 16-bit.
 */
#define REP_MOVSL ".byte 0xF3, 0x67, 0x66, 0xA5\n\t"
#define REP_MOVSB ".byte 0xF3, 0x67, 0xA4\n\t"

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

void *push(size_t n) {
    if ((uint32_t)ext_stack_top - EXTENDED_STACK_LIMIT < n) {
        panic("Overgrown stack");
    }
    ext_stack_top -= n;
    return ext_stack_top;
}

// TODO: for leaking, put it in another stack
void *leak(size_t n) {
    leak_top = push(n);
    return leak_top;
}

void pop(size_t n) {
    if (EXTENDED_STACK_BASE - (uint32_t)ext_stack_top < n) {
        panic("Stack overflow");
    }
    if (leak_top != nullptr && (uint32_t)leak_top - (uint32_t)ext_stack_top < n) {
        panic("Tried to pop intentionally leaked stack memory");
    }
    ext_stack_top += n;
}

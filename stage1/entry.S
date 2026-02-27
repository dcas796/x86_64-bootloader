#include <memory_map.h>

.code16gcc /* this is to be able to call C functions from here */
.org 0

.extern __stage1_end
.extern main

.section .text.start, "a"
.globl _start
_start:
    /* save the boot drive number passed in as an argment */
    popw %ax
    movb %al, boot_drive
    /* set up the stack */
    movw $STAGE1_STACK_TOP, %sp
    movw $STAGE1_STACK_BASE, %bp
jump_c:
    /* we are ready to call C */
    call main
loop:
    hlt
    jmp loop

.globl enable_unreal_mode
enable_unreal_mode:
    pushw %ax
    cli
    lgdt gdtr
    /* enable 32-bit protected mode */
    mov %cr0, %eax
    orb $1, %al
    mov %eax, %cr0

    /* data selector */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    /* leave 32-bit protected mode */
    mov %cr0, %eax
    and $0xFFFFFFFE, %eax
    mov %eax, %cr0
    popw %ax
    ret

.section .data

.globl boot_drive
boot_drive:
    .byte 0

gdtr:
   .word (gdt_end - gdt - 1)
   .word gdt
   .word 0

gdt:
gdt_null:
    .quad 0
    .quad 0
gdt_code:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .byte 0b11000000
    .byte 0
gdt_data:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .byte 0b11000000
    .byte 0
gdt_end:


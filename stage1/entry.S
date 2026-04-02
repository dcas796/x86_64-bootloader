#include <memory_map.h>

.code16gcc /* this is to be able to call C functions from here */
.org 0

.extern __stage1_end
.extern main

.section .bootloader_start, "ax"
.globl _start
_start:
    /* save the boot drive number passed in as an argment */
    popw %ax
    movb %al, boot_drive
    /* set up the stack */
    movw $STAGE1_STACK_TOP, %sp
    movw $STAGE1_STACK_BASE, %bp
    call enable_unreal_mode
jump_c:
    /* we are ready to call C */
    call main
.globl loop
loop:
    hlt
    jmp loop

.globl enable_unreal_mode
enable_unreal_mode:
    cli
    pushl %eax
    pushw %bx
    pushw %ds
    lgdt gdtr_unreal

    movl  %cr0, %eax
    orb $1, %al
    movl %eax, %cr0
    ljmp $0x8, $pmode
pmode:
    movw $0x10, %bx
    movw %bx, %ds

    andb $0xFE, %al
    movl %eax, %cr0
    ljmp $0x0, $unreal
unreal:
    popw %ds
    popw %bx
    popl %eax
    sti
    ret

.section .data

.globl boot_drive
boot_drive:
    .byte 0

gdtr_unreal:
   .word (gdt_unreal_end - gdt_unreal - 1)
   .long gdt_unreal

gdt_unreal:
gdt_unreal_null:
    .quad 0
gdt_unreal_code:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011010
    .byte 0b00000000
    .byte 0
gdt_unreal_data:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010010
    .byte 0b11001111
    .byte 0
gdt_unreal_end:

.globl gdtr
gdtr:
   .word (gdt_end - gdt - 1)
   .long gdt

gdt:
gdt_null:
    .quad 0
gdt_code:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011010
    .byte 0b11001111
    .byte 0
gdt_data:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010010
    .byte 0b11001111
    .byte 0
gdt_end:


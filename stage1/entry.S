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
    movw $STAGE1_STACK_BASE, %sp
    /* we are ready to call C */
    call main
loop:
    hlt
    jmp loop

.section .data

.globl boot_drive
boot_drive:
    .byte 0

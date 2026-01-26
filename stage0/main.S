.extern stack_bottom
.extern stack_top
.extern boot_entry_offset

.set STAGE1_OFFSET, 0x8000
.set STAGE1_SEGMENT, 0x0000

.code16

.section .bpb, "a"
/* immediately jump to after the bpb */
    .byte 0xeb
    .byte 0x58  /* this is the relative address to _start */
    .byte 0x90

/* BPB fields that will be populated correctly when the drive is formatted */
OEMName:
    .fill 8, 1, 0      /* 8 bytes */
BPB_BytsPerSec:
    .word   0
BPB_SecPerClus:
    .byte   0
BPB_RsvdSecCnt:
    .word   0           /* we only care about this one */
/* ... more BPB fields ... */

/* === PROGRAM START === */
.section .text
.globl _start
_start:
    /* clear interrupts, we don't want anyone messing this */
    cli

    /* set up the segment registers */
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    movw stack_bottom, %sp

    /* turn them back on */
    sti

    /* save boot drive */
    movb %dl, boot_drive

    /* set video mode */
    movb $0, %ah
    movb $3, %al
    int $0x10

    /* write a character */
    movw $stage0_str, %si
    call puts

    /* enable a20 line */
    inb $0x92, %al
    orb $2, %al
    outb %al, $0x92

    /* check int 13h extensions */
    movb $0x41, %ah
    movw $0x55aa, %bx
    int $0x13
    jc no_extensions    /* extensions not present */
    cmp $0xaa55, %bx
    jne no_extensions   /* somehow the carry flag not set, but something failed */
    andw $1, %cx
    jz no_extensions    /* extensions present, but not Device Access using the packet structure */

    /* the number of sectors to read */
    movw BPB_RsvdSecCnt, %cx
    cmp $0, %cx
    je nothing_to_do
read_loop:
    /* Read up to 127 sectors per call */
    movw    %cx, %bx
    cmpw    $127, %bx
    jbe     set_cnt
    movw    $127, %bx
set_cnt:
    /* set the sector count to read */
    movw %bx, dap_nsects
    /* read the sectors */
    movw $0x42, %ax
    int $0x13
    /* check for errors */
    jc disk_error
    /* decrement %cx */
    subw %bx, %cx
    jz read_complete
    /* advance LBA address by %bx */
    addw %bx, dap_lba+0
    adcw $0, dap_lba+2
    /* advance destination pointer by %bx * 512 */
    shlw $9, %bx
    addw %bx, dap_off
    adcw $0, dap_seg

    jmp read_loop
read_complete:
    /* jump to stage1 */
    ljmp $STAGE1_SEGMENT, $STAGE1_OFFSET
loop:
    hlt
    jmp loop

/* === MISC FUNCTIONS === */
no_extensions:
    pushw %cx
    movb %ah, %al
    pushw %ax
    movw $no_ext_str, %si
    call puts
    popw %ax
    call put_hex
    movb $',', %al
    call putc
    popw %ax
    call put_hex
    jmp loop

disk_error:
    movb %ah, %ch
    movw $disk_str, %si
    call puts
    movw $err_str, %si
    call puts
    movb %ch, %al
    call put_hex
    jmp loop

nothing_to_do:
    movw $nothing_str, %si
    call puts
    jmp loop


/*
  Prints the character in %al
*/
putc:
    movb $0x0e, %ah
    xorb %bh, %bh
    int $0x10
    ret

/*
  Prints a string passed in by ES:SI
*/
puts:
    movb $0x0e, %ah
    xorb %bh, %bh
puts_loop:
    lodsb
    cmp $0, %al
    je puts_break
    int $0x10
    jmp puts_loop
puts_break:
    ret

/*
  Prints in hex format the number in %al
*/
put_hex:
    movb %al, %ch
    movb $'0', %al
    call putc
    movb $'x', %al
    call putc

    movb %ch, %al
    shrb $4, %al
    cmpb $10, %al
    jae 1f
    addb $'0', %al
    jmp 2f
1:
    addb $'7', %al
2:
    call putc
    movb %ch, %al
    andb $0xf, %al
    cmpb $10, %al
    jae 1f
    addb $'0', %al
    jmp 2f
1:
    addb $'7', %al
2:
    call putc
    ret

/* === DATA SECTION === */
.section .data

boot_drive:
    .byte 0

stage0_str:
    .asciz "Loading stage1...\r\n"
no_ext_str:
    .asciz "This BIOS does not support int 13h extensions. Cannot continue. "
disk_str:
    .asciz "Disk "
err_str:
    .asciz "error: "
nothing_str:
    .asciz "Nothing to do.\r\n"

dap:
dap_size:
    .byte 0x10  /* must always be 0x10 */
dap_rsvd:
    .byte 0
dap_nsects:
    .word 0
dap_off:
    .word STAGE1_OFFSET     /* initialized */
dap_seg:
    .word STAGE1_SEGMENT    /* constant */
dap_lba:
    .quad 1     /* initialized to sector 1 */
    .quad 0

/* === MAGIC === */
/* this will be moved to the end by the linker */
.section .magic, "a"
.word 0xaa55

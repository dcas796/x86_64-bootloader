ENTRY(_start)
/* OUTPUT_FORMAT("binary") */

SECTIONS {
    . = 0x7c00;

    .bpb : {
        *(.bpb*)
    }

    . = 0x7c00 + 90;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }

    .data : {
        *(.data*)
    }

    stack_bottom = .;
    . = 0x7c00 + 510;
    stack_top = .;

    .magic : {
        *(.magic*)
    }
}

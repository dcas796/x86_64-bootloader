ENTRY(entry)

SECTIONS {
    . = 0x100000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .bss : {
        *(.bss*)
    }
   
    .data : {
        *(.data*)
    }
}

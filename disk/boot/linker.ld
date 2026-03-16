ENTRY(entry)

SECTIONS {
    . = 0x400000;

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

ENTRY(entry)

SECTIONS {
    . = 0xc000;

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

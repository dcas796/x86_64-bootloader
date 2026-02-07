#include <types.h>
#include "print.h"
#include "string.h"

extern uint8_t boot_drive;

void main() {
    puts("stage2\n\r");
    char boot_drive_str[3];
    puts("Boot drive: 0x");
    itoa((int)boot_drive, boot_drive_str, 16);
    puts(boot_drive_str);

}

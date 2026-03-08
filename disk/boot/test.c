// x86_64-elf-gcc -m16 -Wall -Wextra -nostdlib -nostdinc -ffreestanding -c test.c -o test.o && x86_64-elf-ld -m elf_i386 -Tlinker.ld test.o -o kernel.elf

void entry(void (*puts)(const char *)) {
    puts("i am born and i shall die\r\n");
}

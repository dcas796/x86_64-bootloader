# x86_64-bootloader

A simple bootloader for x86_64 kernels.

This bootloader loads a raw executable binary from a FAT formatted disk into memory and executes it. 
`stage0` is stored in the boot sector (sector 0) and `stage1` is stored in the first reserved sectors of the FAT file 
system.

## Prerequisites

You will need:
- The Make build system
- The `x86_64-elf` toolchain & the `x86_64-elf-gcc` compiler
- The Python 3 interpreter
- The `mkfs.fat` & `mcopy` commands (from the `dosfstools` & `mtools` packages, respectively)

## Usage

The `make` command will write a FAT formatted disk image to `./build/boot.img` with the contents of the folder 
`./disk/`. The bootloader has a config file, `/boot/options.txt`, where boot parameters can be specified.

### `options.txt` format

The file consists of a list of key-value pairs, in the form of `key=value`, separated by newlines. The possible config
parameters are:

- `boot_binary=[filepath]`, where `filepath` is the absolute path of ELF executable to load and execute. No long filename
support, yet, so keep in mind that long filenames will be truncated into 8.3 format.

**Notes:** 
- Due to memory limitations, this file needs to be less than or equal to 1KiB (1024 bytes) in size.
- The ELF cannot load segments to memory below 1MiB, as there are important BIOS and bootloader code and data.

## Debugging

### Prerequisistes

To debug this bootloader you will need additional tools:
- QEMU
- `x86_64-elf-gdb` or `i386-elf-gdb`

### Debugging with GDB

When building `boot.img`, there are several generated intermediate artifacts:
- `stage0/build/stage0.elf`
- `stage1/build/stage1.elf`

These ELFs have all the debugging information that GDB needs.

To start the debugging process, first run one of the two commands:
```shell
make clean && make qemu-debug
```
or 
```shell
make clean && make qemu-debug-32
```
depending on whether you are using `x86_64-elf-gdb` or `i386-elf-gdb` (the latter one is recommended over the other)

Then, start GDB with the following arguments:
```shell
i386-elf-gdb -ex "sym $STAGE/build/$STAGE.elf" \
    -ex "target remote localhost:9000" \
    -ex "set architecture i8086" \
    -ex "b _start" \
    -ex "c" 
```
where `$STAGE` is either `stage0` or `stage1` .

**Notes:**
- It is recommended to use `i386-elf-gdb` as it decodes function arguments and variables better.
- When using `x86_64-elf-gdb` instead of `i386-elf-gdb`, remove the line `-ex "set architecture i8086"`.
- Both GDBs produce trash disassembly, so don't trust it.

### Debugging with CLion

To start debugging with CLion, you will first need to start QEMU like with GDB:
```shell
make clean && make qemu-debug
```
or
```shell
make clean && make qemu-debug-32
```

Then, using the provided debug configurations run either `Debug QEMU` or `Debug QEMU (32 bit)`, depending on which 
command you have run.

**Notes:**
- Again, it is recommended to use the 32 bit version for the same reasons as before.
- Out of the box, it will start debugging `stage1`. Change the symbol file to debug for `stage0`.
- The provided debug configurations assume that you have installed GDB into `/opt/homebrew/bin/...`. Change this if 
your GDB is somewhere else.
- Same as before, don't trust the disassembled functions.


## License

See [LICENSE](LICENSE)

---

Made by [dcas796](https://dcas796.github.com/)

# x86_64-bootloader

A simple bootloader for x86_64 kernels.

This bootloader loads an ELF binary from a FAT formatted disk into memory and executes it. 
`stage0` is stored in the boot sector (sector 0) and `stage1` is stored in the first reserved sectors of the FAT file 
system.

## Prerequisites

You will need:
- The Make build system
- The `x86_64-elf` toolchain & the `x86_64-elf-gcc` compiler
- The Python 3 interpreter
- The `mkfs.fat` & `mcopy` commands (from the `dosfstools` & `mtools` packages, respectively)

## Usage

The `make` command will write a FAT formatted bootable disk image to `./build/boot.img` with the contents of the folder 
`./disk/`. The bootloader has a config file, `/boot/options.txt`, where boot parameters can be specified.

### Makefile options
- `make clean` - removes all build-related files, including the disk image.
- `make build` - builds the bootloader and the disk image.
- `make qemu` - runs the generated disk image in QEMU.
- `make qemu-debug` - runs the generated disk image in QEMU in debugging mode.

### Makefile variables
- `RELEASE` - if set to `1`, the bootloader will be built with compiler optimizations.

### `options.txt` format

The file consists of a list of key-value pairs, in the form of `key=value`, separated by newlines. The possible config
parameters are:

- `boot_binary=[filepath]`, where `filepath` is the absolute path of ELF executable to load and execute. No long filename
support, yet, so keep in mind that long filenames will be truncated into 8.3 format.

**Notes:** 
- Neither dynamically linked nor relocatable executables are supported.
- The ELF cannot load segments to memory below 1MiB, as there are important BIOS and bootloader code and data.

### `boot_binary` entry point

The bootloader expects the ELF entry point to have the following signature:
```c++
void __attribute__((noreturn)) entry(const sysinfo_t *info);
```
where `sysinfo_t` is a struct that contains information about the system. Its header file is located in 
`include/sysinfo.h`.

## Debugging

### Prerequisites

To debug this bootloader you will need additional tools:
- QEMU
- `i386-elf-gdb`

### Debugging with GDB

When building `boot.img`, there are several generated intermediate artifacts:
- `stage0/build/stage0.elf`
- `stage1/build/stage1.elf`

These ELFs have all the debugging information that GDB needs.

To start the debugging process, first run the command:
```shell
make clean && make qemu-debug
```

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
- GDB produces trash 16-bit disassembly, so don't trust it.

### Debugging with CLion

To start debugging with CLion, you will first need to start QEMU like with GDB:
```shell
make clean && make qemu-debug
```

Then, using the provided debug configurations, run `Debug QEMU`.

**Notes:**
- Out of the box, it will start debugging `stage1`. Change the symbol file to debug for `stage0`.
- The provided debug configurations assume that you have installed GDB into `/opt/homebrew/bin/...`. Change this if 
your GDB is somewhere else.
- Same as before, don't trust the disassembled functions.


## License

See [LICENSE](LICENSE)

---

Made by [dcas796](https://dcas796.github.com/)

# x86_64-bootloader

A simple bootloader for the x86_64 architecture.

This bootloader loads a raw executable binary from a FAT formatted disk into memory and executes it. `stage0` is stored in the boot sector (sector 0) and `stage1` is stored in the first reserved sectors of the FAT file system.

## Prerequisites

You will need:
- The Make build system
- The x86_64-elf toolchain & the x86_64-elf-gcc compiler
- The Python 3 interpreter
- The mkfs.fat & mcopy commands (from the dosfstools & mtools packages, respectively)

## Usage

The `make` command will write a FAT formatted disk image to `./build/boot.img` with the contents of the folder `./disk/`. The bootloader has a config file, `/boot/options.txt`, where boot parameters can be specified.

### `options.txt` format

The file consists of a list of key-value pairs, in the form of `key=value`, separated by newlines. The possible config parameters are:

- `boot_binary=[filepath]`, where `filepath` is the absolute path of the binary to load and execute.
- `load_offset=[offset]`, where `offset` is the memory address where the `boot_binary` will be loaded into.

## Boot process

### Stage 0

When the computer boots up, the BIOS load the first sector of disk to `0x7c00`. It will then jump to that address, where the BIOS has just loaded `stage0`. As `stage0` must fit in less than 512 bytes, it can do very little. This means that it will read the second sector of disk, where the information about `stage1` can be found, such as the start and end sector at which it can be found, and where in memory to load it. After loading `stage1`, it will jump to it and start executing it.

### Stage 1

In `stage1`, it will load `boot_binary` to `load_offset` using BIOS boot services. Then, it will load a temporary GPT and IDT and clear interrupts. Finally, it will jump to `load_offset`.

## License

MIT License

Copyright (c) 2025 Daniel O. (dcas796)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

Made by [dcas796](https://dcas796.github.com/)

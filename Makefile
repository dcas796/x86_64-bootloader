export CC := x86_64-elf-gcc
export AS := x86_64-elf-as
export LD := x86_64-elf-ld
export OBJCOPY := x86_64-elf-objcopy
export PYTHON := python3

BUILD_DIR := ./build
SCRIPTS_DIR := ./scripts
STAGE0_DIR := ./stage0
STAGE0_BIN := $(BUILD_DIR)/stage0.bin
STAGE1_DIR := ./stage1
STAGE1_BIN := $(BUILD_DIR)/stage1.bin

BOOT_IMG := $(BUILD_DIR)/boot.img

.DEFAULT_GOAL := build
.PHONY: all build build-dir clean qemu qemu-debug qemu-debug-32

all: build

build: build-dir $(BOOT_IMG)

build-dir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(STAGE0_DIR) clean
	$(MAKE) -C $(STAGE1_DIR) clean

qemu: build
	qemu-system-x86_64 -drive format=raw,file=$(BOOT_IMG)

qemu-debug: build
	qemu-system-x86_64 -drive format=raw,file=$(BOOT_IMG) -s -S -monitor stdio

qemu-debug-32: build
	qemu-system-i386 -drive format=raw,file=$(BOOT_IMG) -gdb tcp::9000 -S -monitor stdio

$(BOOT_IMG): $(STAGE0_BIN) $(STAGE1_BIN)
	$(PYTHON) $(SCRIPTS_DIR)/create_boot_img.py $@ $^

$(STAGE0_BIN): $(STAGE0_DIR) build-dir
	$(MAKE) -C $<
	cp $</build/stage0.bin $@

$(STAGE1_BIN): $(STAGE1_DIR) build-dir
	$(MAKE) -C $<
	cp $</build/stage1.bin $@

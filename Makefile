export CC := x86_64-elf-gcc
export AS := x86_64-elf-as
export LD := x86_64-elf-ld
export OBJCOPY := x86_64-elf-objcopy

BUILD_DIR := ./build
STAGE0_DIR := ./stage0
STAGE0_BIN := $(BUILD_DIR)/stage0.bin

.DEFAULT_GOAL := build
.PHONY: build build-dir clean qemu

build: build-dir $(STAGE0_BIN) # build/stage1.bin

build-dir:
	mkdir -p $(BUILD_DIR)

$(STAGE0_BIN):
	$(MAKE) -C $(STAGE0_DIR)
	cp $(STAGE0_DIR)/build/stage0.bin $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(STAGE0_DIR) clean
#	$(MAKE) -C $(STAGE1_DIR) clean

qemu: build
	qemu-system-x86_64 -drive format=raw,file=$(STAGE0_BIN)

qemu-debug: build
	qemu-system-x86_64 -drive format=raw,file=$(STAGE0_BIN) -s -S -monitor stdio

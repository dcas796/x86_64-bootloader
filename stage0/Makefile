INCLUDE_DIR := ../include
BUILD_DIR := ./build
LINKER_SCRIPT := $(BUILD_DIR)/linker.ld
LINKER_SCRIPT_IN := ./linker.ld.in
DEBUG_ELF := $(BUILD_DIR)/stage0.elf
RESULT := $(BUILD_DIR)/stage0.bin

CFLAGS := -g -c -m16 -std=c23 -ffreestanding -fno-pic -fno-stack-protector -mgeneral-regs-only -nostdlib -Wall -Wextra
CFLAGS += -I$(INCLUDE_DIR)
ASFLAGS := -g -m32 -c -I$(INCLUDE_DIR)
LDFLAGS := -g -m elf_i386 -T$(LINKER_SCRIPT)

C_SRCS := $(wildcard *.c)
S_SRCS := $(wildcard *.S)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS)) $(patsubst %.S, $(BUILD_DIR)/%.o, $(S_SRCS))

.DEFAULT_GOAL := $(RESULT)
.PHONY: clean

$(RESULT): $(DEBUG_ELF)
	$(OBJCOPY) -O binary $< $@

$(DEBUG_ELF): $(BUILD_DIR) $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.S
	$(CC) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(LINKER_SCRIPT): $(LINKER_SCRIPT_IN)
	$(CC) -E -P -I$(INCLUDE_DIR) -x c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

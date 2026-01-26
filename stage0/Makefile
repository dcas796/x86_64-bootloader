INCLUDE_DIR := ../include
BUILD_DIR := ./build
LINKER_SCRIPT := ./linker.ld
DEBUG_ELF := $(BUILD_DIR)/stage0.elf
RESULT := $(BUILD_DIR)/stage0.bin

CFLAGS := -g -ffreestanding -fno-pic -fno-stack-protector -nostdlib -Wall -Wextra -I$(INCLUDE_DIR)
ASFLAGS := -g -c -I$(INCLUDE_DIR)
LDFLAGS := -g -T$(LINKER_SCRIPT)

C_SRCS := $(wildcard *.c)
S_SRCS := $(wildcard *.S)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS)) $(patsubst %.S, $(BUILD_DIR)/%.o, $(S_SRCS))

.DEFAULT_GOAL := $(RESULT)
.PHONY: clean

$(RESULT): $(DEBUG_ELF)
	$(OBJCOPY) -O binary $< $@

$(DEBUG_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(BUILD_DIR)
	$(CC) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

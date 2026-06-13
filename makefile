# ==============================================================================
# 1. OUTPUT AND TOOLCHAIN CONFIGURATION
# ==============================================================================
# Nuke built-in implicit rules to speed up make and prevent weird behavior
.SUFFIXES:

# The name of your final kernel binary and output ISO image
override OUTPUT := kernel.elf
ISO_IMAGE := unknownOS.iso

# Default toolchain definitions (Cross-compiler for bare-metal x86_64)
TOOLCHAIN        :=
TOOLCHAIN_PREFIX :=

ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

# If toolchain is not specified, default to your x86_64-elf cross-compiler
ifeq ($(TOOLCHAIN_PREFIX),)
    CC := x86_64-elf-gcc
    LD := x86_64-elf-ld
else
    CC := $(TOOLCHAIN_PREFIX)gcc
    LD := $(TOOLCHAIN_PREFIX)ld
endif

# LLVM/Clang auto-detection and setup if requested
ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    LD := ld.lld
endif

# Check if the compiler is Clang to apply correct bare-metal target flags
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)
ifeq ($(CC_IS_CLANG),1)
    override CC += -target x86_64-unknown-none-elf
endif

# ==============================================================================
# 2. COMPILER AND LINKER FLAGS (PROTECTED WITH OVERRIDE)
# ==============================================================================
# User-controllable optimization and debug flags
CFLAGS := -g -O2 -pipe
NASMFLAGS := -g

# Critical Kernel CFLAGS that must NEVER be overwritten by the user
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel

# Preprocessor flags including your src/include folder for clean header resolution
override CPPFLAGS := \
    -I src/include \
    -I src \
    $(CPPFLAGS) \
    -MMD \
    -MP

# Critical NASM flags for 64-bit ELF generation and debugging symbols
override NASMFLAGS += \
    -f elf64 \
    $(patsubst -g,-g -F dwarf,$(NASMFLAGS)) \
    -Wall

# Critical Linker flags to enforce static freestanding layout using your linker script
override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker.lds

# ==============================================================================
# 3. AUTOMATIC FILE DETECTION (GLOBBING)
# ==============================================================================
# Automatically find all source files in the src/ tree
override SRCFILES  := $(shell find -L src -type f 2>/dev/null | LC_ALL=C sort)
override CFILES    := $(filter %.c,$(SRCFILES))
override ASFILES   := $(filter %.S,$(SRCFILES))
override NASMFILES := $(filter %.asm,$(SRCFILES))

# Map discovered source files to their corresponding object files inside obj/
override OBJ         := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Declare fake targets
.PHONY: all clean run

# ==============================================================================
# 4. BUILD RULES (RECIPES)
# ==============================================================================
# Default target points to the final ISO image generation
all: $(ISO_IMAGE)

# Safely include generated header dependencies (.d files) tracking modifications
-include $(HEADER_DEPS)

# Recipe to link the final kernel ELF binary
$(OUTPUT): linker.lds $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

# Pattern rule to compile standard C files
obj/%.c.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Pattern rule to compile GNU Assembly files (.S)
obj/%.S.o: %.S
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Pattern rule to compile Intel/NASM Assembly files (.asm)
obj/%.asm.o: %.asm
	@mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) $< -o $@

# ==============================================================================
# 5. ISO CREATION AND BOOTLOADER STAGING
# ==============================================================================
# Build the final bootable ISO image containing Limine and your kernel
$(ISO_IMAGE): $(OUTPUT) limine.conf
	# Ensure the native limine host tool is compiled on Fedora
	make -C limine
	
	# Construct the official ISO 9660 and UEFI compliant directory layout
	mkdir -p iso_root/boot
	mkdir -p iso_root/boot/limine
	mkdir -p iso_root/EFI/BOOT
	
	# Stage your system files
	cp -v $(OUTPUT) iso_root/boot/
	cp -v limine.conf iso_root/boot/limine/
	
	# Stage the bootloader binaries fetched from the submodule
	cp -v limine/limine-bios.sys iso_root/boot/limine/
	cp -v limine/limine-bios-cd.bin iso_root/boot/limine/
	cp -v limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	
	# Master the hybrid bootable image via xorriso
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
            -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
            -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
            -efi-boot-part --efi-boot-image --protective-msdos-label \
            iso_root -o $(ISO_IMAGE)
	
	# Patch the MBR of the ISO file to enable legacy BIOS booting from USB drives
	./limine/limine bios-install $(ISO_IMAGE)

# ==============================================================================
# 6. UTILITY TARGETS
# ==============================================================================
# Launch the automated build pipeline and fire up the QEMU emulator
run: $(ISO_IMAGE)
	qemu-system-x86_64 -M q35 -m 256M -cdrom $(ISO_IMAGE) -boot d -serial stdio

# Wipe out all generated build artifacts to reset the workspace environment
clean:
	rm -rf obj $(OUTPUT) $(ISO_IMAGE) iso_root
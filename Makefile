override KERNEL := Qnixx.elf
 
# Convenience macro to reliably declare overridable command variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef
 
TARGET := linux-gnu
CC := aarch64-$(TARGET)-gcc
LD := aarch64-$(TARGET)-ld


# User controllable CFLAGS.
CFLAGS ?= -g -O2 -pipe -Wall -Wextra
 
# User controllable preprocessor flags. We set none by default.
CPPFLAGS ?=
 
# User controllable nasm flags.
NASMFLAGS ?= -F dwarf -g
 
# User controllable linker flags. We set none by default.
LDFLAGS ?=
 
# Internal C flags that should not be changed by the user.
override CFLAGS +=       \
    -std=c11             \
    -ffreestanding       \
    -fno-stack-protector \
    -fno-stack-check     \
    -fno-lto             \
    -fno-pie             \
    -fno-pic             \
    -mgeneral-regs-only  \
    -MMD                 \
    -Isys/include/lib       \
    -Isys/include
 
# Internal linker flags that should not be changed by the user.
override LDFLAGS +=         \
    -nostdlib               \
    -static                 \
    -m aarch64elf           \
    -z max-page-size=0x1000 \
    -T sys/src/linker.ld
 
# Check if the linker supports -no-pie and enable it if it does.
ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
    override LDFLAGS += -no-pie
endif
 
# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -f elf64
 
# Use find to glob all *.c, *.S, and *.asm files in the directory and extract the object names.
override CFILES := $(shell find sys/src -type f -name '*.c')
override ASFILES := $(shell find sys/src -type f -name '*.S')
override NASMFILES := $(shell find sys/src -type f -name '*.asm')
override OBJ := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(ASFILES:.S=.d)
 
# Default target.
.PHONY: all
all: clean $(KERNEL) makeiso
	make clean
 
# Link rules for the final kernel executable.
$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
 
# Include header dependencies.
-include $(HEADER_DEPS)
 
# Compilation rules for *.c files.
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
 
# Compilation rules for *.S files.
%.o: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
 
# Compilation rules for *.asm (nasm) files.
%.o: %.asm
	nasm $(NASMFLAGS) $< -o $@
 
# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS) firmware/

makeiso:
	bash ./get-iso.sh


MACHINE := virt
ARGS	:= -cpu cortex-a72          \
            -m 512m                 \
            -bios firmware/OVMF.fd  \
            -monitor stdio          \
            -device ramfb           \
            -device qemu-xhci       \
            -device usb-kbd         \
            -d int                  \

run: clean-objs ovmf-firmware qemu

clean-objs:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS) limine/ bin/ obj/

ovmf-firmware:
	mkdir -p firmware
	cd firmware && curl -o OVMF-AA64.zip https://efi.akeo.ie/OVMF/OVMF-AA64.zip && unzip OVMF-AA64.zip
	rm firmware/readme.txt &&	rm firmware/OVMF-AA64.zip

qemu:
	qemu-system-aarch64 -M $(MACHINE) $(ARGS) -cdrom Qnixx.iso

LOCAL_IP = 192.168.1.152
INSTALL_DIR = /qnixx_install
QEMU_ARGS = -cpu qemu64 -M q35 -m 3G -cdrom Qnixx.iso -boot d -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 
CFILES = $(shell find sys/src/ -name "*.c")
CFLAGS = -fexceptions -std=gnu11 -ffreestanding -fno-stack-protector \
  -fno-pic -Werror=implicit -Werror=implicit-function-declaration -Werror=implicit-int \
  -Werror=int-conversion \
  -Werror=incompatible-pointer-types -Werror=int-to-pointer-cast -Werror=return-type -Wunused \
  -mabi=sysv -mno-80387 -mno-mmx \
  -mno-3dnow -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel

CC = cross/bin/x86_64-elf-gcc
LD = cross/bin/x86_64-elf-ld


.PHONY: all
all: cfiles asmfiles link limine cleanup
	@#cd usr; make
	@echo "Generating ISO..."
	@mkdir -p iso_root
	@mkdir -p iso_root/Qnixx
	@cp etc/limine.cfg \
		limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	@cp sys/kernel.sys meta/system/*.sys iso_root/Qnixx/
	@xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o Qnixx.iso
	@echo "Deploying Limine..."
	@limine/limine-deploy Qnixx.iso
	@rm -rf iso_root


.PHONY: link
link:
	@echo "Linking object files..."
	@#mv $(shell find sys/src/ -name "*.o") ./
	@$(LD) *.o -nostdlib -zmax-page-size=0x1000 -static -Tsys/link.ld -o sys/kernel.sys

.PHONY: cfiles
cfiles: $(CFILES)
	@echo "Compiling C sources..."
	@$(CC) -march=x86-64 $(CFLAGS) -c $^ -Isys/include/ -D_KERNEL

asmfiles:
	@echo "Compiling assembly sources..."
	@for source in $$(find sys/src/ -name "*.asm"); do nasm -felf64 $$source; done

limine:
	@echo "Fetching Limine binaries..."
	@git clone https://github.com/limine-bootloader/limine.git --branch=v4.0-binary --depth=1
	@make -C limine

sbin/diskimg.img:
	@#echo "Creating filesystem image.."
	@#dd if=/dev/zero of=sbin/diskimg.img bs=1M count=512
	@#echo "Creating ext2 filesystem on disk image.."
	@#mke2fs sbin/diskimg.img

.PHONY: debug
debug: sbin/diskimg.img
	@qemu-system-x86_64 --enable-kvm $(QEMU_ARGS) -monitor stdio


.PHONY: debug
debug1: sbin/diskimg.img
	@qemu-system-x86_64 $(QEMU_ARGS) -monitor stdio -d int

.PHONY:
run: sbin/diskimg.img
	@echo "Creating tap..."
	@sudo ip tuntap add dev tap0 mode tap user $(shell id -u)
	@sudo ip address add $(LOCAL_IP) dev tap0
	@sudo ip link set dev tap0 up
	@echo "Running..."
	@qemu-system-x86_64 --enable-kvm $(QEMU_ARGS) -serial stdio -netdev tap,id=br0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=br0,mac=52:55:00:d1:55:01
	@echo "Removing tap..."
	@sudo ip tuntap del dev tap0 mode tap

.PHONY: toolchain
toolchain:
	bash tools/cross.sh

.PHONY:
cleanup:
	@echo "Cleaning up..."
	@rm *.o

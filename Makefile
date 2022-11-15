LOCAL_IP = 192.168.1.152
INSTALL_DIR = /media/kai/6402-F321
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
all: romfs TauLang cfiles asmfiles taufiles link limine cleanup
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


install_limine:
	@echo "Installing Limine..."
	@mkdir -p $(INSTALL_DIR)/EFI/BOOT
	@cp limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin $(INSTALL_DIR)/
	@cp limine/BOOTX64.EFI $(INSTALL_DIR)/EFI/BOOT/BOOTX64.EFI

install_qnixx:
	@echo "Installing Qnixx..."
	@mkdir -p $(INSTALL_DIR)/Qnixx
	@cp etc/limine.cfg $(INSTALL_DIR)/limine.cfg
	@cp sys/kernel.sys meta/system/* $(INSTALL_DIR)/Qnixx/

.PHONY: link
link:
	@echo "Linking object files..."
	@mv $(shell find sys/src/ -name "*.o") ./
	@$(LD) *.o -nostdlib -zmax-page-size=0x1000 -static -Tsys/link.ld -o sys/kernel.sys

.PHONY: cfiles
cfiles: $(CFILES)
	@echo "Compiling C sources..."
	@$(CC) -march=x86-64 $(CFLAGS) -c $^ -Isys/include/ -D_KERNEL

asmfiles:
	@echo "Compiling assembly sources..."
	@for source in $$(find sys/src/ -name "*.asm"); do nasm -felf64 $$source; done

.PHONY: taufiles
taufiles:
	@echo "Compiling TauLang sources..."
	@for source in $$(find sys/src/ -name "*.tau"); do	tau -i $$source -c -o "$$source.o"; done

limine:
	@echo "Fetching Limine binaries..."
	@git clone https://github.com/limine-bootloader/limine.git --branch=v4.0-binary --depth=1
	@make -C limine

TauLang: 
	@echo "Building and installing TauLang compiler..."
	@git clone https://github.com/Ian-Marco-Moffett/TauLang
	@cd TauLang; make; make install_linux

.PHONY:
romfs:
	mv meta/system/raw/ ./; cd raw; tar -cvf ../meta/system/romfs.sys *; cd ../; mv raw meta/system/

.PHONY: debug_kvm
debug_kvm:
	qemu-system-x86_64 --enable-kvm -cpu qemu64 -M q35 -m 3G -drive file=Qnixx.iso,format=raw -boot d -monitor stdio -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -device rtl8139

.PHONY:
run:
	@echo "Creating tap..."
	@sudo ip tuntap add dev tap0 mode tap user $(shell id -u)
	@sudo ip address add $(LOCAL_IP) dev tap0
	@sudo ip link set dev tap0 up
	@echo "Running..."
	@qemu-system-x86_64 --enable-kvm -cpu qemu64 -M q35 -m 3G -drive file=Qnixx.iso,format=raw -boot d -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -serial stdio -netdev tap,id=br0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=br0,mac=52:55:00:d1:55:01
	@echo "Removing tap..."
	@sudo ip tuntap del dev tap0 mode tap

.PHONY: toolchain
toolchain:
	bash tools/cross.sh

.PHONY:
cleanup:
	@echo "Cleaning up..."
	@rm *.o

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
all: TauLang cfiles taufiles link limine cleanup
	mkdir -p iso_root
	mkdir -p iso_root/Qnixx
	cp etc/limine.cfg \
		limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	cp sys/kernel.sys meta/internals/* iso_root/Qnixx/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o Qnixx.iso
	limine/limine-deploy Qnixx.iso
	rm -rf iso_root


.PHONY: link
link:
	mv $(shell find sys/src/ -name "*.o") ./
	$(LD) *.o -nostdlib -zmax-page-size=0x1000 -static -Tsys/link.ld -o sys/kernel.sys

.PHONY: cfiles
cfiles: $(CFILES)
	$(CC) -march=x86-64 $(CFLAGS) -c $^ -Isys/include/

.PHONY: taufiles
taufiles:
	for source in $$(find sys/src/ -name "*.tau"); do	tau -i $$source -c -o "$$source.o"; done

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v4.0-binary --depth=1
	make -C limine

TauLang: 
	git clone https://github.com/Ian-Marco-Moffett/TauLang
	cd TauLang; make; make install_linux

.PHONY: debug_kvm
debug_kvm:
	qemu-system-x86_64 --enable-kvm -cpu qemu64 -M q35 -m 3G -drive file=Qnixx.iso,format=raw -boot d -monitor stdio -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -device rtl8139

.PHONY:
run:
	bash tools/init_tap.sh
	qemu-system-x86_64 --enable-kvm -cpu qemu64 -M q35 -m 3G -drive file=Qnixx.iso,format=raw -boot d -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -serial stdio -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=mynet0,mac=52:55:00:d1:55:01 -net user

.PHONY: toolchain
toolchain:
	bash tools/cross.sh

.PHONY:
cleanup:
	rm *.o

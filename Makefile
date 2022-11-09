CFILES = $(shell find sys/src/ -name "*.c")


.PHONY:
all: sys/kernel.sys cfiles link limine cleanup
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
	ld *.o -nostdlib -zmax-page-size=0x1000 -static -Tsys/link.ld -o sys/kernel.sys

.PHONY: cfiles
cfiles: $(CFILES)
	gcc -c $^

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v4.0-binary --depth=1
	make -C limine

.PHONY:
run:
	qemu-system-x86_64 --enable-kvm -cpu qemu64 -M q35 -m 3G -drive file=Qnixx.iso -boot d -monitor stdio -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0

.PHONY:
cleanup:
	rm *.o

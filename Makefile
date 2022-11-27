CFLAGS = -Ignu-efi/inc -Isrc/ -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c
LDFLAGS = $ -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi -Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o
CC = gcc
CFILES = $(shell find src/ -name "*.c")


.PHONY: all
all: gnu-efi cfiles
	ld $(LDFLAGS) *.o -o main.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 main.so BOOTX64.EFI
	rm *.o *.so
	mkdir -p mnt

.PHONY: test
run:	
	dd if=/dev/zero of=Qnixx.img bs=512 count=93750
	mformat -i Qnixx.img
	mmd -i Qnixx.img ::/EFI
	mmd -i Qnixx.img ::/EFI/BOOT
	mcopy -i Qnixx.img BOOTX64.EFI ::/EFI/BOOT
	mcopy -i Qnixx.img startup.nsh ::
	qemu-system-x86_64 -drive file=Qnixx.img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="OVMFbin/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="OVMFbin/OVMF_VARS-pure-efi.fd" -net none -monitor stdio -d int -no-reboot -D logfile.txt -M smm=off

.PHONY: files
cfiles: $(CFILES)
	$(CC) $(CFLAGS) $^

gnu-efi:
	git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
	cd gnu-efi; make

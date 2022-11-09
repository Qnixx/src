BOOT_CFLAGS = -Iboot/uefi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args
BOOT_LDFLAGS = -shared -Bsymbolic -Lboot/uefi/x86_64/lib -Lboot/uefi/x86_64/gnuefi -Tboot/uefi/gnuefi/elf_x86_64_efi.lds boot/uefi/x86_64/gnuefi/crt0-efi-x86_64.o
BOOT_CFILES = $(shell find boot/src/ -name "*.c")

.PHONY: all
all: bootloader clean

.PHONY: bootloader
bootloader: $(BOOT_CFILES)
	cd boot/uefi; make
	gcc $(BOOT_CFLAGS) -c $^
	ld $(BOOT_LDFLAGS) *.o -o main.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 main.so BOOTX64.EFI
	sudo dd if=/dev/zero of=Qnixx.img bs=1k count=1440
	sudo mformat -i Qnixx.img -f 1440 ::
	sudo mmd -i Qnixx.img ::/EFI
	sudo mmd -i Qnixx.img ::/EFI/BOOT
	sudo mcopy -i Qnixx.img BOOTX64.EFI ::EFI/BOOT
	sudo mcopy -i Qnixx.img meta/startup.nsh ::

.PHONY: run
run:
	sudo qemu-system-x86_64 -drive file=Qnixx.img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="sbin/OVMFbin/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="sbin/OVMFbin/OVMF_VARS-pure-efi.fd" -net none -monitor stdio -smp 2

.PHONY: clean
clean:
	rm *.o *.so *.EFI

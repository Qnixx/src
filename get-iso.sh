#!/bin/sh
git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
 
# Build limine-deploy.
make -C limine
 
# Create a directory which will be our ISO root.
mkdir -p iso_root
 
# Copy the relevant files over.
cp -v myos.elf etc/limine.cfg limine/limine.sys \
      limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
 
# Create the bootable ISO.
xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-cd-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o Qnixx.iso
 
# Install Limine stage 1 and 2 for legacy BIOS boot.
./limine/limine-deploy Qnixx.iso

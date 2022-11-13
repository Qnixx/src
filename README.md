# Qnixx
### A new Unix standard!

## Prebuilt ISOs
Go to [the most recent job](https://github.com/Qnixx/src/actions) and Qnixx.iso will be located under Artifacts.

## Bare-Metal Installation
Before installing, make sure you have a build of Qnixx and the Limine binaries. Once you have the materials ready, mount your partition and modify the Makefile's `INSTALL_DIR` variable to its path.
### Installing limine
Qnixx is booted by the Limine bootloader. If you already have Limine installed, skip this step.
To install Limine, you need an empty FAT32 partition of at least 4MB. If you have another bootloader installed, you will also need a BIOS that allows you to select a partition to boot from. Run `make install_limine` to instal limine to `INSTALL_DIR`.
### Installing Qnixx
Once you have Limine installed, run `make install_qnixx` to install the kernel binaries and limine configuration to `INSTALL_DIR`

## Networking
Edit the Makefile and change the IP at ``LOCAL_IP`` to
your local ip which you can find with ``ifconfig``

## Etc
Come hang out with us on [our discord!](https://discord.gg/2rtRsbm4Am)

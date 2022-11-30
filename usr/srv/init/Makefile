CFLAGS = -fno-stack-protector \
	-fno-pic -Werror=implicit -Werror=implicit-function-declaration -Werror=implicit-int \
	-Werror=int-conversion -Wno-builtin-declaration-mismatch -no-pie 										 \
	-Werror=incompatible-pointer-types -Werror=int-to-pointer-cast -I ../../lib/libc/include/

CFILES = $(shell find src/ -name "*.c")

.PHONY: all
all: $(CFILES)
	gcc -c $(CFLAGS) $^
	ld -T ../../link.ld *.o -o ../../../meta/initrd/init.sys -L../../lib/libc/ -l:libqc.a

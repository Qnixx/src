CFLAGS = -fno-stack-protector \
	-fno-pic -Werror=implicit -Werror=implicit-function-declaration -Werror=implicit-int \
	-Werror=int-conversion -Wno-builtin-declaration-mismatch -no-pie 										 \
	-Werror=incompatible-pointer-types -Werror=int-to-pointer-cast

CFILES = $(shell find src/ -name "*.c")

.PHONY: all
all: $(CFILES)
	gcc -c $(CFLAGS) $^
	ld -T ../../link.ld *.o -o ../../../meta/system/init.sys

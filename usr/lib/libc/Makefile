CFILES = $(shell find . -name "*.c")
ASMFILES = $(shell find . -name "*.S")


libqc.a: cfiles
	ar -rc $@ *.o
	rm *.o

.PHONY: cfiles
cfiles: $(CFILES) $(ASMFILES)
	gcc -c $^ -I include/ -Wno-builtin-declaration-mismatch

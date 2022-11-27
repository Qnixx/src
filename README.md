![Banner](res/logo_trans.png)

---

<h1 align="center">Zenith</h1>
A small, experimental AArch kernel.

<h2 align="center">Compilation</h2>

Compilation requirements:
- Any target of AArch64 binutils. (elf, gnu-eabi, none, etc.)

To compile using `aarch64-elf-*`, you can easily do `make`. If
you would like to change this, run `make` with `TARGET` equal
to whatever target you'd like.

For example, to compile Zenith for `aarch-none-eabi`, you can do
`make TARGET=none-eabi`.

### Running
Running requires a few dependecies. They are:
- Aarch64 QEMU
- Curl
- XORRISO
- Unzip
- git

Running is simple, just `make run`. However, to debug, you need
`make debug`.

<h1 align="center"> Goals / Roadmap </h1>

## Ground Zero
- [X] Proper UART driver
- [ ] A little MM
	- [X] `free()`
	- [X] `malloc()`
	- [X] `calloc()`
	- [ ] `realloc()`
- [ ] Some form of interrupt handling

:Author:
  Ian Marco Moffett

:Last updated: 2022-11-19

=========================
Welcome to the codebase!
=========================

The ``sys/`` directory.
~~~~~~~~~~~~~~~~~~~~~~~~~
This is where the kernel code goes.

``sys/src/arch/`` -> *architecture related code.*

``sys/src/arch/bus/`` -> *code releated to bus's like PCI and USB*

``sys/src/arch/cpu/`` -> *code related to the CPU (subject to change)*

``sys/src/arch/memory/`` -> *memory related code*

``sys/src/arch/x86/`` -> *x86 related code*

``sys/src/arch/x64/`` -> *x86_64 related code*

``sys/src/block`` -> *code related to block devices like a hard disk*

``sys/src/drivers/`` -> *qnixx drivers*

``sys/src/fs/`` -> *filesystem related code*

``sys/src/intr/`` -> *interrupt related code (see https://en.wikipedia.org/wiki/Interrupt)*

``sys/src/lib/`` -> *kernel libraries that contain helper functions*

``sys/src/mm/`` -> *memory management related code*

``sys/src/net/`` -> *networking related code*

``sys/src/proc/`` -> *process management related code*

``sys/src/tty`` -> *code for the TTY/console*



=====================
Terminology
=====================

Peripheral Component Interconnect (PCI)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A computer bus for attaching hardware devices
like network controllers or hard drive controllers.


=========================
Qnixx coding conventions
=========================

Usage of the ``static`` keyword.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Reduce linker namespace pollution

- Hint to the optimizer (which may decide to inline a large function if it is static and called just once)

- Can be detected as dead code when it is no longer used

- Intention of locality is documented.

Usage of ``__`` prefix.
~~~~~~~~~~~~~~~~~~~~~~~~
Do not use this everywhere as it makes things messy.

Only use this if you are naming a function
that should *only* be used by others
if they know what they are doing.

Brace style
~~~~~~~~~~~~
Curly braces like so:
::
  void function(void) {
    ..
  }

**Note the void keyword in the argument list.**


Indentation
~~~~~~~~~~~
Use spaces instead of tabs and have an 
indent width of 2.

For neovim users:
::
  set expandtab ts=2 sw=2 number nowrap

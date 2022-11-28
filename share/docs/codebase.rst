:Author:
  Ian Marco Moffett

:Last updated: 2022-11-28

=========================
Welcome to the codebase!
=========================

The ``sys/`` directory.
~~~~~~~~~~~~~~~~~~~~~~~~~
This is where the kernel code goes.

=========================
Qnixx coding conventions
=========================

Usage of the ``static`` keyword.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use static for global variables/functions that will not be used
outside the module.

For functions this will ensure they aren't used outside
the module if they are only helpers, the reason for this is if
they are only meant to be helpers and you use them outside that is
confusing and messy.


For variables this is used so we don't have too much
global and mutable variables all over the place.


This also helps increase link speed by 
not cluttering link tables with functions that will not be linked.

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

Note for when writing drivers for Qnixx.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please put your credit like so in a multiline comment.
::
  Description: What the driver is for.
  Author(s):

  Your Name <youremail@something.com>

Indentation
~~~~~~~~~~~
Use spaces instead of tabs and have an 
indent width of 2.

For neovim users:
::
  set expandtab ts=2 sw=2 number nowrap

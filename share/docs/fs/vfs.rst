:Author:
  Ian Marco Moffett

:Last updated: 2022-11-19

===============
Structures
===============

The Qnixx VFS module can be found in ``sys/include/fs/vfs.h``.


The ``vfs_superblock_t`` struct:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This struct holds information about the filesystem,
each filesystem will have it's own ``vfs_superblock_t``
struct.

``vfs_superblock_t`` fields:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
``size`` -> *Size of the filesystem in blocks.*

``blocksize`` -> *Blocksize for the filesystem.*


The ``fs_ops_t`` struct:
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Holds function pointers to file system
operations.


The ``fs_t`` struct:
~~~~~~~~~~~~~~~~~~~~~
The filesystem descriptor.


=================
Purpose
=================

"The purpose of a VFS is to allow client applications
to access different types of concrete file systems in a
uniform way."


=============
Interface
=============

The ``vfs_parse_path()`` function:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function parses a path a returns the array
of filenames (**note: this array must be freed with** ``kfree()``)

The ``vfs_mount()`` function.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function mounts a filesystem.
It takes *two* arguments:

``fs``  -> *filesystem descriptor*

``mountpoint`` -> *path to mount the filesystem to*


==============
How it works
==============

When a filesystem gets mounted it will lead
to ``vfs_mountfs()`` being called which does
the following:

- Checks if that mountpoint exists.

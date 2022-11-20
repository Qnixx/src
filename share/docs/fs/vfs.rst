:Author:
  Ian Marco Moffett

:Last updated: 2022-11-20

===============
Structures
===============

The Qnixx VFS module can be found in ``sys/include/fs/vfs.h``.


The ``fs_descriptor_t`` struct:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This struct holds information about the filesystem,
each filesystem will have it's own ``fs_descriptor_t``
struct.

``fs_descriptor_t`` fields:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
``size`` -> *Size of the filesystem in blocks.*

``blocksize`` -> *Blocksize for the filesystem.*

``ops`` -> *Filesystem operations*

The ``fs_ops_t`` struct:
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Holds function pointers to file system
operations.


The ``fs_t`` struct:
~~~~~~~~~~~~~~~~~~~~~
Represents a filesystem.


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
This function parses a path a returns a linked list of parse nodes
 (**note: this array must be freed with** ``vfs_free_parse_nodes()``)

The ``vfs_mountfs()`` function.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function mounts a filesystem.
It takes *two* arguments:

``mountpoint``  -> *mountpoint path*

``desc`` -> *Filesystem descriptor*

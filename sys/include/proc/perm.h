#ifndef PERM_H_
#define PERM_H_

#include <lib/types.h>

/* Allows a process to grant permissions */
#define PERM_PERM (1 << 0)

/* Allows a process to read/write to I/O ports */
#define PERM_IO (1 << 1)

/* Allows a process to log to system logs */
#define PERM_SYSLOG (1 << 2)


/* All permissions (BE CAREFUL WITH THIS PLEASE!) */
#define PERM_SUPERUSER               \
  (PERM_PERM                         \
   | PERM_IO                         \
   | PERM_SYSLOG)


typedef uint32_t perm_t;


#endif // PERM_H_

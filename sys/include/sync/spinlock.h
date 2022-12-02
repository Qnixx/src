#ifndef SYNC_SPINLOCK_H_
#define SYNC_SPINLOCK_H_

#include <lib/types.h>
#include <lib/asm.h>

typedef uint8_t lock;

#define SPINLOCK_ACQUIRE(LOCK)                        \
  do {                                                \
    if (__sync_bool_compare_and_swap(&LOCK, 0, 1)) {  \
      break;                                          \
    }                                                 \
    ASMV("pause");                                    \
  } while (1);


#define SPINLOCK_RELEASE(LOCK) __sync_bool_compare_and_swap(&LOCK, 1, 0)

#endif

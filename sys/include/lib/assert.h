#ifndef ASSERT_H_
#define ASSERT_H_

#include <lib/log.h>
#include <lib/asm.h>

#define ASSERT(condition, failmsg)                                                                     \
  if (!(condition)) {                                                                                  \
    printk(PRINTK_PANIC "ASSERTION \"" #condition "\" FAILED (%s:%d)\n" failmsg, __FILE__, __LINE__);  \
    ASMV("cli; hlt");                                                                                  \
  }

#endif

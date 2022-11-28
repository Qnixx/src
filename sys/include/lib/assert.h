#ifndef ASSERT_H_
#define ASSERT_H_

#include <tty/console.h>
#include <lib/module.h>
#include <lib/asm.h>

#define ASSERT(condition, failmsg)                                                                     \
  if (!(condition)) {                                                                                  \
    vprintk(PRINTK_PANIC "ASSERTION \"" #condition "\" FAILED (%s:%d)\n" failmsg, __FILE__, __LINE__);  \
    ASMV("cli; hlt");                                                                                  \
  }

#endif

#ifndef ASSERT_H_
#define ASSERT_H_

#include <lib/log.h>
#include <lib/panic.h>
#include <lib/module.h>

#define ASSERT(condition, failmsg)                                                                     \
  if (!(condition)) {                                                                                  \
    printk(PRINTK_PANIC "ASSERTION \"" #condition "\" FAILED (%s:%d)\n" failmsg, __FILE__, __LINE__);  \
    panic();                                                                                          \
  }

#define ASSERTF(condition, failmsg, ...)                                                                            \
  if (!(condition)) {                                                                                               \
    printk(PRINTK_PANIC "ASSERTION \"" #condition "\" FAILED (%s:%d)\n" failmsg, __FILE__, __LINE__, __VA_ARGS__);  \
    panic();                                                                                                       \
  }


#endif

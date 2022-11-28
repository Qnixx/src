#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdarg.h>
#include <lib/types.h>
#include <lib/log.h>

// vprintk with module name.
#define vnprintk(fmt, ...)          \
  vprintk("[");                     \
  vprintk(__MODULE_NAME);           \
  vprintk("]: " fmt, __VA_ARGS__);


// Video printk.
void vprintk(const char* fmt, ...);


#endif

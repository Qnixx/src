#include <proc/proc.h>
#include <proc/sched.h>
#include <arch/x86/cpu/smp.h>
#include <tty/console.h>
#include <lib/module.h>


MODULE_NAME("proc");
MODULE_DESCRIPTION("Process management");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


static uint8_t supports_smp = 0;
core_t* proc_corelist = NULL;


uint8_t is_smp_supported(void) {
  return supports_smp;
}


_noreturn void tasking_init(void) {
  // Bootstrap the cores.
  supports_smp = !__smp_bootstrap_cores(&proc_corelist);

  while (1);
  __builtin_unreachable();
}

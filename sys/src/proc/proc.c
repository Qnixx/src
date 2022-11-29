#include <proc/proc.h>
#include <arch/x86/cpu/smp.h>


static uint8_t supports_smp = 0;


uint8_t is_smp_supported(void) {
  return supports_smp;
}


_noreturn void tasking_init(void) {
  // Bootstrap the cores.
  supports_smp = !__smp_bootstrap_cores();

  while (1) {
    ASMV("hlt");
  }

  __builtin_unreachable();
}

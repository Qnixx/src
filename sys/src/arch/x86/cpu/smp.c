#include <arch/x86/cpu/smp.h>
#include <lib/limine.h>
#include <lib/asm.h>
#include <lib/module.h>
#include <tty/console.h>
#include <mm/vmm.h>

MODULE_NAME("smp");
MODULE_DESCRIPTION("Symmetric Multiprocessing module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


static volatile struct limine_smp_request smp_req = {
  .id = LIMINE_SMP_REQUEST,
  .revision = 0
};


static void __core_entry(struct limine_smp_info* info) {
  while (1) {
    ASMV("hlt");
  }
}


uint8_t __smp_bootstrap_cores(void) {
  struct limine_smp_response* smp_resp = smp_req.response;

  if (smp_resp->cpu_count == 1) {
    vnprintk("CPU only has a single core, SMP not supported (skipping)\n", 0);
    return 1;
  }

  vnprintk("CPU has %d cores.\n", smp_resp->cpu_count);
  vnprintk("Bootstrapping %d cores", smp_resp->cpu_count - 1);

  for (size_t i = 0; i < smp_resp->cpu_count; ++i) {
    // Do not bootstrap the BSP!
    if (smp_resp->cpus[i]->lapic_id == smp_resp->bsp_lapic_id) continue;

    vprintk(".");
    smp_resp->cpus[i]->extra_argument = (uint64_t)get_cr3();
    smp_resp->cpus[i]->goto_address = __core_entry;
  }

  vprintk("\n");
  return 0;
}

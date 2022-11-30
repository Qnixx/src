#include <arch/x86/cpu/smp.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x64/idt.h>
#include <arch/x86/gdt.h>
#include <arch/x86/tss.h>
#include <lib/limine.h>
#include <lib/asm.h>
#include <lib/module.h>
#include <tty/console.h>
#include <mm/vmm.h>
#include <mm/heap.h>

MODULE_NAME("smp");
MODULE_DESCRIPTION("Symmetric Multiprocessing module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


static volatile struct limine_smp_request smp_req = {
  .id = LIMINE_SMP_REQUEST,
  .revision = 0
};


static void __core_entry(struct limine_smp_info* info) {
  load_idt();
  load_gdt();
  tss_init();
  __lapic_init();

  ASMV("sti");
  while (1) {
    ASMV("hlt");
  }
}

uint8_t smp_get_core_count(void) {
  return smp_req.response->cpu_count;
}

uint8_t __smp_bootstrap_cores(core_t** cores) {
  struct limine_smp_response* smp_resp = smp_req.response;

  if (smp_resp->cpu_count == 1) {
    vnprintk("CPU only has a single core, SMP not supported (skipping)\n", 0);
    return 1;
  }

  *cores = kmalloc(sizeof(core_t) * (smp_resp->cpu_count));
  core_t* core_list = *cores;
  size_t core_list_idx = 0;

  vnprintk("CPU has %d cores.\n", smp_resp->cpu_count);
  vnprintk("Bootstrapping %d cores", smp_resp->cpu_count - 1);

  for (size_t i = 0; i < smp_resp->cpu_count; ++i) {
    // Do not bootstrap the BSP!
    if (smp_resp->cpus[i]->lapic_id == smp_resp->bsp_lapic_id) continue;

    // Add core to the core list.
    core_list[core_list_idx].queue_size = 0;
    core_list[core_list_idx++].lapic_id = smp_resp->cpus[i]->lapic_id;

    vprintk(".");
    smp_resp->cpus[i]->extra_argument = (uint64_t)get_cr3();
    smp_resp->cpus[i]->goto_address = __core_entry;
  }

  vprintk("\n");
  return 0;
}

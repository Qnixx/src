#include <arch/cpu/smp.h>
#include <arch/x64/idt.h>
#include <arch/x86/apic/lapic.h>
#include <intr/intr.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <lib/limine.h>
#include <lib/module.h>
#include <lib/log.h>
#include <lib/panic.h>
#include <lib/asm.h>

MODULE("smp");

static uintptr_t start_pml4 = 0;
static size_t core_count = 0;


static volatile struct limine_smp_request smp_req = {
  .id = LIMINE_SMP_REQUEST,
  .revision = 0
};


static struct limine_smp_response* smp_resp = NULL;


size_t smp_get_core_count(void) {
  return core_count;
}


size_t smp_get_bsp_id(void) {
  return smp_resp->bsp_lapic_id;
}


_noreturn static void core_init(struct limine_smp_info* info) {
  ASMV("mov %0, %%cr3" :: "a" (info->extra_argument));
  load_idt();
  init_interrupts();
  lapic_init();

  while (1) {
    ASMV("hlt");
  }
}


void smp_init(core_t** core_list_ptr) {
  smp_resp = smp_req.response;
  start_pml4 = mkpml4();
  core_count = smp_resp->cpu_count;
  struct limine_smp_info** cores = smp_resp->cpus;

  printk("[%s]: CPU has %d cores.\n", MODULE_NAME, core_count);

  if (core_count == 1) {
    printk(PRINTK_PANIC "CPU only has 1 core, Qnixx requires at least 2!\n");
    panic();
  }

  *core_list_ptr = kmalloc(sizeof(core_t) * core_count);
  for (size_t i = 0; i < core_count; ++i) {
    cores[i]->extra_argument = start_pml4;
    cores[i]->goto_address = core_init;

    (*core_list_ptr)[i].roll = 0;
    (*core_list_ptr)[i].queue_head = 0;
    (*core_list_ptr)[i].queue_base = 0;
    (*core_list_ptr)[i].queue_size = 0;
    (*core_list_ptr)[i].lapic_id = cores[i]->lapic_id;

    if (cores[i]->lapic_id != smp_resp->bsp_lapic_id)
      printk("[%s]: Bootstrapped core with LAPIC of %d\n", MODULE_NAME, cores[i]->lapic_id);
  }
}

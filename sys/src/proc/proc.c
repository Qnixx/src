#include <proc/proc.h>
#include <proc/sched.h>
#include <arch/x86/cpu/smp.h>
#include <arch/x86/apic/lapic.h>
#include <tty/console.h>
#include <lib/module.h>
#include <lib/bootmodule.h>
#include <lib/assert.h>
#include <lib/elf.h>


MODULE_NAME("proc");
MODULE_DESCRIPTION("Process management");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


static uint8_t supports_smp = 0;
core_t* proc_corelist = NULL;


_noreturn static void processor_idle(void) {
  while (1) {
    ASMV("hlt");
  }
}


static void send_process(process_t* p, uint64_t rip) {
  trapframe_t* tf = &p->tf;
  tf->rip = rip;
  tf->cs = 0x38 | 3;
  tf->rflags = 0x202;
  tf->rsp = p->rsp;
  tf->ss = 0x40 | 3;

  /*
   * If we are on an SMP system (p->host_core != NULL)
   * then we must send an IPI to the host core.
   *
   * Otherwise we can just do int 0x82.
   *
   */
  if (p->host_core != NULL) {
    lapic_send_ipi(p->host_core->lapic_id, 0x82);
  } else {
    ASMV("int $0x82");
  }
}

core_t* proc_find_core(size_t lapic_id) {
  for (uint8_t i = 0; i < smp_get_core_count(); ++i) {
    if (proc_corelist[i].lapic_id == lapic_id) {
      return &proc_corelist[i];
    }
  }

  return NULL;
}


uint8_t is_smp_supported(void) {
  return supports_smp;
}


process_t* get_running_process(void) {
  if (is_smp_supported()) {
    return proc_find_core(lapic_read_id())->running_process;
  } else {
    return get_running_process_singlecore();
  }
}


_noreturn void tasking_init(void) {
  // Bootstrap the cores.
  supports_smp = !__smp_bootstrap_cores(&proc_corelist);

  struct limine_file* init = get_module("/Qnixx/init.sys");
  ASSERT(init != NULL, "init.sys not found!\n");

  uintptr_t init_entrypoint = (uintptr_t)elf_load(init->address);
  process_t* init_process = sched_make_task(1);
  init_process->pmask |= PERM_SUPERUSER;


  send_process(init_process, init_entrypoint);

  processor_idle();
  __builtin_unreachable();
}

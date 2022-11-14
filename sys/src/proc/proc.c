#include <proc/proc.h>
#include <proc/sched.h>
#include <arch/cpu/smp.h>
#include <mm/heap.h>
#include <lib/log.h>
#include <lib/module.h>
#include <intr/intr.h>

MODULE("proc");

core_t* proc_cores = NULL;
static size_t core_count = 0;


_noreturn void tasking_init(void) {
  smp_init(&proc_cores);
  core_count = smp_get_core_count();
  sched_init();

  init_etc_irqs();

  while (1);
}

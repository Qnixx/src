#include <proc/proc.h>
#include <arch/cpu/smp.h>
#include <mm/heap.h>
#include <lib/log.h>
#include <lib/module.h>

MODULE("proc");

static core_t* cores = NULL;
static size_t core_count = 0;


_noreturn void tasking_init(void) {
  core_count = smp_get_core_count();
  cores = kmalloc(sizeof(core_t) * core_count);

  smp_init(cores);

  while (1);
}

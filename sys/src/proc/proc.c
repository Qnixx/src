#include <proc/proc.h>
#include <proc/sched.h>
#include <arch/cpu/smp.h>
#include <arch/x86/apic/lapic.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/module.h>
#include <lib/asm.h>
#include <intr/intr.h>


MODULE("proc");

core_t* proc_cores = NULL;
static size_t core_count = 0;


core_t* proc_find_core(size_t lapic_id) {
  for (size_t i = 0; i < core_count; ++i) {
    if (proc_cores[i].lapic_id == lapic_id) {
      return &proc_cores[i];
    }
  }

  return NULL;
}


void proc_clone(uint64_t to_rip) {
  ASMV("cli");
  core_t* this_core = proc_find_core(lapic_read_id());
  process_t* new_process = sched_make_task(this_core, 0);
  
  // Setup the IRET stackframe.
  trapframe_t* tf = &new_process->tf;
  tf->rip = to_rip;
  tf->cs = 0x28;
  tf->rflags = 0x202; 
  tf->rsp = new_process->rsp;
  tf->ss = 0x30;
  
  // Setup the the address space and other fields.
  new_process->cr3 = this_core->running_process->cr3;
  new_process->is_ring3 = 0;
  
  // Setup stack.
  void* parent_stack = (void*)this_core->running_process->stack_base;
  void* child_stack = (void*)new_process->stack_base;

  kmemcpy(child_stack, parent_stack, PSTACK_SIZE);
  new_process->stack_base = (uint64_t)child_stack;
  new_process->rsp = (uint64_t)child_stack + PSTACK_SIZE-1;
  new_process->tf.rsp = new_process->rsp;
  ASMV("sti");
}

_noreturn void tasking_init(void) {
  smp_init(&proc_cores);
  core_count = smp_get_core_count();
  sched_init();
  
  // Make a new task for the bootstrap processor (kernel main task).
  core_t* this_core = proc_find_core(lapic_read_id());
  process_t* p = sched_make_task(this_core, 0);
  p->cr3 = get_cr3();
  this_core->running_process = p;

  init_etc_irqs();
  while (1);
}

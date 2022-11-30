#include <proc/sched.h>
#include <arch/x86/cpu/smp.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x86/ring3.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <mm/mmap.h>
#include <lib/module.h>
#include <lib/string.h>
#include <lib/assert.h>
#include <intr/intr.h>

#define USER_STACKBASE 0x1000


MODULE_NAME("sched");
MODULE_DESCRIPTION("Scheduler");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");



extern core_t* proc_corelist;

// Stuff for CPUs with a single core.
static process_t* sc_queuebase = NULL;
static process_t* sc_queuehead = NULL;
static process_t* sc_running_process = NULL;
static size_t next_pid = 1;


/*
 * Locates a core with
 * the smallest queue.
 *
 */

static core_t* locate_with_smallest_queue(void) {
  uint8_t core_count = smp_get_core_count();
  core_t* chosen = NULL;

  for (uint8_t i = 0; i < core_count; ++i) {
    if (chosen == NULL) {
      chosen = &proc_corelist[i];
    } else if (proc_corelist[i].queue_size < chosen->queue_size) {
      chosen = &proc_corelist[i];
    }
  }

  return chosen;
}


static void init_proc(process_t* p, uint8_t is_ring3) {
  p->cr3 = vmm_mkpml4();
  p->is_ring3 = is_ring3;

  if (!(is_ring3)) {
    p->rsp_base = (uintptr_t)kmalloc(0x1000);
  } else {
    p->rsp_base = USER_STACKBASE;

    uintptr_t old_cr3 = get_cr3();
    ASMV("mov %0, %%cr3" :: "r" (p->cr3));
    k_mmap((void*)USER_STACKBASE, 1, PROT_READ | PROT_WRITE | PROT_USER);
    ASMV("mov %0, %%cr3" :: "r" (old_cr3));
  }

  p->rsp = p->rsp_base + 0x1000-1;
  p->pid = next_pid++;
  p->next = NULL;
}


static process_t* sched_singlecore(uint8_t is_ring3) {
  if (sc_queuebase == NULL) {
    sc_queuebase = kmalloc(sizeof(process_t));
    sc_queuehead = sc_queuebase;
    sc_running_process = sc_queuebase;
    sc_running_process->host_core = NULL;
    init_proc(sc_queuehead, is_ring3);
    return sc_queuehead;
  }

  sc_queuehead->next = kmalloc(sizeof(process_t));
  sc_queuehead = sc_queuehead->next;
  init_proc(sc_queuehead, is_ring3);
  return sc_queuehead;
}


static process_t* sched_multicore(uint8_t is_ring3) {
  /* Find a core with the smallest queue */
  core_t* core = locate_with_smallest_queue();
  if (core->queue_base == NULL) {
    core->queue_base = kmalloc(sizeof(process_t));
    core->queue_head = core->queue_base;
    core->running_process = core->queue_base;
    core->running_process->host_core = core;
    init_proc(core->queue_head, is_ring3);
    return core->queue_head;
  }

  core->queue_head->next = kmalloc(sizeof(process_t));
  core->queue_head = core->queue_head->next;
  core->queue_head->host_core = core;
  init_proc(core->queue_head, is_ring3);
  return core->queue_head;
}


_noreturn static void __sched_multicore(trapframe_t* tf) {
  core_t* this_core = proc_find_core(lapic_read_id());

  /* Copy the trapframe over */
  kmemcpy(&this_core->running_process->tf, tf, sizeof(trapframe_t));

  /* Get to the next process */
  this_core->running_process =
      this_core->running_process->next != NULL
      ? this_core->running_process->next : this_core->queue_base;

  /* Copy over the new trapframe */
  kmemcpy(tf, &this_core->running_process->tf, sizeof(trapframe_t));

  /* Change stack and address space */
  ASMV("mov %0, %%cr3" :: "r" ((this_core->running_process->cr3)));
  ASMV("mov %0, %%rsp" :: "r" ((this_core->running_process->rsp)));

  trap_exit();
  __builtin_unreachable();
}


_noreturn static void __sched_singlecore(trapframe_t* tf) {
  kmemcpy(&sc_running_process->tf, tf, sizeof(trapframe_t));

  sc_running_process =
    sc_running_process->next != NULL
    ? sc_running_process->next : sc_queuebase;

  kmemcpy(tf, &sc_running_process->tf, sizeof(trapframe_t));
  ASMV("mov %0, %%cr3" :: "r" ((sc_running_process->cr3)));
  ASMV("mov %0, %%rsp" :: "r" ((sc_running_process->rsp)));

  trap_exit();
  __builtin_unreachable();
}


_noreturn void __sched(trapframe_t* tf) {
  if (is_smp_supported()) {
    __sched_multicore(tf);
  } else {
    __sched_singlecore(tf);
  }

  __builtin_unreachable();
}

_noreturn void __user_process_entry(void) {
  ASMV("cli");
  core_t* this_core = proc_find_core(lapic_read_id());
  enter_ring3(this_core->running_process->tf.rip, this_core->running_process->tf.rsp);
  __builtin_unreachable();
}

void sched(trapframe_t* tf) {
  for (uint8_t i = 0; i < smp_get_core_count(); ++i) {
    if (proc_corelist[i].queue_base != NULL) {
      /* This vector sends the core to __sched() */
      lapic_send_ipi(proc_corelist[i].lapic_id, 0x81);
    }
  }
}


process_t* sched_make_task(uint8_t is_ring3) {
  if (!(is_smp_supported())) {
    return sched_singlecore(is_ring3);
  }

  return sched_multicore(is_ring3);
 }

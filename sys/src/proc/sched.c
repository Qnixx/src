#include <proc/sched.h>
#include <arch/x86/cpu/smp.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <lib/module.h>

MODULE_NAME("sched");
MODULE_DESCRIPTION("Scheduler");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");



extern core_t* proc_corelist;

// Stuff for CPUs with a single core.
static process_t* sc_queuebase = NULL;
static process_t* sc_queuehead = NULL;
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


static void init_proc(process_t* p) {
  p->cr3 = vmm_mkcr3();
  p->rsp_base = (uintptr_t)kmalloc(0x1000);
  p->rsp = p->rsp_base + 0x1000-1;
  p->pid = next_pid++;
  p->next = NULL;
}


static process_t* sched_singlecore(void) {
  if (sc_queuebase == NULL) {
    sc_queuebase = kmalloc(sizeof(process_t));
    sc_queuehead = sc_queuebase;
    init_proc(sc_queuehead);
    return sc_queuehead;
  }

  sc_queuehead->next = kmalloc(sizeof(process_t));
  sc_queuehead = sc_queuehead->next;
  init_proc(sc_queuehead);
  return sc_queuehead;
}


static process_t* sched_multicore(void) {
  /* Find a core with the smallest queue */
  core_t* core = locate_with_smallest_queue();
  if (core->queue_base == NULL) {
    core->queue_base = kmalloc(sizeof(process_t));
    core->queue_head = core->queue_base;
    init_proc(core->queue_head);
    return core->queue_head;
  }

  core->queue_head->next = kmalloc(sizeof(process_t));
  core->queue_head = core->queue_head->next;
  init_proc(core->queue_head);
  return core->queue_head;
}


process_t* sched_task(void) {
  if (!(is_smp_supported())) {
    return sched_singlecore();
  }

  return sched_multicore();
 }

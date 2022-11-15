#include <proc/sched.h>
#include <lib/rand.h>
#include <lib/assert.h>
#include <lib/string.h>
#include <lib/asm.h>
#include <arch/cpu/smp.h>
#include <arch/x86/apic/lapic.h>
#include <mm/heap.h>


static uint32_t core_count = 0;
static size_t next_pid = 1;



/*
 *  Finds a core with the smallest queue.
 *
 */
static core_t* locate_smallest_queue(void) {
  core_t* core = &proc_cores[0];
  uint8_t is_all_same = 1;
  for (size_t i = 1; i < core_count; ++i) {
    if (proc_cores[i].queue_size < core->queue_size) {
      is_all_same = 0;
      core = &proc_cores[i];
    }
  }

  if (is_all_same)
    return NULL;

  return core;
}


static core_t* locate_highest_roll(void) {
  core_t* highest_roller = &proc_cores[0];
  uint8_t found_high_roll = 0;

  do {
    for (size_t i = 1; i < core_count; ++i) {
      proc_cores[i].roll = rand();
      if (proc_cores[i].roll > highest_roller->roll) {
        found_high_roll = 1;
        highest_roller = &proc_cores[i];
      }
    }
  } while (!(found_high_roll));

  return highest_roller;
}


core_t* sched_core(void) { 
  if (locate_smallest_queue() == NULL) {
    return locate_highest_roll();
  }
  
  return NULL;
}


process_t* sched_make_task(core_t* to, uint8_t is_ring3) {
  ASSERT(to != NULL, "to is NULL!\n");
  if (to->queue_base == NULL) {
    to->queue_base = kmalloc(sizeof(process_t));
    to->queue_head = to->queue_base;
  } else {
    to->queue_head->next = kmalloc(sizeof(process_t));
    to->queue_head = to->queue_head->next;
  }

  process_t* head = to->queue_head;
  kmemzero(&head->tf, sizeof(trapframe_t));
  head->pid = next_pid++;
  
  // TODO: For userspace tasks, allocate memory on the lower half instead of higher half.
  // NOTE: kmalloc() allocates on higher half.
  if (!(is_ring3)) {
    head->stack_base = (uint64_t)kmalloc(PSTACK_SIZE);
    head->rsp = head->stack_base + PSTACK_SIZE-1;
  }

  head->is_ring3 = is_ring3;
  head->next = NULL;
  return head;
}


void sched(trapframe_t* tf) {
  // Copy current trapframe into current process's copy.
  core_t* this_core = proc_find_core(lapic_read_id());
  kmemcpy(&this_core->running_process->tf, tf, sizeof(trapframe_t));

  // Switch tasks.
  if (this_core->running_process->next) {
    this_core->running_process = this_core->running_process->next;
  } else {
    this_core->running_process = this_core->queue_base;
  }
  
  // Copy trapframe copy into current trapframe and switch CR3.
  kmemcpy(tf, &this_core->running_process->tf, sizeof(trapframe_t));
  ASMV("mov %0, %%cr3" :: "a" (this_core->running_process->cr3));
}


void sched_init(void) {
  core_count = smp_get_core_count();
  srand(0xFF ^ core_count);
}

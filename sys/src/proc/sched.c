#include <proc/sched.h>
#include <lib/rand.h>
#include <arch/cpu/smp.h>


static uint32_t core_count = 0;



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


void sched_init(void) {
  core_count = smp_get_core_count();
  srand(0xFF ^ core_count);
}

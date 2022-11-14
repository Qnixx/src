#ifndef SCHED_H_
#define SCHED_H_

#include <proc/proc.h>


core_t* sched_core(void);

// Pushes a task to a core's queue.
process_t* sched_make_task(core_t* to);

void sched_init(void);

#endif

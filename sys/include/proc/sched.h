#ifndef SCHED_H_
#define SCHED_H_

#include <proc/proc.h>

process_t* sched_make_task(uint8_t is_ring3);
process_t* get_running_process_singlecore(void);
void sched(trapframe_t* tf);
_noreturn void __sched(trapframe_t* tf);
_noreturn void __user_process_entry(void);

#endif

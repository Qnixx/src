#include <intr/syscall.h>
#include <proc/proc.h>
#include <sys/errno.h>
#include <tty/console.h>


/*
 * Returned in RAX: Errno.
 * RBX: Msg.
 *
 */
static void sys_syslog(trapframe_t* tf) {
  perm_t perms = get_running_process()->pmask;
  if (!(perms & PERM_SYSLOG)) {
    tf->rax = EPERM;
    return;
  }

  vprintk((const char*)tf->rbx);
}

void(*syscalls[MAX_SYSCALLS])(trapframe_t* tf) = {
  sys_syslog,
};

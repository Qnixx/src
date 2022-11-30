#include <intr/syscall.h>
#include <proc/proc.h>
#include <sys/errno.h>
#include <tty/console.h>
#include <arch/x86/io.h>


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
  tf->rax = 0;
}

/*
 *
 * For port I/O.
 *
 * RBX: Port I/O type.
 * RCX: Port.
 * RDX: Value.
 *
 */

#define PIO_TYPE_BYTES 0
#define PIO_TYPE_WORDS 1
#define PIO_TYPE_DWORDS 2
static void sys_pio(trapframe_t* tf) {
  perm_t perms = get_running_process()->pmask;

  if (!(perms & PERM_PIO)) {
    tf->rax = EPERM;
    return;
  }

  switch (tf->rbx) {
    case PIO_TYPE_BYTES:
      outb(tf->rcx, tf->rdx);
      break;
    case PIO_TYPE_WORDS:
      outw(tf->rcx, tf->rdx);
      break;
    case PIO_TYPE_DWORDS:
      outl(tf->rcx, tf->rdx);
      break;
  }

  tf->rax = 0;
}

void(*syscalls[MAX_SYSCALLS])(trapframe_t* tf) = {
  sys_syslog,
  sys_pio
};

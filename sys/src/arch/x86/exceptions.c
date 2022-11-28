#include <arch/x86/exceptions.h>
#include <arch/x64/idt.h>
#include <tty/console.h>

#define INTR_END \
  ASMV("cli; hlt\n");

typedef struct {
  uint64_t error_code;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} stackframe_t;


static void dump_state(void* stackframe) {
  stackframe_t* frame = stackframe;
  vprintk(PRINTK_PANIC "-- Begin dump of exception stackframe --\n");
  vprintk(PRINTK_PANIC "ERROR_CODE=%x RIP=%x\n", frame->error_code, frame->rip);
  vprintk(PRINTK_PANIC "RFLAGS=%x RSP=%x\n", frame->rflags, frame->rsp);
  vprintk(PRINTK_PANIC "SS=%x\n", frame->ss);
}

E_ISR(0x0) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Division by zero\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x1) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Debug exception\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x2) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Non-Maskable Interrupt\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x3) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Breakpoint exception\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x4) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Overflow exception\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x5) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "BOUND range exceeded\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x6) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Invalid opcode\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x7) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Coprocessor not available\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x8) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Double fault\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x9) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Coprocessor segment overrun\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0xA) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Invalid TSS\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0xB) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Segment not present\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0xC) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Stack segment fault\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0xD) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "General protection fault\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0xE) {
  ASMV("cli");
  vprintk(PRINTK_PANIC "Page fault\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0xF) {
  vprintk(PRINTK_PANIC "Reserved exception\n");
  dump_state(stackframe);
  INTR_END;
}

E_ISR(0x10) {
  vprintk(PRINTK_PANIC "Coprocessor error\n");
  dump_state(stackframe);
  INTR_END;
}

void init_exceptions(void) {
  register_exception_handler(0x0, REF_E_ISR(0x0));
  register_exception_handler(0x1, REF_E_ISR(0x1));
  register_exception_handler(0x2, REF_E_ISR(0x2));
  register_exception_handler(0x3, REF_E_ISR(0x3));
  register_exception_handler(0x4, REF_E_ISR(0x4));
  register_exception_handler(0x5, REF_E_ISR(0x5));
  register_exception_handler(0x6, REF_E_ISR(0x6));
  register_exception_handler(0x7, REF_E_ISR(0x7));
  register_exception_handler(0x8, REF_E_ISR(0x8));
  register_exception_handler(0x9, REF_E_ISR(0x9));
  register_exception_handler(0xA, REF_E_ISR(0xA));
  register_exception_handler(0xB, REF_E_ISR(0xB));
  register_exception_handler(0xC, REF_E_ISR(0xC));
  register_exception_handler(0xD, REF_E_ISR(0xD));
  register_exception_handler(0xE, REF_E_ISR(0xE));
  register_exception_handler(0xF, REF_E_ISR(0xF));
  register_exception_handler(0x10, REF_E_ISR(0x10));
}

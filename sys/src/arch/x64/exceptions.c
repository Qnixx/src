#include <arch/x64/exceptions.h>
#include <arch/x64/idt.h>
#include <lib/log.h>
#include <lib/asm.h>

#define INTR_END \
  ASMV("cli; hlt\n");

E_ISR(0x0) {
  printk(PRINTK_PANIC "Division by zero\n");
  INTR_END;
}

E_ISR(0x1) {
  printk(PRINTK_PANIC "Debug exception\n");
  INTR_END;
}

E_ISR(0x2) {
  printk(PRINTK_PANIC "Non-Maskable Interrupt\n");
  INTR_END;
}

E_ISR(0x3) {
  printk(PRINTK_PANIC "Breakpoint exception\n");
  INTR_END;
}

E_ISR(0x4) {
  printk(PRINTK_PANIC "Overflow exception\n");
  INTR_END;
}

E_ISR(0x5) {
  printk(PRINTK_PANIC "BOUND range exceeded\n");
  INTR_END;
}

E_ISR(0x6) {
  printk(PRINTK_PANIC "Invalid opcode\n");
  INTR_END;
}

E_ISR(0x7) {
  printk(PRINTK_PANIC "Coprocessor not available\n");
  INTR_END;
}

E_ISR(0x8) {
  printk(PRINTK_PANIC "Double fault\n");
  INTR_END;
}

E_ISR(0x9) {
  printk(PRINTK_PANIC "Coprocessor segment overrun\n");
  INTR_END;
}

E_ISR(0xA) {
  printk(PRINTK_PANIC "Invalid TSS\n");
  INTR_END;
}

E_ISR(0xB) {
  printk(PRINTK_PANIC "Segment not present\n");
  INTR_END;
}

E_ISR(0xC) {
  printk(PRINTK_PANIC "Stack segment fault\n");
  INTR_END;
}

E_ISR(0xD) {
  printk(PRINTK_PANIC "General protection fault\n");
  INTR_END;
}

E_ISR(0xE) {
  printk(PRINTK_PANIC "Page fault\n");
  INTR_END;
}

E_ISR(0xF) {
  printk(PRINTK_PANIC "Reserved exception\n");
  INTR_END;
}

E_ISR(0x10) {
  printk(PRINTK_PANIC "Coprocessor error\n");
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

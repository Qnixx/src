#include <intr/pic.h>
#include <arch/x86/io.h>

#define SLAVE_INDEX 2
#define PIC0_CMD 0x20
#define PIC0_DAT 0x21
#define PIC1_CMD 0xA0
#define PIC1_DAT 0xA1
#define IRQ_BASE 0x20

void pic_enable(uint8_t vector) {
    uint8_t imr;
    if(vector & 8) {
        imr = inb(PIC1_DAT);
        imr &= ~(1 << (vector - 8));
        outb(PIC1_DAT, imr);
    } else {
        imr = inb(PIC0_DAT);
        imr &= ~(1 << vector);
        outb(PIC0_DAT, imr);
    }
}

void pic_disable(uint8_t vector) {
    uint8_t imr;
    if(vector & 8) {
        imr = inb(PIC1_DAT);
        imr |= 1 << (vector - 8);
        outb(PIC1_DAT, imr);
    } else {
        imr = inb(PIC0_DAT);
        imr |= 1 << vector;
        outb(PIC0_DAT, imr);
    }
}

void pic_EOI(uint8_t vector) {
    if(vector & 8) outb(PIC1_CMD, 0x20);
    outb(PIC0_CMD, 0x20);
}

uint16_t pic_getIRR(void) {
    outb(PIC0_CMD, 0x0a);
    outb(PIC1_CMD, 0x0a);
    return (inb(PIC1_CMD) << 8) | inb(PIC0_CMD);
}

uint16_t pic_getISR(void) {
    outb(PIC0_CMD, 0x0b);
    outb(PIC1_CMD, 0x0b);
    return (inb(PIC1_CMD) << 8) | inb(PIC0_CMD);
}

void pic_init(void) {
    // Enter edge triggered mode with cascading controllers
    outb(PIC0_CMD, 0x11);
    outb(PIC1_CMD, 0x11);

    // Remap IRQs to avoid exception ISRs
    outb(PIC0_DAT, IRQ_BASE);
    outb(PIC1_DAT, IRQ_BASE + 8);

    // Configure master-slave relationship
    outb(PIC0_DAT, 1 << SLAVE_INDEX);
    outb(PIC1_DAT, SLAVE_INDEX);

    // Enter x86 mode
    outb(PIC0_DAT, 0x01);
    outb(PIC1_DAT, 0x01);

    // Mask all interrupts on both PICs (handlers will unmask them)
    outb(PIC0_DAT, 0xff);
    outb(PIC1_DAT, 0xff);

    // Enable the master-slave communication line
    pic_enable(SLAVE_INDEX);
}

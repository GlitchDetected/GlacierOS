#include "../../headers/irq.h"
#include "../../headers/idt.h"
#include "../../headers/isr.h"
#include "../../headers/x86.h"
#include <stdint.h>

// PIC constants
#define PIC1 0x20
#define PIC1_OFFSET 0x20
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xA0
#define PIC2_OFFSET 0x28
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20
#define PIC_MODE_8086 0x01
#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10

#define PIC_WAIT() do {         \
asm ("jmp 1f\n\t"       \
"1:\n\t"        \
"    jmp 2f\n\t"\
"2:");          \
} while (0)

static void (*handlers[32])(struct Registers *regs) = { 0 };

static void stub(struct Registers *regs) {
    if (regs->int_no <= 47 && regs->int_no >= 32) {
        if (handlers[regs->int_no - 32]) {
            handlers[regs->int_no - 32](regs);
        }
    }

    // send EOI
    if (regs->int_no >= 0x40) {
        outp(PIC2, PIC_EOI);
    }

    outp(PIC1, PIC_EOI);
}

static void irq_remap() {
    uint8_t mask1 = inp(PIC1_DATA), mask2 = inp(PIC2_DATA);
    outp(PIC1, ICW1_INIT | ICW1_ICW4);
    outp(PIC2, ICW1_INIT | ICW1_ICW4);
    outp(PIC1_DATA, PIC1_OFFSET);
    outp(PIC2_DATA, PIC2_OFFSET);
    outp(PIC1_DATA, 0x04); // PIC2 at IRQ2
    outp(PIC2_DATA, 0x02); // Cascade identity
    outp(PIC1_DATA, PIC_MODE_8086);
    outp(PIC2_DATA, PIC_MODE_8086);
    outp(PIC1_DATA, mask1);
    outp(PIC2_DATA, mask2);
}

static void irq_set_mask(size_t i) {
    uint16_t port = i < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inp(port) | (1 << (i % 8));
    outp(port, value);
}

static void irq_clear_mask(size_t i) {
    uint16_t port = i < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inp(port) & ~(1 << (i % 8));
    outp(port, value);
}

void irq_install(size_t i, void (*handler)(struct Registers *)) {
    x86_cli();
    handlers[i] = handler;
    irq_clear_mask(i);
    x86_sti();
}

void irq_init() {
    irq_remap();

    for (size_t i = 0; i < 16; i++) {
        irq_install(32 + i, stub);
    }
}

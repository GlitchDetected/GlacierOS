#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>
#include "types.h"

typedef struct Registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} Registers;

void irq_install(size_t i, void (*handler)(struct Registers*));
void irq_init();

#endif

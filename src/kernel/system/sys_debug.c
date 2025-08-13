#include "../../headers/process.h"
#include "../../headers/kernel.h"
#include "../../headers/strings.h"
#include "../../headers/isr.h"
#include "../../headers/x86.h"

#define __UNUSED__ __attribute__((unused))

// syscall_debug_puts(char* string)
long syscall_debug_puts(isr_ctx_t *regs) {
    DEBUG("APP[%i]: %s\n", task_list_current->id, regs->rdi);
    return 1;
}


    .section .text
    .globl interrupt
    .extern isr_handler
    .extern irq_handler

    .text
    .p2align 4

# ------------------------------
# Macro equivalents for GAS
# ------------------------------

# ISR stub with error code
.macro isr_stub_err num
    .globl isr_stub_\num
isr_stub_\num:
    cli
    movl $\num, 4(%rsp)    # store interrupt number in high dword of error code
    jmp isr_common_stub
.endm

# ISR stub without error code
.macro isr_stub_noerr num
    .globl isr_stub_\num
isr_stub_\num:
    cli
    pushq $0                # push dummy 64-bit error code
    movl $\num, 4(%rsp)    # store interrupt number in high dword
    jmp isr_common_stub
.endm

# IRQ ISR stub
.macro isr_stub_irq num
    .globl isr_stub_\num
isr_stub_\num:
    cli
    pushq $0
    movl $\num, 4(%rsp)
    jmp isr_common_stub
.endm

# NOP stub
.macro isr_stub_nop num
    .globl isr_stub_\num
isr_stub_\num:
    iretq
.endm

# IPI stub
.macro isr_stub_ipi num val
    .globl isr_stub_\num
isr_stub_\num:
    cli
    pushq $\val
    jmp isr_common_stub
.endm

# ------------------------------
# Common ISR handler
# ------------------------------
isr_common_stub:
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    mov %rsp, %rdi
    call isr_handler

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    add $8, %rsp
    sti
    iretq

# ------------------------------
# Standard interrupts/traps
# ------------------------------
isr_stub_noerr 0
isr_stub_noerr 1
isr_stub_noerr 2
isr_stub_noerr 3
isr_stub_noerr 4
isr_stub_noerr 5
isr_stub_noerr 6
isr_stub_noerr 7
isr_stub_err 8
isr_stub_noerr 9
isr_stub_err 10
isr_stub_err 11
isr_stub_err 12
isr_stub_err 13
isr_stub_err 14
isr_stub_noerr 15
isr_stub_noerr 16
isr_stub_err 17
isr_stub_noerr 18
isr_stub_noerr 19
isr_stub_noerr 20
isr_stub_noerr 21
isr_stub_noerr 22
isr_stub_noerr 23
isr_stub_noerr 24
isr_stub_noerr 25
isr_stub_noerr 26
isr_stub_noerr 27
isr_stub_noerr 28
isr_stub_noerr 29
isr_stub_err 30
isr_stub_noerr 31

# ------------------------------
# IRQ section (32-47)
# ------------------------------
isr_stub_irq 33
isr_stub_irq 34
isr_stub_irq 35
isr_stub_irq 36
isr_stub_irq 37
isr_stub_irq 38
isr_stub_irq 39
isr_stub_irq 40
isr_stub_irq 41
isr_stub_irq 42
isr_stub_irq 43
isr_stub_irq 44
isr_stub_irq 45
isr_stub_irq 46
isr_stub_irq 47

# ------------------------------
# Syscall
# ------------------------------
# isr_stub_noerr 128

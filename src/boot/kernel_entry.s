.code64
.section .text.prologue

.global kernel64_start
kernel64_start:
    mov $0xFFFF800000100000, %rsp
    mov rbx, %rsi
    mov $0xDEADBEEF, %rdi
    call kernel_main

halt_loop:
    hlt
    jmp halt_loop

.section .text
.align 4

.global idt_load
.type idt_load, @function
idt_load:
    mov 8(%rsp), %rax
    lidt (%rax)
    ret

.macro ISR_NO_ERR index
    .global _isr\index
    _isr\index:
        cli
        push 0
        push $\index
        jmp isr_common64
.endm

.macro ISR_ERR index
    .global _isr\index
    _isr\index:
        cli
        push $\index
        jmp isr_common64
.endm

ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR 8
ISR_NO_ERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15

.extern isr_handler
.type isr_handler, @function

isr_common64:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    push ds
    push es
    push fs
    push gs

    mov $0x10, %ax
    mov %ax, ds
    mov %ax, es
    mov %ax, fs
    mov %ax, gs
    cld

    push rsp
    call isr_handler
    add $8, %rsp

    pop gs
    pop fs
    pop es
    pop ds

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add $16, %rsp
    iretq

.section .bss
.align 32
stack:
    .skip 0x4000

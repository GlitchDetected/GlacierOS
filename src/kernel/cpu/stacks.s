    .globl gdt64
    .globl gdt64.tss
    .globl gdt64.pointer
    .globl tss64
    .globl tss64.rsp0
    .globl kernel_stack_top
    .globl ist_stack_1
    .globl ist_stack_2
    .globl interrupt_stack_table

    .bss
    .align 16

# Kernel stack
kernel_stack_bottom:
    .skip 8192
kernel_stack_top:
    .skip 4096

# IST stacks
ist_stack_1:
    .skip 4096
ist_stack_2:
    .skip 4096

# TSS
tss64:
    .skip 104
tss64.rsp0:
    .skip 24

    .data
    .align 16

# Interrupt stack table
interrupt_stack_table:
    .quad ist_stack_1
    .quad ist_stack_2
    .zero 5*8
    .word 0
    .word 0
    .word 0

# GDT (64-bit)
gdt64:
gdt64.null:
    .quad 0
gdt64.kernel_code:
    .word 0
    .word 0
    .byte 0
    .byte 0x98
    .byte 0x20
    .byte 0
gdt64.kernel_data:
    .word 0
    .word 0
    .byte 0
    .byte 0x92
    .byte 0
    .byte 0
gdt64.tss:
    .skip 104            # TSS descriptor
gdt64.pointer:
    .word . - gdt64 - 1  # limit
    .quad gdt64           # base

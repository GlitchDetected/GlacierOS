    .text
    .global isr_stub_128
    .extern system_call_handler

isr_stub_128:
    cli

    # Save general-purpose registers
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

    # Load kernel data segment into segment registers
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Call system_call_handler with pointer to register state
    mov %rsp, %rdi
    call system_call_handler

    # Restore registers
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
    # skip popping rax, adjust stack manually
    add $8, %rsp

    sti
    iretq

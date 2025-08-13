    .section .text
    .code64
    .global isr_stub_32
    .global irq0_first_jump

    .extern next_task
    .extern timer_callback
    .extern current_task_index
    .extern current_task
    .extern __switch_to

isr_stub_32:
    # Disable interrupts
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

    # Save current task's stack pointer
    call current_task        # returns pointer to current task struct in %rax
    mov %rsp, (%rax)         # store current rsp

irq0_first_jump:
    # Get next task to run
    call next_task           # returns pointer to next task struct in %rax
    mov (%rax), %rsp         # restore stack pointer of next task

    # Call task switch function
    mov %rax, %rdi           # argument: pointer to next task struct
    call __switch_to

    # Timer callback (optional tick handler)
    call timer_callback

    # Send End-of-Interrupt (EOI) to PIC
    mov $0x20, %al
    outb %al, $0x20

    # Optional: switch to user data segments (commented out)
    # mov $0x23, %ax         # GDT_USER_DATA | 3
    # mov %ax, %ds
    # mov %ax, %es
    # mov %ax, %fs
    # mov %ax, %gs

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
    pop %rax

    # Re-enable interrupts
    sti

    # Return from interrupt
    iretq

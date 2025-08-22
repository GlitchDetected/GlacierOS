.code64
.section .text.prologue
.global _start
_start:
    cli

    # Flush RIP to high-half (like flush_rip)
    leaq flush_rip(%rip), %rax
    jmp *%rax

flush_rip:
    # Load GDT
    mov gdt64.pointer(%rip), %rax
    lgdt (%rax)

    # Reload segment registers
    mov $0x10, %ax                  # GDT_KERNEL_DATA
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Setup new stack
    mov kernel_stack_top(%rip), %rsp
    mov tss64.rsp0(%rip), %rax
    mov %rsp, (%rax)

    # Setup 64-bit TSS descriptor
    lea gdt64.tss(%rip), %rdi       # pointer to TSS descriptor
    lea tss64(%rip), %rax           # pointer to TSS
    movw %ax, 2(%rdi)
    shr $16, %rax
    movb %al, 4(%rdi)
    shr $8, %rax
    movb %al, 7(%rdi)
    shr $8, %rax
    movl %eax, 8(%rdi)

    # Load TSS
    mov $0x28, %ax                  # GDT_TSS
    ltr %ax

    # Call kernel_main
    call kernel_main

halt_loop:
    hlt
    jmp halt_loop

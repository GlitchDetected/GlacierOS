###############################################################################
# bootloader.s - 16-bit real mode → 32-bit protected mode → 64-bit long mode
###############################################################################

.section .text
.code16gcc
.org 0x7c00
.global _start
_start:
    cli

    # Setup segment registers
    movw %cs, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    # Set stack
    movw $0x8000, %sp

    # Enable A20
    call enable_a20

    # Load GDT and IDT
    lgdt gdtp
    lidt idtp

    # Enable protected mode (PE flag)
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0

    # Far jump to flush prefetch queue and enter 32-bit protected mode
    ljmp $0x08, $entry32

###############################################################################
# 32-bit protected mode
###############################################################################
.code32
.globl entry32
entry32:
    # Temporary stack
    movl $0x90000, %esp

    # Setup paging (stub, you can fill later)
    call setup_paging_64

    # Enable PAE
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4

    # Enable paging (PG flag)
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    # Enable long mode in EFER
    movl $0xc0000080, %ecx
    rdmsr
    orl $0x100, %eax   # LME = 1
    wrmsr

    # Far jump to 64-bit kernel
    ljmp $0x10, $kernel64_start

###############################################################################
# 64-bit long mode
###############################################################################
.code64
.globl kernel64_start
kernel64_start:
    movq $0xFFFF800000100000, %rsp  # Higher-half stack

hlt_loop:
    hlt
    jmp hlt_loop

###############################################################################
# A20 line
###############################################################################
enable_a20:
    inb $0x64, %al
    testb $2, %al
    jnz enable_a20
    ret

###############################################################################
# Paging setup stub
###############################################################################
setup_paging_64:
    ret

###############################################################################
# GDT (protected + long mode)
###############################################################################
.align 16
gdt_start:
gdt_null:
    .quad 0x0
gdt_code32:
    .word 0xFFFF
    .word 0x0
    .byte 0x0
    .byte 0x9A
    .byte 0xCF
    .byte 0x0
gdt_data32:
    .word 0xFFFF
    .word 0x0
    .byte 0x0
    .byte 0x92
    .byte 0xCF
    .byte 0x0
gdt_code64:
    .word 0xFFFF
    .word 0x0
    .byte 0x0
    .byte 0x9A
    .byte 0xAF
    .byte 0x0
gdt_data64:
    .word 0xFFFF
    .word 0x0
    .byte 0x0
    .byte 0x92
    .byte 0xAF
    .byte 0x0
gdt_end:

gdtp:
    .word gdt_end - gdt_start - 1
    .long gdt_start

###############################################################################
# Empty IDT
###############################################################################
idt:
    .word 0
    .long 0
idtp:
    .word 0
    .long idt

###############################################################################
# Bootloader padding to 512 bytes
###############################################################################
.fill 510-(.-_start),1,0
.word 0xAA55

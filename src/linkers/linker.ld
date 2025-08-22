OUTPUT_FORMAT("elf64-x86-64")

/* Physical load address */
KERNEL_LMA = 0x100000;
/* Virtual address start */
KERNEL_VMA = 0xFFFF800000000000;

ENTRY(kernel_main)

SECTIONS
{
    /* Load the Multiboot2 header physically at KERNEL_LMA */
    . = KERNEL_LMA;
    .boot :
    {
        *(.multiboot_header)
    }

    . = KERNEL_VMA;

    /* Text/code section */
    .text : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.text*)
    }

    /* Read-only data */
    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.rodata*)
    }

    /* Initialized data */
    .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.data*)
    }

    /* Uninitialized data */
    .bss ALIGN(0x1000) : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.bss*)
        *(COMMON)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }

    kernel_end = .;
}

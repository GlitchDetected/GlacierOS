#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(__i386__)
#error "needs to be compiled with a ix86-elf compiler"
#endif

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

void kernel_main(void) {
    const char *msg = "Hello, OSDev!";
    uint16_t *vga = (uint16_t*)0xB8000;
    for (size_t i = 0; msg[i] != '\0'; i++) {
        vga[i] = (uint8_t)msg[i] | 0x0700;
    }
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
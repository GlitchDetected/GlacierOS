#include "../headers/isr.h"
#include "../headers/irq.h"
#include "../headers/timer.h"
#include "../headers/keyboard.h"
#include "../headers/memory.h"
#include "../headers/fpu.h"
#include "../headers/pic.h"
#include "../headers/kernel.h"
#include "../headers/mouse.h"
#include "../headers/process.h"
#include "../headers/windows.h"
#include "../headers/paging.h"
#include "../headers/vesa.h"
#include "../headers/fat.h"
#include <stdint.h>
#include "../headers/x86.h"
#include "../headers/serial.h"
#include "../headers/stdarg.h"
#include "../headers/printf.h"
#include "../headers/ata.h"

#define GDT_NULL        0x00
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x18
#define GDT_USER_DATA   0x20
#define GDT_TSS         0x28

int serial_printf_help(unsigned c, void *ptr __UNUSED__) {
    serial_write_com(1, c);
    return 0;
}

void HALT_AND_CATCH_FIRE(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)_printf(fmt, args, serial_printf_help, NULL);
    va_end(args);
    x86_hlt();
    while(1) {}
}

void _kernel_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)_printf(fmt, args, serial_printf_help, NULL);
    va_end(args);
}

void idle() {
    while(1) {
        x86_hlt();
    }
}

void kernel_main(unsigned long magic __UNUSED__, multiboot_info_t* mbi_phys) {
    init_serial();
    init_vesa(TO_VMA_PTR(multiboot_info_t *, mbi_phys));
    init_paging();
    init_pic();
    init_isr();
    init_fpu();
    init_irq();
    init_timer();
    init_keyboard();
    init_mouse();
    init_ata();
    init_fat();
    init_window_manager();
    create_kernel_process((void*)idle);
    create_user_process_file("/apps/desktop/desktop");
    do_first_task_jump();
    while(1) { }
}
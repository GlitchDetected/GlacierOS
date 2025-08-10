#include "./cpu/idt.h"
#include "./cpu/isr.h"
#include "./cpu/irq.h"
#include "./cpu/timer.h"
#include "./drivers/display.h"
#include "./drivers/keyboard.h"
#include "./cpu/fpu.h"
#include <stddef.h>
#include <stdint.h>
#include "utils.h"
#include "memory.h"
#include "kernel.h"
#include "./drivers/mouse.h"

#define LOGO_HEIGHT 5
static const char *LOGO[LOGO_HEIGHT] = {
    "==============================",
    "= GlacierOS                  =",
    "= Made by GlitchDetected     =",
    "=                            =",
    "==============================",
};

void print_logo(void) {
    for (int i = 0; i < LOGO_HEIGHT; i++) {
        print_string(LOGO[i]);
        print_nl();
    }
}

void* alloc(int n) {
    int *ptr = (int *) mem_alloc(n * sizeof(int));
    if (ptr == NULL_POINTER) {
        print_string("Memory not allocated.\n");
        screen_swap();
    }
    return ptr;
}

void execute_command(char *input) {
    if (compare_string(input, "EXIT") == 0) {
        print_string("Stopping the CPU processes... \n");
        screen_swap();
        asm volatile("hlt");
    }
    else if (compare_string(input, "HELLO") == 0) {
        print_string("Hello, user!\n> ");
        screen_swap();
    }
    else if (compare_string(input, "MEMORY") == 0) {
        print_string("Memory status:\n");
        print_dynamic_mem();
        print_string("> ");
        screen_swap();
    }
    else if (compare_string(input, "meminfo") == 0) {
        print_string("init_dynamic_mem()\n");
        print_dynamic_node_size();
        print_dynamic_mem();
        print_nl();
        screen_swap();
    }
    else if (compare_string(input, "alloc1") == 0) {
        ptr1 = alloc(5);
        print_string("int *ptr1 = alloc(5)\n");
        print_dynamic_mem();
        print_nl();
        screen_swap();
    }
    else if (compare_string(input, "alloc2") == 0) {
        ptr2 = alloc(10);
        print_string("int *ptr2 = alloc(10)\n");
        print_dynamic_mem();
        print_nl();
        screen_swap();
    }
    else if (compare_string(input, "alloc3") == 0) {
        ptr3 = alloc(2);
        print_string("int *ptr3 = alloc(2)\n");
        print_dynamic_mem();
        print_nl();
        screen_swap();
    }
    else if (compare_string(input, "free1") == 0) {
        if (ptr1 != NULL) {
            mem_free(ptr1);
            print_string("mem_free(ptr1)\n");
            print_dynamic_mem();
            print_nl();
            ptr1 = NULL;
        } else {
            print_string("ptr1 is already free or not allocated.\n> ");
        }
        screen_swap();
    }
    else if (compare_string(input, "free2") == 0) {
        if (ptr2 != NULL) {
            mem_free(ptr2);
            print_string("mem_free(ptr2)\n");
            print_dynamic_mem();
            print_nl();
            ptr2 = NULL;
        } else {
            print_string("ptr2 is already free or not allocated.\n> ");
        }
        screen_swap();
    }
    else if (compare_string(input, "free3") == 0) {
        if (ptr3 != NULL) {
            mem_free(ptr3);
            print_string("mem_free(ptr3)\n");
            print_dynamic_mem();
            print_nl();
            ptr3 = NULL;
        } else {
            print_string("ptr3 is already free or not allocated.\n> ");
        }
        screen_swap();
    }
    else if (compare_string(input, "") == 0) {
        print_string("\n> ");
        screen_swap();
    }
    else {
        print_string("Unknown command: ");
        print_string(input);
        print_string("\n> ");
        screen_swap();
    }
}

void start_kernel() {
    idt_init();
    isr_init();
    fpu_init();
    irq_init();
    screen_init();
    timer_init();
    keyboard_init();
    mouse_init();
    asm volatile("sti");
    init_dynamic_mem();

    clear_screen(COLOR(0, 0, 0));
    print_logo();
    screen_swap();

    mouse();

    print_string("> ");
    screen_swap();
    while (1) {
        for (int sc = 0; sc < 128; sc++) {
            if (keyboard_char(sc)) {
                char c = (char)sc;

                if (c == '\n' || c == '\r') {
                    input_buffer[input_len] = '\0';
                    print_string("\n");
                    execute_command(input_buffer);
                    input_len = 0;
                }
                else if (c == '\b') {
                    if (input_len > 0) {
                        input_len--;
                        print_string("\b \b");
                    }
                }
                else {
                    if (input_len < INPUT_BUFFER_SIZE - 1) {
                        input_buffer[input_len++] = c;
                        char str[2] = {c, '\0'};
                        print_string(str);
                    }
                }
                keyboard.chars[(uint8_t)c] = false;
            }
        }
            screen_swap();
    }
}
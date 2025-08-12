#include "../include/idt.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/timer.h"
#include "../include/display.h"
#include "../include/keyboard.h"
#include "../include/fpu.h"
#include <stddef.h>
#include <stdint.h>
#include "../include/utils.h"
#include "../include/memory.h"
#include "../include/kernel.h"
#include "../include/mouse.h"

#define LOGO_HEIGHT 5
static const char *LOGO[LOGO_HEIGHT] = {
    "==============================",
    "= GlacierOS                  =",
    "= Made by GlitchDetected     =",
    "=                            =",
    "==============================",
};

bool screen_dirty = false;

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
    print_string("> ");
    screen_swap();
    while (1) {
        bool input_processed = false;
        for (int sc = 0; sc < 128; sc++) {
            if (keyboard_char(sc)) {
                char c = (char)sc;

                if (c == '\n' || c == '\r') {
                    input_buffer[input_len] = '\0';
                    print_string("\n");
                    execute_command(input_buffer);
                    input_len = 0;
                    screen_dirty = true;
                }
                else if (c == '\b') {
                    if (input_len > 0) {
                        input_len--;
                        print_string("\b \b");
                        screen_dirty = true;
                    }
                }
                else {
                    if (input_len < INPUT_BUFFER_SIZE - 1) {
                        input_buffer[input_len++] = c;
                        char str[2] = {c, '\0'};
                        print_string(str);
                        screen_dirty = true;
                    }
                }
                keyboard.chars[(uint8_t)c] = false;
                input_processed = true;
            }
        }

        if (screen_dirty) {
            screen_swap();
            screen_dirty = false;
        }

        if (!input_processed) {
            asm volatile("hlt");
        }
    }
}

void execute_command(char *input) {
    if (compare_string(input, "EXIT") == 0) {
        print_string("Stopping the CPU processes... \n");
        screen_swap();
        asm volatile("hlt");
    }
    else if (compare_string(input, "ping") == 0) {
        print_string("pong\n> ");
        print_nl();
        screen_swap();
    }
    else if (compare_string(input, "meminfo") == 0) {
        print_string("init_dynamic_mem()\n");
        print_dynamic_node_size();
        print_dynamic_mem();
        print_nl();
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
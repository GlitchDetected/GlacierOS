#include "mouse.h"
#include "../cpu/irq.h"
#include "../utils.h"
#include "display.h"

#define CURSOR_WIDTH 11
#define CURSOR_HEIGHT 16
static int mouse_x = SCREEN_WIDTH / 2;
static int mouse_y = SCREEN_HEIGHT / 2;
static int mouse_x_position = -1;
static int mouse_y_position = -1;

static u8 mouse_packet[3];
static int mouse_cycle = 0;

short mouse_icon[] =  {
    1,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,
    1,2,2,2,1,0,0,0,0,0,0,
    1,2,2,2,2,1,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,
    1,2,2,2,2,2,2,1,0,0,0,
    1,2,2,2,2,2,2,2,1,0,0,
    1,2,2,2,2,2,2,2,2,1,0,
    1,2,2,2,2,1,1,1,1,1,1,
    1,2,2,2,1,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,0,0,0,0,0,0,
};
u32 mouse_color_mapping[] = {0, 0xFFFFFFFF, 0};

static u32 background_buffer[CURSOR_WIDTH * CURSOR_HEIGHT];

void mouse_handler(struct Registers *regs) {
    u8 data = inportb(0x60);

    mouse_packet[mouse_cycle++] = data;
    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        int dx = (i8) mouse_packet[1];
        int dy = (i8) mouse_packet[2];

        mouse_x += dx;
        mouse_y -= dy;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x >= SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 1;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y >= SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 1;
    }
}

void mouse_init() {
    irq_install(12, mouse_handler);
}

void mouse() {
    if (mouse_x_position >= 0 && mouse_y_position >= 0) {
        for (int y = 0; y < CURSOR_HEIGHT; y++) {
            for (int x = 0; x < CURSOR_WIDTH; x++) {
                int px = mouse_x_position + x;
                int py = mouse_y_position + y;
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    screen_set(background_buffer[y * CURSOR_WIDTH + x], px, py);
                }
            }
        }
    }

    // Save bg at new position
    for (int y = 0; y < CURSOR_HEIGHT; y++) {
        for (int x = 0; x < CURSOR_WIDTH; x++) {
            int px = mouse_x + x;
            int py = mouse_y + y;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                background_buffer[y * CURSOR_WIDTH + x] = screen_offset(px, py);
            } else {
                background_buffer[y * CURSOR_WIDTH + x] = 0; // Out of bounds, save 0
            }
        }
    }

    // Draw cursor icon
    for (int y = 0; y < CURSOR_HEIGHT; y++) {
        for (int x = 0; x < CURSOR_WIDTH; x++) {
            short pixel = mouse_icon[y * CURSOR_WIDTH + x];
            if (pixel != 0) {
                int draw_x = mouse_x + x;
                int draw_y = mouse_y + y;
                if (draw_x >= 0 && draw_x < SCREEN_WIDTH && draw_y >= 0 && draw_y < SCREEN_HEIGHT) {
                    u32 color = mouse_color_mapping[pixel];
                    screen_set(color, draw_x, draw_y);
                }
            }
        }
    }

    mouse_x_position = mouse_x;
    mouse_y_position = mouse_y;
}

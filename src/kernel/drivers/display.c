#include "../../headers/display.h"
#include "../../headers/font.h"
#include "../../headers/memory.h"
#include "../../headers/strings.h"
#include <stdint.h>
#include "../../headers/x86.h"

static size_t cursor_x = 0;
static size_t cursor_y = 0;
static uint8_t *BUFFER = (uint8_t *) 0xA0000;

uint8_t _sbuffers[2][SCREEN_SIZE];
uint8_t _sback = 0;

#define CURRENT (_sbuffers[_sback])
#define SWAP() (_sback = 1 - _sback)

#define PALETTE_MASK 0x3C6
#define PALETTE_READ 0x3C7
#define PALETTE_WRITE 0x3C8
#define PALETTE_DATA 0x3C9

void set_cursor(int x, int y) {
    cursor_x = x * CHAR_W; // convert column to pixel position horizontally
    cursor_y = y * CHAR_H; // convert row to pixel position vertically
}

int get_cursor() {
    return cursor_y * SCREEN_WIDTH + cursor_x;
}

int get_offset(int col, int row) {
    return 2 * (row * MAX_COLS + col);
}

int get_row_from_offset(int offset) {
    return offset / (2 * MAX_COLS);
}

int move_offset_to_new_line(int offset) {
    return get_offset(0, get_row_from_offset(offset) + 1);
}

void set_char_at_video_memory(char character, int offset) {
    uint8_t *vidmem = (uint8_t *) VIDEO_ADDRESS;
    vidmem[offset] = character;
    vidmem[offset + 1] = WHITE_ON_BLACK;
}

int scroll_ln(void) {
    memcpy(
        (uint8_t *)(get_offset(0, 1) + VIDEO_ADDRESS),
        (uint8_t *)(get_offset(0, 0) + VIDEO_ADDRESS),
        MAX_COLS * (MAX_ROWS - 1) * 2
    );

    for (int col = 0; col < MAX_COLS; col++) {
        set_char_at_video_memory(' ', get_offset(col, MAX_ROWS - 1));
    }

    return 0;
}

void print_string(const char *string) {
    int i = 0;

    while (string[i] != 0) {
        if (string[i] == '\n') {
            cursor_x = 0;
            cursor_y += CHAR_H;
            int row = cursor_y / CHAR_H;
            if (row >= MAX_ROWS) {
                scroll_ln();
                cursor_y = (MAX_ROWS - 1) * CHAR_H;
            }
        } else {
            uint8_t color = WHITE_ON_BLACK;

            font_char(string[i], cursor_x, cursor_y, color);
            cursor_x += CHAR_W;
            if (cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y += CHAR_H;
                if (cursor_y >= SCREEN_HEIGHT) {
                    scroll_ln();
                    cursor_y = SCREEN_HEIGHT - CHAR_H;
                }
            }
        }
        i++;
    }
}

void print_nl() {
    cursor_x = 0;
    cursor_y += CHAR_H;
    int row = cursor_y / CHAR_H;
    if (row >= MAX_ROWS) {
        scroll_ln();
        cursor_y = (MAX_ROWS - 1) * CHAR_H;
    }
}

void clear_screen(uint8_t color) {
    memset(CURRENT, color, SCREEN_SIZE);
}

void print_backspace() {
    if (cursor_x >= CHAR_W) {
        cursor_x -= CHAR_W;
    } else if (cursor_y >= CHAR_H) {
        cursor_y -= CHAR_H;
        cursor_x = SCREEN_WIDTH - CHAR_W;
    }
    font_char(' ', cursor_x, cursor_y, 0);
}

void screen_swap() {
    memcpy(BUFFER, CURRENT, SCREEN_SIZE);
    SWAP();
}

void screen_init() {
    outp(PALETTE_MASK, 0xFF);
    outp(PALETTE_WRITE, 0);
    for (uint8_t i = 0; i < 255; i++) {
        outp(PALETTE_DATA, (((i >> 5) & 0x7) * (256 / 8)) / 4);
        outp(PALETTE_DATA, (((i >> 2) & 0x7) * (256 / 8)) / 4);
        outp(PALETTE_DATA, (((i >> 0) & 0x3) * (256 / 4)) / 4);
    }

    // set color 255 = white
    outp(PALETTE_DATA, 0x3F);
    outp(PALETTE_DATA, 0x3F);
    outp(PALETTE_DATA, 0x3F);
}
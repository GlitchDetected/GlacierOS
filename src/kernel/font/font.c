#include "../../headers/font.h"
#include "../../headers/display.h"
#include "../../headers/system.h"
#include "../../font8x8/font8x8_basic.h"
#include <stdint.h>

void font_char(char c, size_t x, size_t y, uint8_t color) {
    assert(c >= 0, "INVALID CHARACTER");

    const uint8_t *glyph = font8x8_basic[(size_t)c];

    for (size_t yy = 0; yy < 8; yy++) {
        for (size_t xx = 0; xx < 8; xx++) {
            if (glyph[yy] & (1 << xx)) {
                screen_set(color, x + xx, y + yy);
            }
        }
    }
}

void font_str(const char *s, size_t x, size_t y, uint8_t color) {
    char c;

    while ((c = *s++) != 0) {
        font_char(c, x, y, color);
        x += 8;
    }
}

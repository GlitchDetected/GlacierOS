#include "../../headers/system.h"
#include "../../headers/display.h"
#include "../../headers/font.h"
#include "../../headers/strings.h"

static uint32_t rseed = 1;

void seed(uint32_t s) {
    rseed = s;
}

void panic(const char *err) {
    clear_screen(COLOR(7, 0, 0));

    if (err != NULL) {
        font_str(err, (SCREEN_WIDTH - font_width(err)) / 2, SCREEN_HEIGHT / 2 - 4, COLOR(7, 7, 3));
    }

    screen_swap();
    for (;;) {}
}
#ifndef FONT_H
#define FONT_H

#include "display.h"
#include <stdint.h>
#include "types.h"

#define font_width(_s) (strlen((_s)) * 8)
#define font_height() (8)
#define font_str_doubled(_s, _x, _y, _c) do {\
const char *__s = (_s);\
size_t __x = (_x);\
size_t __y = (_y);\
uint8_t __c = (_c);\
font_str(__s, __x + 1, __y + 1, COLOR_ADD(__c, -2));\
font_str(__s, __x, __y, __c);\
} while (0)

void font_char(char c, size_t x, size_t y, uint8_t color);
void font_str(const char *s, size_t x, size_t y, uint8_t color);

#endif

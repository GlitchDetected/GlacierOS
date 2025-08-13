#ifndef __LOGS_H
#define __LOGS_H

#include <stdint.h>

#define MESSAGE_MOUSE_MOVE       1
#define MESSAGE_MOUSE_PRESS      2
#define MESSAGE_MOUSE_RELEASE    3
#define MESSAGE_KEY_PRESS        4
#define MESSAGE_KEY_RELEASE      5
#define MESSAGE_WINDOW_DRAG      6

typedef struct {
    uint16_t    message;
    uint16_t    handle;
    uint16_t    x;
    uint16_t    y;
    uint32_t    key;
} message_t;

#endif
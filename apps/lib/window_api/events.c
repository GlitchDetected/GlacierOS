#include "../../headers/window_api/events.h"
#include "../../headers/os_api.h"
#include "../../headers/system_calls.h"
#include "../../headers/stdbool.h"
#include "../../headers/stdint.h"

void window_send_message(window_t* win, message_t* msg) {
    syscall(SYSCall_messaging_create, msg, win->handle);
}

void window_send_message_simple(window_t* win, int message) {
    message_t msg;
    msg.message = message;
    window_send_message(win, &msg);
}

bool window_get_message(window_t* win, message_t* msg) {
    return syscall(SYSCall_messaging_get, msg, win->handle);
}

bool window_peek_message(window_t* win, message_t* msg) {
    return syscall(SYSCall_messaging_peek, msg, win->handle);
}
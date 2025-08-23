#ifndef __SERIAL_H
#define __SERIAL_H
#include <stdbool.h>

void serial_init(void);
void serial_write_com(int, unsigned char);
void init_serial();

bool serial_recieve_buffer_empty(void);

char serial_getchar(void);

bool serial_transmit_buffer_empty(void);

void serial_write_char(char a);

void serial_write(const char* str);

#endif

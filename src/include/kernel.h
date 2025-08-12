#ifndef KERNEL_H
#define KERNEL_H

void execute_command(char *input);

#define INPUT_BUFFER_SIZE 128
char input_buffer[INPUT_BUFFER_SIZE];
int input_len = 0;

static int *ptr1 = NULL;
static int *ptr2 = NULL;
static int *ptr3 = NULL;
#define LOGO_HEIGHT 5

#endif
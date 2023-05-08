#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define SERPAEOS_MAX_PATH 108
#include <stdbool.h>


void kernel_main();

void panic(const char* msg);
void kernel_page();
void kernel_registers();

unsigned char terminal_get_attr();
void terminal_cursor_reverse();

struct configeration
{
    unsigned char magic[4]; //S, O, S, 0x3E
    int timezone;
    char username[21];
    char password[31];
}__attribute__((packed));
#define config struct configeration

config *get_config();
void save_config();

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
#endif
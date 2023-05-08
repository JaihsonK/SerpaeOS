#include "graphics.h"
#include "memory/memory.h"
#include "string/string.h"
#include "io/io.h"
#include "sound/sound.h"
#include <stdint.h>

extern bool speaker_on;

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

static uint8_t get_attribute(uint16_t x, uint16_t y)
{
    uint8_t *tmp_video_mem = (uint8_t *)0xb8000;
    return tmp_video_mem[1 + (((y * VGA_WIDTH) + x) * 2)];
}

uint8_t terminal_get_attr()
{
    return get_attribute(terminal_col, terminal_row);
}

uint16_t terminal_make_char(char c, char colour)
{
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}
void terminal_backspace(bool white)
{
    if (terminal_row == 0 && terminal_col == 0)
    {
        return;
    }

    if (terminal_col <= 0)
    {
        terminal_row -= 1;
        terminal_col = VGA_WIDTH - 1;
        terminal_writechar(' ', 15);
        terminal_row -= 1;
        terminal_col = VGA_WIDTH - 1;
    }
    else
    {
        terminal_col -= 1;
        terminal_writechar(' ', 15);
        terminal_col -= 1;
    }
}

void scroll_down()
{
    for (int i = 1; i < VGA_HEIGHT; i++)
        memcpy(&video_mem[(i - 1) * 80], &video_mem[i * 80], VGA_WIDTH * sizeof(uint16_t));
    for (int i = 0; i < VGA_WIDTH; i++)
        terminal_putchar(i, 24, ' ', get_attribute(terminal_col, terminal_row));
}

void terminal_writechar(char c, char colour)
{
    if ((c == '\n' || c == '\r') && terminal_row < VGA_HEIGHT)
    {
        if (terminal_row == 24) // last row
            scroll_down();
        else
            terminal_row += 1;
        terminal_col = 0;
        return;
    }
    if (c == '\f' && terminal_row < VGA_HEIGHT)
    {
        if (terminal_row == 24) // last row
            scroll_down();
        else
            terminal_row += 1;
        return;
    }
    if(c == '\a')
    {
        beep();
        return;
    }
    if (c == '\b')
    {
        terminal_backspace(true);
        return;
    }
    if (c == '\t')
    {
        terminal_col = (terminal_col == 75)? 79 : terminal_col + 4;
    }


    if (terminal_col == 79 && terminal_row == 24)
        scroll_down();

    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1;
    if (terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row += 1;
    }
}

void terminal_initialize()
{
    video_mem = (uint16_t *)(0xB8000);
    terminal_row = 0;
    terminal_col = 0;
    clrscr(15);
}

uint16_t *get_cursor()
{
    static uint16_t res[2];
    res[0] = terminal_col;
    res[1] = terminal_row;
    return res;
}
void set_cursor(uint16_t x, uint16_t y)
{
    if(x >= VGA_WIDTH)
        x = VGA_WIDTH - 1;
    if(y >= VGA_HEIGHT)
        y = VGA_HEIGHT - 1;
    terminal_col = x;
    terminal_row = y;
}

void graphics_put(char c, int x, int y)
{
    char *tmp_video_mem = (char *)0xb8000;
    tmp_video_mem[1 + (((y * VGA_WIDTH) + x) * 2)] = c;
}

void graphics_draw_line(char c, int y)
{
    for (int i = 0; i < VGA_WIDTH; i++)
        graphics_put(c, i, y);
}

void graphics_draw_collumn(char c, int x)
{
    for (int i = 0; i < VGA_HEIGHT; i++)
        graphics_put(c, x, i);
}

void clrscr(char c)
{
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        graphics_draw_line(c, y);
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x, y, ' ', (char)get_attribute(terminal_col, terminal_row));
        }
    }
    set_cursor(0, 0);
}

void print(const char *str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
        terminal_writechar(str[i], get_attribute(terminal_col, terminal_row));
}

/*EXPERIMENTAL GRAPHICS MODE*/
//Thanks to SaphireOS



static struct VBE_info_block *VBE = VBE_ADDRESS;

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    r /= 3;
    g /= 2;
    b /= 3;

    return (r << 11) | (g << 5) | b;
}

void draw_pixel(int x, int y, uint16_t colour)
{
    unsigned offset = y * VBE->x_resolution + x;
    *((uint16_t *)VBE->screen_ptr + offset) = colour;
}

void clear_screen(uint16_t colour)
{
    for(int y = 0; y < VBE->y_resolution; y++)
    {
        for(int x = 0; x < VBE->x_resolution; x++)
        {
            draw_pixel(x, y, colour);
        }
    }
}
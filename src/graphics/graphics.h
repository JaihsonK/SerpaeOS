#ifndef GRAPHICS_H
#define GRAPHICS_H

#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#include <stdbool.h>

void terminal_backspace(bool white);
void clrscr(char c);
void print(const char *str);
void terminal_writechar(char c, char colour);
unsigned short terminal_make_char(char c, char colour);
void set_cursor(unsigned short x, unsigned short y);
unsigned short *get_cursor();
unsigned char terminal_get_attr();

void graphics_put(char c, int x, int y);
void graphics_draw_line(char c, int y);
void graphics_draw_collumn(char c, int x);

#define colour(forg, back) ((back << 4) | forg)

//text mode
enum VGAcolours
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GREY,
    DARK_GREY = 8,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE
};


/*EXPERIMENTAL GRAPHICS MODE*/
//Thanks to SaphireOS


struct VBE_info_block
{
    unsigned short mode_attribute;
    unsigned char win_a_attribute;
    unsigned char win_b_attribute;
    unsigned short win_granuality;
    unsigned short win_size;
    unsigned short win_a_segment;
    unsigned short win_b_segment;
    unsigned int win_func_ptr;
    unsigned short bytes_per_scan_line;
    unsigned short x_resolution;
    unsigned short y_resolution;
    unsigned char char_x_size;
    unsigned char char_y_size;
    unsigned char number_of_planes;
    unsigned char bits_per_pixel;
    unsigned char number_of_banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char number_of_image_pages;
    unsigned char b_reserved;
    unsigned char red_mask_size;
    unsigned char red_field_position;
    unsigned char green_mask_size;
    unsigned char green_field_position;
    unsigned char blue_mask_size;
    unsigned char blue_field_position;
    unsigned char reserved_mask_size;
    unsigned char reserved_field_position;
    unsigned char direct_color_info;
    unsigned int screen_ptr;
}__attribute__((packed));

#define VBE_ADDRESS (struct VBE_info_block *)0x400

#endif
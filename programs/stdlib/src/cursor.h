#ifndef CURSOR_H
#define CURSOR_H

#define colour(forg, back) ((back << 4) | forg)

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
    SKY_BLUE = LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE
};

void set_cursor(unsigned short x, unsigned short y);
void draw_block(unsigned char colour, unsigned short x, unsigned short y);
void draw_row(unsigned char colour, unsigned short row);
void draw_collumn(unsigned char colour, unsigned short collumn);

#endif
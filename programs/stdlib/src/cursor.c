#include "cursor.h"

#define uint8_t unsigned char
#define uint16_t unsigned short

#define VGAWIDTH 80
#define VGAHEIGHT 25

void draw_row(uint8_t colour, uint16_t row)
{
    for(int i = 0; i < VGAWIDTH; i++)
        draw_block(colour, i, row);
}

void draw_collumn(uint8_t colour, uint16_t collumn)
{
    for(int i = 0; i < VGAHEIGHT; i++)
        draw_block(colour, collumn, i);
}


#include "sound.h"
#include <stdint.h>
#include <stdbool.h>
#include "io/io.h"
#include "../idt/idt.h"


void play(uint32_t hertz)
{
    
    uint32_t div = 1193180 / hertz;
    uint8_t tmp;

    outb(0x43, 0xb6);
    outb(0x42, (uint8_t)div);
    outb(0x42, (uint8_t)(div >> 8));

    tmp = inb(0x61);
    if(tmp != (tmp | 3))
    {
        outb(0x61, tmp | 3);
    }
}

void quit_sound()
{

    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void beep()
{
    play(440);
    for(int i = 0; i < (50000000 * 3); i++);
    quit_sound();
}

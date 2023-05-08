#include "classic.h"
#include "keyboard.h"
#include "io/io.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"
#include "task/process.h"

#include <stdint.h>
#include <stddef.h>

bool shift_pressed = false;
bool caps_pressed = false;
bool escape_pressed = false;
bool backspace_pressed = false;
bool alt_pressed = false;
bool ctrl_pressed = false;
bool enter_pressed = false;
bool dir_on = false;
bool superkey_pressed = false;
bool superkey_was_used = false;
bool e0_check = false;


int classic_keyboard_init();

struct keyboard classic_keyboard = {
    .name = {"Classic"},
    .init = classic_keyboard_init
};


void classic_keyboard_handle_interrupt();

int classic_keyboard_init()
{
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, classic_keyboard_handle_interrupt);

    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    return 0;
}

unsigned char classic_keyboard_scancode_to_char(unsigned char scancode) 
{
    if(scancode == 0xE0)
    {
        e0_check = true;
        return 0;
    }
    if (scancode ==  0x01)
    {
        //escape_pressed = true;
        return '\e';
    }        
    if (scancode ==  0x02)
        if (shift_pressed == true)
            return '!';
        else 
            return '1';
        
    else if (scancode ==  0x03)
        if (shift_pressed == true)
            return '@';
        else 
            return '2';
        
    else if (scancode ==  0x04)
        if (shift_pressed == true)
            return '#';
        else 
            return '3';
        
    else if (scancode ==  0x05)
        if (shift_pressed == true)
            return '$';
        else 
            return '4';
            
    else if (scancode == 0x06)
        if (shift_pressed == true)
            return '%';
        else 
            return '5';
        
    else if (scancode == 0x07)
        if (shift_pressed == true)
            return '^';
        else 
            return '6';
        
    else if (scancode == 0x08)
        if (shift_pressed == true)
            return '&';
        else 
            return '7';
        
    else if (scancode == 0x09)
        if (shift_pressed == true)
            return '*';
        else 
            return '8';
        
    else if (scancode == 0x0A)
        if (shift_pressed == true)
            return '(';
        else 
            return '9';
        
    else if (scancode == 0x0B)
        if (shift_pressed == true)
            return ')';
        else 
            return '0';
        
    else if (scancode == 0x0C)
        if (shift_pressed == true)
            return '_';
        else 
            return '-';
            
    else if (scancode == 0x0D)
        if (shift_pressed == true)
            return '+';
        else 
            return '=';
        
    // Backspace
    else if (scancode == 0x0E)
    {
        backspace_pressed = true;
        return '\b';
    }

        
        
    else if (scancode == 0x0F)
        return '\t';
        
    else if (scancode == 0x10)
        if(ctrl_pressed && !shift_pressed)
        {
            outb(0x20, 0x20);
            process_terminate(process_current());
            return 0;
        }
        else if (shift_pressed == true || caps_pressed == true)
        {
            return 'Q';
        }
        else 
            return 'q';
        
    else if (scancode == 0x11)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'W';
        }
        else 
            return 'w';
        
    else if (scancode == 0x12)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'E';
        }
        else 
            return 'e';
        
    else if (scancode == 0x13)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'R';
        }
        else 
            return 'r';
        
    else if (scancode == 0x14)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'T';
        }
        else 
            return 't';
        
    else if (scancode == 0x15)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'Y';
        }
        else 
            return 'y';
        
    else if (scancode == 0x16)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'U';
        }
        else 
            return 'u';
        
    else if (scancode == 0x17)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'I';
        }
        else 
            return 'i';
        
    else if (scancode == 0x18)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'O';
        }
        else 
            return 'o';
        
    else if (scancode == 0x19)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'P';
        }
        else 
            return 'p';
        
    else if (scancode == 0x1A)
        if (shift_pressed == true)
            return '{';
        else 
            return '[';
        
    else if (scancode == 0x1B)
        if (shift_pressed == true)
            return '}';
        else 
            return ']';
        
    // enter pressed
    else if (scancode == 0x1C) {
        enter_pressed = true;
        return '\r';
    }
        
        
    // ctrl pressed
    else if (scancode == 0x1D)
    {
        ctrl_pressed = true;
        return 0;
    }
    else if(scancode == 0x9D)
    {
        ctrl_pressed = false;
        return 0;
    }
        
        
    else if (scancode == 0x1E)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'A';
        }
        else 
            return 'a';
        
    else if (scancode == 0x1F)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'S';
        }
        else 
            return 's';
        
    else if (scancode == 0x20)
        if(ctrl_pressed && !shift_pressed)
            return EOF;
        else if (shift_pressed == true || caps_pressed == true)
        {
            return 'D';
        }
        else 
            return 'd';
        
    else if (scancode == 0x21)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'F';
        }
        else 
            return 'f';
        
    else if (scancode == 0x22)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'G';
        }
        else 
            return 'g';
        
    else if (scancode == 0x23)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'H';
        }
        else 
            return 'h';
        
    else if (scancode == 0x24)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'J';
        }
        else 
            return 'j';
        
    else if (scancode == 0x25)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'K';
        }
        else 
            return 'k';
        
    else if (scancode == 0x26)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'L';
        }
        else 
            return 'l';
        
    else if (scancode == 0x27)
        if (shift_pressed == true)
            return ':';
        else 
            return ';';
        
    else if (scancode == 0x28) 
        if (shift_pressed == true)
            return '|';
        else 
            return '\'';
        
    else if (scancode == 0x29)
        if (shift_pressed == true)
            return '~';
        else 
            return '`';
        
    // shift pressed
    else if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return 0;
    }
        
        
    else if (scancode == 0x2B)
        if (shift_pressed == true)
            return '|';
        else 
            return '\\';
    
    else if (scancode == 0x2C)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'Z';
        }
        else 
            return 'z';
    
    else if (scancode == 0x2D)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'X';
        }
        else 
            return 'x';
        
    else if (scancode == 0x2E)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'C';
        }
        else 
            return 'c';
        
    else if (scancode == 0x2F)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'V';
        }
        else 
            return 'v';
        
    else if (scancode == 0x30)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'B';
        }
        else 
            return 'b';
        
    else if (scancode == 0x31)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'N';
        }
        else 
            return 'n';
        
    else if (scancode == 0x32)
        if (shift_pressed == true || caps_pressed == true)
        {
            return 'M';
        }
        else 
            return 'm';
        
    else if (scancode == 0x33)
        if (shift_pressed == true)
            return '<';
        else 
            return ',';
        
    else if (scancode == 0x34)
        if (shift_pressed == true)
            return '>';
        else 
            return '.';
    
    else if (scancode == 0x35)
        if (shift_pressed == true)
            return '?';
        else 
            return '/';

    // shift pressed    
    else if (scancode == 0x36 || scancode == 0x2A) {
        shift_pressed = true;
        scancode = 0;
    }
        

    // alt pressed
    else if (scancode == 0x38)
        alt_pressed = true;
        
        
    else if (scancode == 0x39)
        return ' ';
    
    // Caps pressed
    else if (scancode == 0x3A) {
        caps_pressed = !caps_pressed;
        return 0;
    }
        

    // shift released
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
        return 0;
    }

    //function keys
    if(scancode == 0x3B)
    {
        outb(0x20, 0x20);
        process_flip(1);
        return 0;
    }
    else if(scancode == 0x3C)
    {
        outb(0x20, 0x20);
        process_flip(2);
        return 0;
    }
    else if(scancode == 0x3D)
    {
        outb(0x20, 0x20);
        process_flip(3);
        return 0;
    }
    else if(scancode == 0x3E)
    {
        outb(0x20, 0x20);
        process_flip(4);
        return 0;
    }
    else if(scancode == 0x3F)
    {
        outb(0x20, 0x20);
        process_flip(5);
        return 0;
    }
    else if(scancode == 0x40)
    {
        outb(0x20, 0x20);
        process_flip(6);
        return 0;
    }
    else if(scancode == 0x41)
    {
        outb(0x20, 0x20);
        process_flip(7);
        return 0;
    }
    else if(scancode == 0x42)
    {
        outb(0x20, 0x20);
        process_flip(8);
        return 0;
    }
    else if(scancode == 0x43)
    {
        outb(0x20, 0x20);
        process_flip(9);
        return 0;
    }
    else if(scancode == 0x44)
    {
        outb(0x20, 0x20);
        process_flip(10);
        return 0;
    }
    else if(scancode == 0x45)
    {
        outb(0x20, 0x20);
        process_flip(11);
        return 0;
    }
    else if(scancode == 0x46)
    {
        outb(0x20, 0x20);
        process_flip(12);
        return 0;
    }
    else if(scancode == 0x47 && e0_check)
    {
        e0_check = false;
        outb(0x20, 0x20);
        process_flip(0);
        return 0;
    }

    //windows key
    if((scancode == 0x5B || scancode == 0x5C) && e0_check)
    {
        e0_check = false;
        superkey_pressed = true;
        return 0;
    }
    if((scancode == 0xDB || scancode == 0xDC) && e0_check)
    {
        outb(0x20, 0x20);
        e0_check = superkey_pressed = false;
        if(!superkey_was_used)
        {
            dir_on = !dir_on;
            if(!dir_on)
                hide_directory();
            else
                show_directory();
        }
        superkey_was_used = false;
        return 0;
    }

    //keypad
    if(scancode == 0x34)
        return '/';
    else if(scancode == 37)
        return '*';
    else if(scancode == 0x4A)
        return '-';
    else if(scancode == 0x4E)
        return '+';
    else if(scancode == 0x1C)
        return '\r';
    else if(scancode == 0x53)
    {
        return '.';
    }
    else if(scancode == 0x52)
    {
        return '0';
    }
    else if(scancode == 0x4F)
    {
        return '1';
    }
    else if(scancode == 0x50)
    {
        char c;
        if(e0_check)
            c = KEY_DOWN;
        else
            c = '2';
        e0_check = false;
        return c;
    }
    else if(scancode == 0x51)
    {
        return '3';
    }
    else if(scancode == 0x4B)
    {
        char c;
        if(e0_check)
            c = KEY_LEFT;
        else
            c = '4';
        e0_check = false;
        return c;
    }
    else if(scancode == 0x4C)
    {
        return '5';
    }
    else if(scancode == 0x4D)
    {
        char c;
        if(e0_check)
            c = KEY_RIGHT;
        else
            c = '6';
        e0_check = false;
        return c;
    }
    else if(scancode == 0x47)
    {
        return '7';
    }
    else if(scancode == 0x48)
    {
        char c;
        if(e0_check)
            c = KEY_UP;
        else
            c = '8';
        e0_check = false;
        return c;
    }
    else if(scancode == 0x49)
    {
        return '9';
    }
    
    return '\0';
}

void classic_keyboard_handle_interrupt()
{
    kernel_page();
    uint8_t scancode = inb(KEYBOARD_INPUT_PORT);
    inb(KEYBOARD_INPUT_PORT);

    /*if(scancode & CLASSIC_KEYBOARD_KEY_RELEASED)
    {
        return;
    }*/
    
    uint8_t c = classic_keyboard_scancode_to_char(scancode);
    if (c > 0)
    {
        keyboard_push(c);
    }

    task_page();

}

struct keyboard* classic_init()
{
    return &classic_keyboard;
}
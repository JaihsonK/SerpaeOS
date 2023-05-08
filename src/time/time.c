#include "time.h"
#include "../io/io.h"
#include "kernel.h"
#include <stdint.h>

//code from https://wiki.osdev.org/CMOS

int century_register = 0x00;

int TIMEZONE = CENTRAL; //default timezone CST (Winnipeg, MB, Canada)

enum {
    cmos_address = 0x70,
    cmos_data    = 0x71
};

struct date_time now;

int cmod_get_update_in_progress_flag()
{
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}

uint8_t cmos_get_RTC_reg(uint8_t reg)
{
    outb(cmos_address, reg);
    return inb(cmos_data);
}

void cmos_read_RTC()
{
    uint8_t century;
    struct date_time last;
    uint8_t registerB;

#define conditions (last.seconds != now.seconds)\
|| (last.minutes != now.minutes)\
|| (last.hours != now.hours)\
|| (last.day != now.day)\
|| (last.month != now.month)\
|| (last.year != now.year)\
|| (last.century != century)

    
    while(cmod_get_update_in_progress_flag()); //make sure update isn't currently happening

    now.seconds = cmos_get_RTC_reg(0);
    now.minutes = cmos_get_RTC_reg(0x2);
    now.hours = cmos_get_RTC_reg(0x4) + TIMEZONE;
    now.day = cmos_get_RTC_reg(0x7);
    now.month = cmos_get_RTC_reg(0x8);
    now.year = cmos_get_RTC_reg(0x9);
    
    if(century_register != 0)
        century = cmos_get_RTC_reg(century_register);

    do
    {
        last = now;

        while(cmod_get_update_in_progress_flag());

        now.seconds = cmos_get_RTC_reg(0);
        now.minutes = cmos_get_RTC_reg(0x2);
        now.hours = cmos_get_RTC_reg(0x4) + TIMEZONE;
        now.day = cmos_get_RTC_reg(0x7);
        now.month = cmos_get_RTC_reg(0x8);
        now.year = cmos_get_RTC_reg(0x9);    
        if(century_register != 0)
            century = cmos_get_RTC_reg(century_register);    
    } while (conditions);
#undef conditions
    registerB = cmos_get_RTC_reg(0xB);

    if(!(registerB & 0x4))
    {
        now.seconds = (now.seconds & 0xF) + ((now.seconds / 16) * 10);
        now.minutes = (now.minutes & 0x0F) + ((now.minutes / 16) * 10);
        now.hours =(((now.hours & 0x0F) + (((now.hours & 0x70) / 16) * 10) ) | (now.hours & 0x80)) + TIMEZONE;
        now.day = (now.day & 0x0F) + ((now.day / 16) * 10);
        now.month = (now.month & 0x0F) + ((now.month / 16) * 10);
        now.year = (now.year & 0x0F) + ((now.year / 16) * 10);
        if(century_register != 0) {
                now.century = (now.century & 0x0F) + ((now.century / 16) * 10);
        }
    }

    if(!(registerB & 0x02) && (now.hours & 0x80))
        now.hours = (((now.hours & 0x7F) + 12) % 24) + TIMEZONE;
    
    if(century_register != 0)
        now.year += now.century * 100;
    else
    {
        now.year += (RELEASE_YEAR / 100) * 100;
        if(now.year < RELEASE_YEAR) now.year += 100;
    }    
}

struct date_time *get_current_time()
{
    return &now;
}
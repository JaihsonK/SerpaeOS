#ifndef TIME_H
#define TIME_H

struct date_time
{
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char day;
    unsigned char month;
    unsigned int year;
    unsigned char century;
};

#define RELEASE_YEAR 2023 

enum timezones
{
    MIDWAY = -11,
    HAWAII,
    ALASKA,
    PACIFIC,
    PHOENIX,
    MOUNTAIN = -7,
    CENTRAL,
    EASTERN,
    INDIANA = -5,
    PUERTO_RICO_VIRGIN_ISLANDS,
    ARGENTINA,
    BRAZIL = -3,
    CENTRAL_AFRICA = -1,
    UTC,
    CENTRAL_EUROPEAN,
    EASTERN_EUROPEAN,
    EGYPT = 2,
    EAST_AFRICA,
    NEAR_EAST,
    PAKISTAN,
    BANGLADESH,
    VIETNAM,
    CHINA_TAIWAN,
    JAPAN,
    AUSTRALIAN,
    SOLOMON,
    NEW_ZEALAND
};

struct date_time *get_current_time();
void cmos_read_RTC();

#endif
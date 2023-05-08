#ifndef ISO_9660_H
#define ISO_9660_H

typedef unsigned char int8;
typedef signed char sint8;
typedef unsigned short int16_LSB;
typedef unsigned short int16_MSB;
typedef struct
{
    int16_LSB little;
    int16_MSB big;
}__attribute__((packed)) int16_LSB_MSB;
typedef signed short sint16_LSB;
typedef signed short sint16_MSB;
typedef struct
{
    sint16_LSB little;
    sint16_MSB big;
}__attribute__((packed)) sint16_LSB_MSB;
typedef unsigned int int32_LSB;
typedef unsigned int int32_MSB;
typedef struct
{
    int32_LSB little;
    int32_MSB big;
}__attribute__((packed)) int32_LSB_MSB;
typedef signed int sint32_LSB;
typedef signed int sint32_MSB;
typedef struct
{
    sint32_LSB little;
    sint32_MSB big;
}__attribute__((packed)) sint32_LSB_MSB;

typedef struct
{
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char hundredths[2];
    int8 timezone;
}__attribute__((packed)) datetime_f;

typedef struct 
{
    int8 years; //years since 1900
    int8 month;
    int8 day;
    int8 hour;
    int8 minute;
    int8 second;
    int8 timezone;
}__attribute__((packed)) datetime_t;

#endif
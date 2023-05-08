#ifndef POWER_H
#define POWER_H
#include <stdint.h>

struct RSDPtr
{
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t _OEMID[6];
    uint8_t revision;
    uint32_t rsdtAddress;
} __attribute__((packed));

struct FACP
{
    uint8_t signature[8];
    uint32_t length;
    uint8_t unneded1[40-8];
    uint32_t* DSDT;
    uint8_t unneded2[48-44];
    uint32_t* SMI_CMD;
    uint8_t ACPI_Enable;
    uint8_t ACPI_Disable;
    uint8_t unneded3[64-54];
    uint32_t* PM1a_CNT_BLK;
    uint32_t* PM1b_CNT_BLK;
    uint8_t unneded4[89-72];
    uint8_t PM1_CNT_LEN;
}__attribute__((packed));

typedef int ACPI_VER;
#define ACPI_VER_1 1
#define ACPI_VER_2 2

void acpi_poweroff();
int init_acpi();
int acpi_enable();

#endif
#include "power.h"
#include "memory/memory.h"
#include "graphics/graphics.h"
#include "kernel.h" //panic
#include "io/io.h"
#include "status.h"
//ACPI stuff

static uint32_t *SMI_CMD;
static uint8_t ACPI_ENABLE, ACPI_DISABLE;
static uint32_t *PM1a_CNT;
static uint32_t *PM1b_CNT;
static uint16_t SLP_TYPa;
static uint16_t SLP_TYPb;
static uint16_t SLP_EN, SCI_EN;
static uint8_t PM1_CNT_LEN;

static ACPI_VER acpi_version;

unsigned int* acpi_check_RSDPtr(unsigned int* ptr)
{
    char *sig = "RSD PTR ";
    struct RSDPtr* rsdp = (struct RSDPtr*) ptr;
    char* bptr;
    char check = 0;
    int i;

    if(memcmp(sig, rsdp, 8) == 0)
    {
        bptr = (char*) ptr;
        for(i = 0; i < sizeof(struct RSDPtr); i++)
        {
            check += *bptr;
            bptr++;
        }
        if(check == 0)
        {
            if(rsdp->revision == 0)
                acpi_version = ACPI_VER_1;
            else 
                acpi_version = ACPI_VER_2;
            return (unsigned int *) rsdp->rsdtAddress;
        }
    }
    return NULL;
}

unsigned int *acpi_get_RSDPtr()
{
    unsigned int* addr;
    unsigned int* rsdp;

    for(addr = (unsigned int*) 0xE0000; (int) addr < 0x100000; addr += 0x10/sizeof(addr))
    {
        rsdp = acpi_check_RSDPtr((void*)addr);
        if(rsdp != NULL)
            return rsdp;
    }

    int ebda = *((short*)0x40E);
    ebda = ebda*0x10 & 0xFFFFF;

    for(addr = (unsigned int *)ebda; (int) addr < ebda + 1024; addr += 0x10 / sizeof(addr))
    {
        rsdp = acpi_check_RSDPtr((void*)addr);
        if(rsdp != NULL)    
            return rsdp;
    }
    return NULL;
}

int acpi_check_header(unsigned int* ptr, char *sig)
{
    if(memcmp(ptr, sig, 4) == 0)
    {
        char* checkptr = (char*)ptr;
        int len = *(ptr + 1);
        char check = 0;
        while(0 < len--)
        {
            check += *checkptr;
            checkptr++;
        }
        if(check == 0)
            return 0;
    }
    return -EINVARG;
}

int acpi_enable()
{
    if((inw((unsigned int)PM1a_CNT) & SCI_EN) != 0)
        return 0; //already enabled
    if(SMI_CMD == 0 || ACPI_ENABLE == 0)
        return -ECANNOTDO; //no known way to enable acpi
    outb((uint32_t)(*SMI_CMD), ACPI_ENABLE);
    int i;
    while((inw((unsigned int)PM1b_CNT) & SCI_EN) != 1)
    {
        i++;
        if(i > 0x800)
            return -ETIMEDOUT;
    }
    return 0;
}

int init_acpi()
{
    unsigned int *ptr = acpi_get_RSDPtr();
    if(ptr != NULL && acpi_check_header(ptr, "RSDT") == 0)
    {
        int entrys = *(ptr + 1);
        entrys = (entrys - 36) / 4;
        ptr += 36/4;

        while(0 < entrys--)
        {
            if(acpi_check_header((unsigned int *)(*ptr), "FACP") == 0)
            {
                entrys = -2;
                struct FACP* facp = (struct FACP*)*ptr;
                if(acpi_check_header((void*)facp->DSDT, "DSDT") == 0)
                {
                    char* S5Addr = (char*)facp->DSDT + 36;
                    int dsdt_length = *(facp->DSDT + 1) - 36;
                    while(0 < dsdt_length--)
                    {
                        if(memcmp(S5Addr, "_S5_", 4) == 0)
                            break;
                        S5Addr++;
                    }
                    if(dsdt_length > 0)
                    {
                        if( ( *(S5Addr-1) == 0x08 || ( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') ) && *(S5Addr+4) == 0x12 )
                        {
                            S5Addr += 5;
                            S5Addr += ((*S5Addr & 0xC0)>>6) + 2;
                            if(*S5Addr == 0x0A)
                                S5Addr++;
                            SLP_TYPa = *(S5Addr)<<10;
                            S5Addr++;

                            if(*S5Addr == 0x0A)
                                S5Addr++;
                            SLP_TYPb = *(S5Addr)<<10;
                            
                            SMI_CMD = facp->SMI_CMD;
                            ACPI_ENABLE = facp->ACPI_Enable;
                            ACPI_DISABLE = facp->ACPI_Disable;
                            PM1a_CNT = facp->PM1a_CNT_BLK;
                            PM1b_CNT = facp->PM1b_CNT_BLK;
                            PM1_CNT_LEN = facp->PM1_CNT_LEN;
                            SLP_EN = 1<<13;
                            SCI_EN = 1;

                            return 0;
                        }
                        else
                            print("\n\\_S5 parse error");
                    }
                    else
                        print("\n\\_S5 not present\n");
                }
                else
                    print("\nDSDT invalid\n");
            }
            ptr++;
        }
        print("\nNo FACP present");
    }
    return -ECANNOTDO;
}

void acpi_poweroff()
{
    if(SCI_EN == 0)
        return;

    outw((unsigned int)PM1a_CNT, SLP_TYPa | SLP_EN);
    if(PM1b_CNT != 0)
        outw((unsigned int)PM1b_CNT, SLP_TYPb | SLP_EN);
    
    panic("ACPI poweroff failed...");
}
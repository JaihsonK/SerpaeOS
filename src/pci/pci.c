#include "pci.h"
#include "../io/io.h"
#include <stdint.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

enum header_type
{
    GENERAL,
    PCI_2_PCI,
    PCI_2_CardBus
};  

typedef struct
{
    uint8_t io_space:1;
    uint8_t memory_space:1;
    uint8_t bus_master:1;
    uint8_t special_cycles:1;
    uint8_t mem_write_invalid_enable:1;
    uint8_t VGA_palette_snoop:1;
    uint8_t parity_error:1;
    uint8_t reserved:1;
    uint8_t serr_enabled:1;
    uint8_t fast_backtoback:1;
    uint8_t int_disabled:1;
    uint8_t reserved2:5;
}__attribute__((packed)) command_t;

typedef struct
{
    uint8_t reserved:3;
    uint8_t interrupt_stat:1;
    uint8_t capabilities:1;
    uint8_t capable_66mhz:1;
    uint8_t reserved2:1;
    uint8_t fast_backtoback:1;
    uint8_t master_data_parity:1;
    uint8_t DEVSEL_timing:2;
    uint8_t signaled_target_abort:1;
    uint8_t received_target_abort:1;
    uint8_t received_master_abort:1;
    uint8_t signaled_sys_err:1;
    uint8_t detected_parity:1;
}__attribute__((packed)) status_t;

typedef union
{
    struct 
    {
        uint32_t base_addr:28;
        uint8_t prefetchable:1;
        uint8_t type:2;
        uint8_t zero:1;
    }__attribute__((packed)) memory_bar;
    struct
    {
        uint32_t base_addr:30;
        uint8_t reserved:1;
        uint8_t one:1;
    }__attribute__((packed)) io_bar;
}bar_t;


struct pci_device_header
{
    uint16_t device_id;
    uint16_t vendor_id; //https://pcisig.com/membership/member-companies
    status_t status;
    command_t command;
    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if; //Programming Interface Byte
    uint8_t revision_id;
    uint8_t bist;
    uint8_t header_type;
    uint8_t latency_timer;
    uint8_t cache_line_size;
}__attribute__((packed));

#define nheader(name, contents) struct name {contents} __attribute__((packed))

nheader(pci_header_type_0h, 
    uint16_t device_id;
    uint16_t vendor_id; //https://pcisig.com/membership/member-companies
    status_t status;
    command_t command;
    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if; //Programming Interface Byte
    uint8_t revision_id;
    uint8_t bist;
    uint8_t header_type;
    uint8_t latency_timer;
    uint8_t cache_line_size;
    bar_t bar0;
    bar_t bar1;
    bar_t bar2;
    bar_t bar3;
    bar_t bar4;
    bar_t bar5;
    uint32_t cardbus_cis_ptr;
    uint16_t subsys_id;
    uint16_t subsys_vendor;
    uint32_t expansion_rom_base_adr;

    uint32_t reserved:24;
    uint8_t capabilities_ptr;

    uint32_t reserved_2;
    uint8_t max_latency;
    uint8_t min_grant;
    uint8_t interrupt_pin;
    uint8_t interrupt_line;
    );
//-----------------------------------------------------------------------



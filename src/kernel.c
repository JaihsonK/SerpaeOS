#include "kernel.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "keyboard/keyboard.h"
#include "string/string.h"
#include "isr80h/isr80h.h"
#include "task/task.h"
#include "task/process.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "task/tss.h"
#include "gdt/gdt.h"
#include "io/io.h"
#include "time/time.h"
#include "power/power.h"
#include "graphics/graphics.h"
#include "sound/sound.h"
#include "config.h"
#include "status.h"

/*
SerpaeOS v23.3.1    copyright 2023 Jaihson Kresak
*/

config SOS_config;

int config_file;

const uint8_t config_magic[4] = {'S', 'O', 'S', 0x3E};

extern void terminal_initialize();

struct paging_4gb_chunk *kernel_chunk = 0;

void panic(const char *msg)
{
    print("\nKERNEL PANIC (fatal): ");
    print(msg);
    while (1)
        ;
}

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_chunk);
}

struct tss tss;
struct gdt gdt_real[SERPAEOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[SERPAEOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                 // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},           // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},           // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},           // User data segment
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} // TSS Segment
};

char prog_to_load[25] = "0:/sys/bin/shell.elf";

void kernel_main()
{
    terminal_initialize();

    print("GDT initialization...\n");
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, SERPAEOS_TOTAL_GDT_SEGMENTS);

    // Load the gdt
    gdt_load(gdt_real, sizeof(gdt_real));

    print("Heap initialization...\n");
    // Initialize the heap
    kheap_init();

    print("Filesystems initialization...\n");
    // Initialize filesystems
    fs_init();

    print("Disk initialization...\n");
    // Search and initialize the disks
    disk_search_and_init();

    print("IDT initialization...\n");
    // Initialize the interrupt descriptor table
    idt_init();

    print("TSS initialization");
    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // Load the TSS
    tss_load(0x28);

    print("Paging initialization...\n");
    // Setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Switch to kernel paging chunk
    paging_switch(kernel_chunk);

    // Enable paging
    enable_paging();

    print("ISR initialization...\n");
    // Register the kernel commands
    isr80h_register_commands();

    print("Keyboard initialization...\n");
    // Initialize all the system keyboards
    keyboard_init();

    print("Loading configurations...\n");
    config_file = fopen("0:/sys/config/config.dat", "rw");
    if(!config_file)
    {
        panic("config file inaccesable!!");
    }
    fread(config_file, &SOS_config, sizeof(SOS_config));
    if(memcmp(SOS_config.magic, (void *)config_magic, 4) != 0)
        strcpy(prog_to_load, "0:/sys/bin/new_usr");

    cmos_read_RTC();

    struct process *process = 0;
    int res = process_load_switch(prog_to_load, &process, NULL);
    if (res != SERPAEOS_ALL_OK)
    {
        panic("Failed to startup");
    }

    process->priv = true;

    task_run_first_ever_task();    
}
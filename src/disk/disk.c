#include "disk.h"
#include "io/io.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

#define PRIMARY_BASE 0x1F0
#define SECONDARY_BASE 0x170

//pimary or secondary 
#define pors ((primary_secondary == PRIMARY) ? PRIMARY_BASE : SECONDARY_BASE)

struct partition_entry
{
    uint8_t attribute;
    uint8_t starting_head;
    uint16_t starting_address;
    uint8_t sysID;
    uint8_t ending_head;
    uint16_t ending_address;
    uint32_t lba;
    uint32_t size;
}__attribute__((packed));

struct disk disks[SERPAEOS_MAX_DISKS];

static int ata_identify(int master_slave, int primary_secondary, uint16_t *ptr)
{
    int res = 0;

    outb(pors + 6, (master_slave == MASTER) ? 0xA0 : 0xB0);
    for(uint16_t port = (pors + 2); port <= (pors + 5); port++)
        outb(port, 0);
    outb((pors + 7), 0xEC);

    for(int i = 0; i < 10000; i++); //pause to allow time for the device

    uint8_t status = inb((pors + 7));
    if(status == 0)
    {
        res = -ENODISKSPACE;
        goto out;
    }
    
    do
        status = inb((pors + 7));
    while(status & 0x80);

    uint8_t lba_mid = inb((pors + 4)), lba_hi = inb((pors + 5));
    if(lba_hi == 0 || lba_mid == 0)
        res = PATA;
    else if (lba_hi == 0xEB && lba_mid == 0x14)
        res = PATAPI;
    else if (lba_hi == 0x96 && lba_mid == 0x69)
        res = SATAPI;
    else if (lba_hi == 0xC3 && lba_mid == 0x3C)
        res = SATA;
    else
        res = UNKNOWN_DEV_TYPE;
    
    do
        status = inb((pors + 7));
    while(!(status & 0x08) && !(status & 1));


    for (int i = 0; i < 256; i++)
    {
        *ptr = inw(pors);
        ptr++;
    }
out:
    return res;
}

static void init_disk(struct disk *disk, int pri_sec, int master_slave)
{
    disk->id = (int)(disk - disks);
    
    disk->primary_secondary = pri_sec;
    disk->master_slave = master_slave;

    disk->sector_size = (disk->dev == PATA || disk->dev == SATA) ? SERPAEOS_DEFAULT_SECTOR_SIZE : CD_SECTOR_SIZE;
    disk->filesystem = fs_resolve(disk);
}

static int pata_read_sector(int primary_secondary, bool drive, int lba, int total, void *buf);

void disk_search_and_init()
{

    int disk_offset = 0;

    memset(disks, 0, sizeof(disks));
    
    disks[0].dev = PATA; //SerpaeOS default
    init_disk(&disks[disk_offset], PRIMARY, MASTER); //master

//data needed for the next while
    uint16_t buf[256];
    uint8_t *bootsect = (uint8_t*)buf;
    struct partition_entry *entries = (void*)&bootsect[0x1BE];
    int partitions_used = 0;
    int tmp = UNKNOWN_DEV_TYPE;
//------------------------------

//pimary_slave
    if((disks[++disk_offset].dev = tmp = ata_identify(SLAVE, PRIMARY, buf)) <= 0)
        goto secondary_master; //slave doesn't exist or is invalid

    init_disk(&disks[disk_offset], PRIMARY, SLAVE);
    if(disks[disk_offset].filesystem != 0)
    {
        goto secondary_master; //slave is not partitioned
    }

    disk_read_block(&disks[disk_offset], 0, 1, bootsect);

    partitions_used = 0;
    for(int i = 0; i < 4; i++)
    {
        if(entries[i].size == 0)
            continue; //invalid or unused entry
        disks[i+disk_offset].type = SERPAEOS_DISK_TYPE_PARTITION;
        disks[i+disk_offset].dev = tmp;
        disks[i+disk_offset].offset = entries[i].lba;
        init_disk(&disks[disk_offset+i], PRIMARY, SLAVE);
        if(disks[disk_offset + i].filesystem != 0) //sucessful
            partitions_used++;
        else //failed, unformatted partition
            disks[disk_offset + i].dev = UNKNOWN_DEV_TYPE;
    }
    disk_offset += partitions_used;

secondary_master:
    if((disks[++disk_offset].dev = tmp = ata_identify(MASTER, SECONDARY, buf)) <= 0)
        goto secondary_slave; //master doesn't exist

    init_disk(&disks[disk_offset], SECONDARY, MASTER);
    if(disks[disk_offset].filesystem != 0)
    {
        goto secondary_slave; //master is not partitioned
    }

    disk_read_block(&disks[disk_offset], 0, 1, bootsect);

    partitions_used = 0;
    for(int i = 0; i < 4; i++)
    {
        if(entries[i].size == 0)
            continue; //invalid or unused entry
        disks[i+disk_offset].type = SERPAEOS_DISK_TYPE_PARTITION;
        disks[i+disk_offset].dev = tmp;
        disks[i+disk_offset].offset = entries[i].lba;
        init_disk(&disks[disk_offset+i], SECONDARY, MASTER);
        if(disks[disk_offset + i].filesystem != 0)
            partitions_used++;
        else
            disks[disk_offset + i].dev = UNKNOWN_DEV_TYPE;
    }
    disk_offset += partitions_used;

secondary_slave:
    if((disks[++disk_offset].dev = tmp = ata_identify(SLAVE, SECONDARY, buf)) <= 0)
        goto out; //slave doesn't exist

    init_disk(&disks[disk_offset], SECONDARY, SLAVE);
    if(disks[disk_offset].filesystem != 0)
    {
        goto out; //slave is not partitioned
    }

    disk_read_block(&disks[disk_offset], 0, 1, bootsect);

    partitions_used = 0;
    for(int i = 0; i < 4; i++)
    {
        if(entries[i].size == 0)
            continue; //invalid or unused entry
        disks[i+disk_offset].type = SERPAEOS_DISK_TYPE_PARTITION;
        disks[i+disk_offset].dev = tmp;
        disks[i+disk_offset].offset = entries[i].lba;
        init_disk(&disks[disk_offset+i], SECONDARY, SLAVE);
        if(disks[disk_offset + i].filesystem != 0)
            partitions_used++;
        else //failed, unformatted partition
            disks[disk_offset + i].dev = UNKNOWN_DEV_TYPE;
    }
    disk_offset += partitions_used;

out:
    return;
}

struct disk* disk_get(int index)
{
    if(index > SERPAEOS_MAX_DISKS - 1)
        return NULL;
    if(disks[index].dev <= 0)
        return NULL; //disk does not exist
    return &disks[index];
}
//============================================================================================

//if drive == true, targeting master drive
static int pata_read_sector(int primary_secondary, bool drive, int lba, int total, void *buf)
{
    outb((pors + 6), (lba >> 24) | ((drive)? 0xe0 : 0xf0));
    outb((pors + 2), total);
    outb((pors + 3), (unsigned char)(lba & 0xff));
    outb((pors + 4), (unsigned char)(lba >> 8));
    outb((pors + 5), (unsigned char)(lba >> 16));
    outb((pors + 7), 0x20);

    unsigned short* ptr = (unsigned short*) buf;
    for (int b = 0; b < total; b++)
    {
        // Wait for the buffer to be ready
        char c = inb((pors + 7));
        int timer = 0;
        while(!(c & 0x08))
        {
            c = inb((pors + 7));
            timer++;
            if(timer == 0x10000)
                return -ETIMEDOUT;
        }

        // Copy from hard disk to memory
        for (int i = 0; i < 256; i++)
        {
            *ptr = inw(pors);
            ptr++;
        }

    }
    return 0;
}

int pata_write_sector(int primary_secondary, bool drive, int lba, int total, void* buf)
{
    outb((pors + 6), (lba >> 24) | ((drive)? 0xe0 : 0xf0));
    outb((pors + 2), total);
    outb((pors + 3), (unsigned char)(lba & 0xff));
    outb((pors + 4), (unsigned char)(lba >> 8));
    outb((pors + 5), (unsigned char)(lba >> 16));
    outb((pors + 7), 0x30);
    char c;
    unsigned short* ptr = (unsigned short*) buf;
    for (int b = 0; b < total; b++)
    {
        // Wait for the buffer to be ready
        c = inb((pors + 7));
        int timer = 0;
        while(!(c & 0x08))
        {
            c = inb((pors + 7));
            timer++;
            if(timer == 0x10000)
                return -ETIMEDOUT;
        }

        // Copy from memory to hard disk
        for (int i = 0; i < 256; i++)
        {
            outw(pors, *ptr);
            ptr++;
        }
    }
    outb((pors + 7), 0xE7);
    c = inb((pors + 7));
    while((c & 0b10000000))
    {
        c = inb((pors + 7));
    }

    return 0;
}



//===============================================================================================

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
    if(idisk->dev <= UNKNOWN_DEV_TYPE)
        return -EINVARG;
    return pata_read_sector(idisk->primary_secondary, (idisk->master_slave == MASTER) ? true : false, lba + ((idisk->type == SERPAEOS_DISK_TYPE_PARTITION)? idisk->offset : 0), total, buf);
}
int disk_write_block(struct disk* idisk, int lba, int total, void* buf)
{
    if(idisk->dev <= UNKNOWN_DEV_TYPE)
        return -EINVARG;
    return pata_write_sector(idisk->primary_secondary, (idisk->master_slave == MASTER) ? true : false, lba + idisk->offset, total, buf);
}
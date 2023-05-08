#ifndef DISK_H
#define DISK_H

#include "fs/file.h"

// Represents a real physical hard disk
#define SERPAEOS_DISK_TYPE_REAL 1
// Represents a partition
#define SERPAEOS_DISK_TYPE_PARTITION 2

enum device_type
{
    UNKNOWN_DEV_TYPE,
    PATAPI,
    SATAPI,
    PATA,
    SATA
};


#define MASTER 1
#define SLAVE 2

#define PRIMARY 1
#define SECONDARY 2

struct disk
{
    int type; // disk type code (REAL or PARTITION)
    int sector_size;
    enum device_type dev;
    // The id of the disk
    int id;

    struct filesystem *filesystem;

    // The private data of our filesystem
    void *fs_private;

    int master_slave;
    int primary_secondary;

    uint32_t offset; //offset for partitions
};

void disk_search_and_init();
struct disk *disk_get(int index);
int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf);
int disk_write_block(struct disk *idisk, int lba, int total, void *buf);

#endif
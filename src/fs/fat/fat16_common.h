#ifndef FAT16_COMMON_H
#define FAT16_COMMON_H

#include <stdint.h>
#include <stddef.h>

#define SERPAEOS_FAT16_SIGNATURE 0x29
#define SERPAEOS_FAT16_FAT_ENTRY_SIZE 0x02
#define SERPAEOS_FAT16_BAD_SECTOR 0xFFF7
#define SERPAEOS_FAT16_UNUSED 0x00

//end of cluster chain
#define EOC 0xFFFF

typedef unsigned char FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

#define FAT16_DIRECTORY_ENTRIES_PER_CLUSTER(disk, private) ((disk->sector_size * private->header.primary_header.sectors_per_cluster) / sizeof(struct fat_directory_item))

struct disk;

struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_setors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended extended_header;
    } shared;
};

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory
{
    struct fat_directory_item *item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item
{
    union
    {
        struct fat_directory_item *item;
        struct fat_directory *directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_file_descriptor
{
    struct path_part *path;
    struct fat_item *item;
    uint32_t pos;
    FILE_MODE mode;
};

struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;

    // Used to stream data clusters
    struct disk_stream *cluster_read_stream;
    // Used to stream the file allocation table
    struct disk_stream *fat_read_stream;

    // Used in situations where we stream the directory
    struct disk_stream *directory_stream;

};

#define fat16_sector_to_absolute(disk, sector) (sector * disk->sector_size)

#define fat16_get_first_cluster(item) ((item->high_16_bits_first_cluster << 16) | item->low_16_bits_first_cluster)

#define fat16_cluster_to_sector(private, cluster) (private->root_directory.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster))

#define fat16_sector_to_cluster(private, sector) ((sector > private->root_directory.ending_sector_pos) ? \
                                                    (sector - private->root_directory.ending_sector_pos + 2) : \
                                                    0)

#define fat16_get_first_fat_sector(private) private->header.primary_header.reserved_sectors

#define fat16_high_16_bits(dword) ((unsigned int)dword >> 16)
#define fat16_low_16_bits(dword) ((unsigned int)dword & 0xffff)

void fat16_to_proper_string(char **out, const char *in, size_t size);
void fat16_get_full_relative_filename(struct fat_directory_item *item, char *out, int max_len);
struct fat_directory_item *fat16_clone_directory_item(struct fat_directory_item *item, int size);
int fat16_get_fat_entry(struct disk *disk, int cluster);
int fat16_get_cluster_for_offset(struct disk *disk, int starting_cluster, int offset);
void fat16_free_directory(struct fat_directory *directory);
void fat16_fat_item_free(struct fat_item *item);
struct fat_item *fat16_find_item_in_directory(struct disk *disk, struct fat_directory *directory, const char *name);
struct fat_directory_item *fat16_find_entry_in_directory(struct fat_directory *dir, const char *name);
struct fat_item *fat16_get_parent_dir(struct disk *disk, struct path_part *path);
struct fat_item *fat16_get_directory_entry(struct disk *disk, struct path_part *path);
void fat16_free_file_descriptor(struct fat_file_descriptor *desc);

uint32_t fat16_filesize_to_cluster_count(struct disk *disk, uint32_t filesize);

int fat16_write_directory_entry(struct disk *disk, struct path_part *path, struct fat_directory_item *new_item);
int fat16_write_fat_entry(struct disk *disk, uint32_t entry, uint16_t val);
int fat16_save_directory_to_disk(struct disk *disk, struct fat_directory *dir);

int fat16_write(struct disk *disk, void *descriptor, char *buf, int size);
int fat16_delete(struct disk *disk, void *descriptor);

int fat16_create_file(struct disk *disk, struct path_part *path);


void string_to_FAT_filename(char filename[13], char *out);

#endif
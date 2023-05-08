#include "fat32.h"
#include "../file.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "config.h"
#include <stdint.h>

//Definitions
typedef unsigned char FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

#define SERPAEOS_FAT32_FAT_ENTRY_SIZE 0x04

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

#define fat32_get_first_cluster(item) ((item->high_16_bits_first_cluster << 16) | item->low_16_bits_first_cluster)

#define fat32_sector_to_absolute(disk, sector) (sector * disk->sector_size)

#define fat32_get_first_fat_sector(private) private->header.primary_header.reserved_sectors

struct fat_header_extended
{
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info_location;
    uint16_t bkup_bootsect_location;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t reserved2;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t label[11];
    uint8_t fs_type[8]; //FAT32
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
    struct fat_header_extended extended;
}__attribute__((packed));

struct fat_fsinfo
{
    uint32_t lead_signature; //0x41615252
    uint8_t resrved1[480];
    uint32_t signature; //0x61417272
    uint32_t free_cluster_count;
    uint32_t next_free_cluster;
    uint8_t reserved2[12];
    uint32_t trail_signature; //0xAA550000
}__attribute__((packed));

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
    struct fat_directory_item *item; //array
    int total;
    uint32_t sector_pos;
    uint32_t ending_sector_pos;
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
    struct fat_directory *root_directory;
    struct fat_fsinfo fsinfo;

    // Used to stream data clusters
    struct disk_stream *cluster_stream;

    // Used to stream the file allocation table
    struct disk_stream *fat_stream;

    // Used in situations where we stream the directory
    struct disk_stream *directory_stream;

};

//*****************************************************

static uint32_t fat32_cluster_to_sector(struct fat_private *private, uint32_t cluster);
static int fat32_read_internal(struct disk *disk, int starting_cluster, int offset, int total, void *out);
void fat32_free_directory(struct fat_directory *directory);

int fat32_resolve(struct disk *disk);
void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode);
int fat32_read(struct disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr);
int fat32_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat32_stat(struct disk *disk, void *private, file_stat *stat);
int fat32_close(void *private);
int fat32_ftell(void *private);
int fat32_write(struct disk *disk, void *descriptor, char *buf, int size);
int fat32_delete(struct disk *disk, void *descriptor);


struct filesystem fat32_fs =
    {
        .resolve = fat32_resolve,
        .open = fat32_open,
        .read = fat32_read,
        .seek = fat32_seek,
        .stat = fat32_stat,
        .close = fat32_close,
        .ftell = fat32_ftell,
        .write = fat32_write,
        .delete = fat32_delete
    };
struct filesystem *fat32_init()
{
    strcpy(fat32_fs.name, "FAT32");
    return &fat32_fs;
}

int fat32_ftell(void *private)
{
    struct fat_file_descriptor *desc = private;
    return desc->pos;
}

static void fat32_init_private(struct disk *disk, struct fat_private *private)
{
    memset(private, 0, sizeof(struct fat_private));
    private->cluster_stream = diskstreamer_new(disk->id);
    private->fat_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);   
}

int fat32_get_total_items_for_directory(struct disk *disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct fat_private *fat_private = disk->fs_private;

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;
    struct disk_stream *stream = fat_private->directory_stream;
    if (diskstreamer_seek(stream, directory_start_pos) != SERPAEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    while (1)
    {
        if (diskstreamer_read(stream, &item, sizeof(item)) != SERPAEOS_ALL_OK)
        {
            res = -EIO;
            goto out;
        }

        if (item.filename[0] == 0x00)
        {
            // We are done
            break;
        }

        // Is the item unused
        if (item.filename[0] == 0xE5)
        {
            continue;
        }

        i++;
    }

    res = i;

out:
    return res;
}

int fat32_load_root_dir(struct disk *disk, struct fat_private *private)
{
    int res = 0;
    struct fat_directory *directory = 0;

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t cluster = private->header.extended.root_cluster;
    uint32_t cluster_sector = fat32_cluster_to_sector(private, cluster);
    int total_items = fat32_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size);
    if (!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }

    res = fat32_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if (res != SERPAEOS_ALL_OK)
    {
        goto out;
    }

    directory->sector_pos = cluster_sector;

    private->root_directory = directory;

out:
    if (res != SERPAEOS_ALL_OK)
    {
        fat32_free_directory(directory);
        private->root_directory = 0;
    }
    return res;
}

int fat32_resolve(struct disk *disk)
{
    int res = 0;
    struct fat_private *fat_private = kzalloc(sizeof(struct fat_private));
    fat32_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat32_fs;

    struct disk_stream *stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        res = -ENOMEM;
        goto out;
    }

    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != SERPAEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (fat_private->header.extended.signature != 0x29)
    {
        res = -ENOTUS;
        goto out;
    }

    if(fat32_load_root_dir(disk, fat_private) != SERPAEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    uint32_t fsinfo_location = fat_private->header.extended.fs_info_location * disk->sector_size;
    if(diskstreamer_seek(stream, fsinfo_location) != SERPAEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if(diskstreamer_read(stream, &fat_private->fsinfo, sizeof(struct fat_fsinfo)) != SERPAEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if((fat_private->fsinfo.lead_signature != 0x41615252) || fat_private->fsinfo.trail_signature != 0xAA550000)
    {
        res = -ENOTUS;
        goto out;
    }
    res = 0;

out:
    if (stream)
    {
        diskstreamer_close(stream);
    }

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

void fat32_to_proper_string(char **out, const char *in, size_t size)
{
    int i = 0;
    while (*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in += 1;
        // We cant process anymore since we have exceeded the input buffer size
        if (i >= size - 1)
        {
            break;
        }
        i++;
    }

    **out = 0x00;
}

void fat32_get_full_relative_filename(struct fat_directory_item *item, char *out, int max_len)
{
    memset(out, 0x00, max_len);
    char *out_tmp = out;
    fat32_to_proper_string(&out_tmp, (const char *)item->filename, sizeof(item->filename));
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat32_to_proper_string(&out_tmp, (const char *)item->ext, sizeof(item->ext));
    }
}

struct fat_directory_item *fat32_clone_directory_item(struct fat_directory_item *item, int size)
{
    struct fat_directory_item *item_copy = 0;
    if (size < sizeof(struct fat_directory_item))
    {
        return 0;
    }

    item_copy = kzalloc(size);
    if (!item_copy)
    {
        return 0;
    }

    memcpy(item_copy, item, size);
    return item_copy;
}

uint32_t fat32_get_fat_entry(struct disk *disk, uint32_t cluster)
{
    uint32_t res = -1;
    struct fat_private *private = disk->fs_private;
    struct disk_stream *stream = private->fat_stream;
    if (!stream)
    {
        goto out;
    }

    uint32_t fat_table_position = (private->header.primary_header.reserved_sectors) * disk->sector_size;
    res = diskstreamer_seek(stream, fat_table_position + ((cluster - 0) * SERPAEOS_FAT32_FAT_ENTRY_SIZE));
    if (res < 0)
    {
        goto out;
    }

    uint32_t result = 0;
    res = diskstreamer_read(stream, &result, sizeof(result));
    if (res < 0)
    {
        goto out;
    }

out:
    if(res != SERPAEOS_ALL_OK) 
        return 0xFFFFFFFF;
    return result;
}

int fat32_get_cluster_for_offset(struct disk *disk, int starting_cluster, int offset)
{
    int res = 0;
    struct fat_private *private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        uint32_t entry = fat32_get_fat_entry(disk, cluster_to_use);
        if (entry >= 0x0FFFFFF8)
        {
            // We are at the last entry in the file
            res = -EIO;
            goto out;
        }

        // Sector is marked as bad?
        if (entry == 0x0FFFFFF7)
        {
            res = -EIO;
            goto out;
        }

        // Reserved sector?
        if (entry == 0x0FFFFFF0 || entry == 0x0FFFFFF6)
        {
            res = -EIO;
            goto out;
        }

        if (entry == 0x00)
        {
            res = -EIO;
            goto out;
        }

        cluster_to_use = (int)entry;
    }

    res = cluster_to_use;
out:
    return res;
}

static uint32_t fat32_cluster_to_sector(struct fat_private *private, uint32_t cluster)
{
    uint32_t res = private->header.primary_header.reserved_sectors + (private->header.primary_header.fat_copies * private->header.extended.sectors_per_fat);
    res += (cluster - 2) * private->header.primary_header.sectors_per_cluster;
    return res;
}

static int fat32_read_internal_from_stream(struct disk *disk, struct disk_stream *stream, uint32_t cluster, int offset, int total, void *out)
{
    int res = 0;
    struct fat_private *private = disk->fs_private;
    uint32_t size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat32_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0)
    {
        res = cluster_to_use;
        goto out;
    }

    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat32_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if (res != SERPAEOS_ALL_OK)
    {
        goto out;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if (res != SERPAEOS_ALL_OK)
    {
        goto out;
    }

    total -= total_to_read;
    if (total > 0)
    {
        // We still have more to read
        res = fat32_read_internal_from_stream(disk, stream, cluster, offset + total_to_read, total, out + total_to_read);
    }

out:
    return res;
}

static int fat32_read_internal(struct disk *disk, int starting_cluster, int offset, int total, void *out)
{
    struct fat_private *fs_private = disk->fs_private;
    struct disk_stream *stream = fs_private->cluster_stream;
    return fat32_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat32_free_directory(struct fat_directory *directory)
{
    if (!directory)
    {
        return;
    }

    if (directory->item)
    {
        kfree(directory->item);
    }

    kfree(directory);
}


void fat32_fat_item_free(struct fat_item *item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat32_free_directory(item->directory);
    }
    else if (item->type == FAT_ITEM_TYPE_FILE)
    {
        kfree(item->item);
    }

    kfree(item);
}

struct fat_directory *fat32_load_fat_directory(struct disk *disk, struct fat_directory_item *item)
{
    int res = 0;
    struct fat_directory *directory = 0;
    struct fat_private *fat_private = disk->fs_private;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t cluster = (item->high_16_bits_first_cluster << 16) | item->low_16_bits_first_cluster;
    uint32_t cluster_sector = fat32_cluster_to_sector(fat_private, cluster);
    int total_items = fat32_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size);
    if (!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }

    res = fat32_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if (res != SERPAEOS_ALL_OK)
    {
        goto out;
    }

    directory->sector_pos = cluster_sector;

out:
    if (res != SERPAEOS_ALL_OK)
    {
        fat32_free_directory(directory);
    }
    return directory;
}

struct fat_item *fat32_new_fat_item_for_directory_item(struct disk *disk, struct fat_directory_item *item)
{
    struct fat_item *f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item)
    {
        return 0;
    }

    if (item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat32_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
        return f_item;
    }

    f_item->type = FAT_ITEM_TYPE_FILE;
    f_item->item = fat32_clone_directory_item(item, sizeof(struct fat_directory_item));
    return f_item;
}

struct fat_item *fat32_find_item_in_directory(struct disk *disk, struct fat_directory *directory, const char *name)
{
    struct fat_item *f_item = 0;
    char tmp_filename[SERPAEOS_MAX_PATH];
    for (int i = 0; i < directory->total; i++)
    {
        if(directory->item[i].attribute & FAT_FILE_VOLUME_LABEL)
            continue;
        fat32_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            // Found it let's create a new fat_item
            f_item = fat32_new_fat_item_for_directory_item(disk, &directory->item[i]);
            goto out;
        }
    }
out:
    return f_item;
}

struct fat_directory_item *fat32_find_entry_in_directory(struct fat_directory *dir, const char *name)
{
    struct fat_directory_item *res = 0;
    char tmp_filename[SERPAEOS_MAX_PATH];
    for (int i = 0; i < dir->total; i++)
    {
        fat32_get_full_relative_filename(&dir->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            res = &dir->item[i];
            break;
        }
    }
    return res;
}

struct fat_item *fat32_get_directory_entry(struct disk *disk, struct path_part *path)
{
    struct fat_private *fat_private = disk->fs_private;
    struct fat_item *current_item = 0;
    struct fat_item *root_item = fat32_find_item_in_directory(disk, fat_private->root_directory, path->part);
    if (!root_item)
    {
        goto out;
    }

    struct path_part *next_part = path->next;
    current_item = root_item;
    while (next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY)
        {
            current_item = 0;
            break;
        }

        struct fat_item *tmp_item = fat32_find_item_in_directory(disk, current_item->directory, next_part->part);
        fat32_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
out:
    return current_item;
}

struct fat_item *fat32_get_parent_dir(struct disk *disk, struct path_part *path)
{
    struct fat_private *private = disk->fs_private;
    if (!path)
        return NULL;
    if (!path->next)
    {
        struct fat_item *res = kzalloc(sizeof(struct fat_item));
        res->type = FAT_ITEM_TYPE_DIRECTORY;
        res->directory = private->root_directory;
        return res;
    }
    struct path_part *curr = 0;
    for (curr = path; curr->next->next; curr = curr->next)
        ;
    struct path_part *tmp = curr->next;
    curr->next = NULL;

    struct fat_item *res = fat32_get_directory_entry(disk, path);
    curr->next = tmp;
    return res;
}

int fat32_write_directory_entry(struct disk *disk, struct path_part *path, struct fat_directory_item *new_item);

void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode)
{
    struct fat_file_descriptor *descriptor = 0;
    int err_code = 0;

    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor)
    {
        err_code = -ENOMEM;
        goto err_out;
    }

    descriptor->item = fat32_get_directory_entry(disk, path);
    if (!descriptor->item)
    {
        err_code = -EIO;
        goto err_out;
    }
    if(descriptor->item->type != FAT_ITEM_TYPE_FILE)
    {
        err_code = -EINVARG;
        goto err_out;
    }

    if(mode & FILE_MODE_WRITE)
        if(descriptor->item->item->attribute & FAT_FILE_READ_ONLY)
        {
            err_code = -ERDONLY;
            goto err_out;
        }

    if(mode & FILE_MODE_APPEND)
        fat32_seek((void *)descriptor, 0, SEEK_END);
    else
        descriptor->pos = 0;

    descriptor->path = path;
    descriptor->mode = mode;

    //update time stamp
    struct date_time *now = get_current_time();
    descriptor->item->item->last_access = (now->day & 31) | (now->month << 5) | ((now->year - 1980) << 9);
    fat32_write_directory_entry(disk, descriptor->path, descriptor->item->item);

    return (void *)descriptor;

err_out:
    if (descriptor)
        kfree(descriptor);

    return ERROR(err_code);
}

void fat32_free_file_descriptor(struct fat_file_descriptor *desc)
{
    fat32_fat_item_free(desc->item);
    kfree(desc);
}

int fat32_close(void *private)
{
    struct fat_file_descriptor *desc = private;
    struct path_root *root = kzalloc(sizeof(struct path_root));
    root->first = desc->path;
    pathparser_free(root);
    fat32_free_file_descriptor(desc);
    return 0;
}

int fat32_stat(struct disk *disk, void *private, file_stat *stat)
{
    int res = 0;
    struct fat_file_descriptor *descriptor = (struct fat_file_descriptor *)private;
    struct fat_item *desc_item = descriptor->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item *ritem = desc_item->item;
    stat->filesize = ritem->filesize;
    stat->flags = 0x00;

    if (ritem->attribute & FAT_FILE_READ_ONLY)
    {
        stat->flags |= FILE_STAT_READ_ONLY;
    }

    stat->creation_datetime.seconds = (ritem->creation_time & 0b00011111) * 2;
    stat->creation_datetime.minutes = (ritem->creation_time >> 5) & 0b00111111;
    stat->creation_datetime.hours = (ritem->creation_time >> 11);

    stat->creation_datetime.day = ritem->creation_date & 0b00011111;
    stat->creation_datetime.month = (ritem->creation_date >> 5) & 0b00001111;
    stat->creation_datetime.year = 1980 + (ritem->creation_date >> 9);

out:
    return res;
}

int fat32_read(struct disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    int res = 0;
    struct fat_file_descriptor *fat_desc = descriptor;
    struct fat_directory_item *item = fat_desc->item->item;
    int offset = fat_desc->pos;
    
    for (uint32_t i = 0; i < nmemb; i++)
    {
        res = fat32_read_internal(disk, fat32_get_first_cluster(item), offset, size, out_ptr);
        if (ISERR(res))
        {
            goto out;
        }

        out_ptr += size;
        offset += size;
    }
    fat_desc->pos = offset;
out:
    return res;
}

int fat32_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    int res = 0;
    struct fat_file_descriptor *desc = private;
    struct fat_item *desc_item = desc->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item *ritem = desc_item->item;
    if (offset >= ritem->filesize)
    {
        res = -EIO;
        goto out;
    }

    switch (seek_mode)
    {
    case SEEK_SET:
        desc->pos = offset;
        break;

    case SEEK_END:
        desc->pos = desc->item->item->filesize - offset;
        break;

    case SEEK_CUR:
        desc->pos += offset;
        break;
    case SEEK_NEG:
        desc->pos -= offset;
        break;

    default:
        res = -EINVARG;
        break;
    }
out:
    return res;
}

extern void panic(const char *);

uint32_t fat32_find_free_entry_in_directory(struct fat_directory *dir)
{
    uint32_t res = 0;
    while(1)
    {
        struct fat_directory_item *item = &dir->item[res];
        if(item->filename[0] == 0x00)
        {
            //done
            res = 0xFFFFFFFF;
            break;
        }
        if(item->filename[0] == 0xE5)
        {
            //found a free entry!
            break;
        }
        res++;
    }

    return res;
}

uint32_t fat32_filesize_to_cluster_count(struct disk *disk, uint32_t filesize)
{
    struct fat_private *private = disk->fs_private;
    uint32_t bytes_per_cluster = disk->sector_size * private->header.primary_header.sectors_per_cluster;
    uint32_t res = filesize / bytes_per_cluster;
    if (filesize % bytes_per_cluster)
        res++;
    return res;
}

int fat32_write_directory_entry(struct disk *disk, struct path_part *path, struct fat_directory_item *new_item)
{
    int i;
    struct fat_item *parent_dir_item = fat32_get_parent_dir(disk, path);
    struct fat_directory *dir = parent_dir_item->directory;
    if (!dir)
        return -EBADPATH;

    struct fat_private *private = disk->fs_private;
    uint32_t location_bytes = dir->sector_pos * disk->sector_size;

    char tmp_path[12];

    for (i = 0; i < dir->total; i++)
    {
        fat32_get_full_relative_filename(&dir->item[i], tmp_path, 12);
        if (istrncmp(pathparser_get_last_part(path)->part, tmp_path, 12) == 0)
        {
            break;
        }
        location_bytes += sizeof(struct fat_directory_item);
    }
    if (i == dir->total)
        return -EINVARG;

    i = diskstreamer_seek(private->directory_stream, location_bytes);
    if (i != SERPAEOS_ALL_OK)
        return i;
    i = diskstreamer_write(private->directory_stream, new_item, sizeof(struct fat_directory_item));

    // clean up
    fat32_fat_item_free(parent_dir_item);

    return i;
}

int fat32_write_fat_entry(struct disk *disk, uint32_t entry, uint32_t val)
{
    struct fat_private *private = disk->fs_private;
    uint32_t location_bytes = (fat32_get_first_fat_sector(private) * disk->sector_size) + (entry * sizeof(uint32_t));
    diskstreamer_seek(private->fat_stream, location_bytes);
    return diskstreamer_write(private->fat_stream, &val, sizeof(uint32_t));
}

static void fat32_update_fsinfo(struct disk *disk, struct fat_private *private)
{
    disk_write_block(disk, private->header.extended.fs_info_location, 1, &private->fsinfo);
}

uint32_t fat32_get_first_free_cluster(struct disk *disk)
{
    struct fat_private *private = disk->fs_private;
    uint32_t cluster;
    if(private->fsinfo.next_free_cluster == 0xFFFFFFFF)
        cluster = 3;
    else
        cluster = private->fsinfo.next_free_cluster;
    uint32_t code;
    while (1)
    {
        
        code = fat32_get_fat_entry(disk, cluster);

        if (code == 0)
            break;
        if (cluster >= private->header.primary_header.sectors_big / private->header.primary_header.sectors_per_cluster)
            return 0;
        cluster++;
    }

    private->fsinfo.next_free_cluster = cluster;
    fat32_update_fsinfo(disk, private);

    return cluster;
}

static uint32_t fat32_find_last_cluster_of_chain(struct disk *disk, uint32_t first_cluster)
{
    uint32_t curr = first_cluster, code;
    while (1)
    {
        code = fat32_get_fat_entry(disk, curr);
        if (code >= 0x0FFFFFF8)
            break;
        curr = code;
    }
    return curr;
}

static int fat32_new_cluster_chain(struct disk *disk, uint32_t *out, int count)
{
    int i;
    for (i = 0; i < count - 1; i++)
    {
        out[i] = fat32_get_first_free_cluster(disk);
        if (out[i] == 0)
            return -ENODISKSPACE;
        
    }
    out[i + 1] = 0x0FFFFFFF;
    return i;
}

static int fat32_get_cluster_chain(struct disk *disk, uint32_t *out, uint32_t first_cluster)
{
    int i = 0;
    uint32_t curr = first_cluster;
    do
    {
        out[i] = curr;
        i++;
    } while ((curr = fat32_get_fat_entry(disk, curr)) < 0x0FFFFFF8);
    return i;
}

static int fat32_count_clusters_in_chain(struct disk *disk, uint16_t first_cluster)
{
    int i = 1;
    uint32_t curr = first_cluster;

    for (; (curr = fat32_get_fat_entry(disk, curr)) < 0x0FFFFFF8; i++)
        ;

    return i;
}

static int fat32_add_cluster_to_chain(struct disk *disk, uint32_t first_cluster)
{
    uint32_t cluster = fat32_find_last_cluster_of_chain(disk, first_cluster);
    uint32_t next_free_cluster = fat32_get_first_free_cluster(disk);
    if (next_free_cluster == 0)
        return -ENODISKSPACE;

    if (fat32_write_fat_entry(disk, cluster, next_free_cluster) != 2)
        return -EIO;
    if (fat32_write_fat_entry(disk, next_free_cluster, 0x0FFFFFFF) != 2)
        return -EIO;
    return 0;
}

static uint32_t fat32_generate_cluster_chain(struct disk *disk, int count)
{
    if (count <= 0 || disk == NULL)
        return -EINVARG;
    
    uint32_t first = fat32_get_first_free_cluster(disk);
    if(first == 0)
        return 0;
    if (fat32_write_fat_entry(disk, first, 0x0FFFFFF) != sizeof(uint32_t))
        return -EIO;
    for(int i = 0; i < count - 1; i++)
        fat32_add_cluster_to_chain(disk, first);
    return first;
}

static int fat32_erase_cluster_chain(struct disk *disk, uint32_t first)
{
    if(first <= 2)
        return -EINVARG;
    
    uint32_t curr = first, code;
    while(1)
    {
        code = fat32_get_fat_entry(disk, curr);
        fat32_write_fat_entry(disk, curr, 0x00);
        if(code == 0x0FFFFFF)
            break;
        curr = code;
    }
    return 0;
}


/*
steps to delete a file:

1. filename[0] = 0x00
2. erase FAT entries
*/

int fat32_delete(struct disk *disk, void *descriptor)
{
    struct fat_file_descriptor *desc = descriptor;
    if(((desc->mode & FILE_MODE_WRITE) == 0) || (desc->item->item->attribute & FAT_FILE_READ_ONLY))
        return -EINVARG; //no permission to write or delete
    
    desc->item->item->filename[0] = 0xE5;
    int res = fat32_write_directory_entry(disk, desc->path, desc->item->item);
    if(res < 0)
        return -EIO;
    
    res = fat32_erase_cluster_chain(disk, (desc->item->item->high_16_bits_first_cluster << 16) | desc->item->item->low_16_bits_first_cluster);
    fat32_free_file_descriptor(desc);
    return res;
}

/*
steps to write to file

1. add cluster to chain (if necissary)
2. add to filesize of dir entry
3. write to disk
*/

int fat32_write(struct disk *disk, void *descriptor, char *buf, int size)
{
    if (!descriptor || !buf || !size || !disk)
        return -EINVARG;

    struct fat_file_descriptor *desc = descriptor;
    if (desc->item->type != FAT_ITEM_TYPE_FILE)
        return -EINVARG;
    if (!(desc->mode & FILE_MODE_WRITE))
        return -ERDONLY;

    int res = 0;

    struct fat_private *private = disk->fs_private;

    // check if it is necissary to write cluster to chain
    uint32_t old_filesize = desc->item->item->filesize;
    uint32_t old_cluster_count = fat32_filesize_to_cluster_count(disk, old_filesize);

    // first cluster to write is currently last cluster of chain
    uint32_t first_cluster_to_write = fat32_find_last_cluster_of_chain(disk, (desc->item->item->high_16_bits_first_cluster << 16) | desc->item->item->low_16_bits_first_cluster);

    if ((old_filesize + size) > (old_cluster_count * (disk->sector_size * private->header.primary_header.sectors_per_cluster)))
    {
        uint32_t count_of_needed_clusters = fat32_filesize_to_cluster_count(disk, old_filesize + size) - old_cluster_count;
        // add anoher cluster
        while (count_of_needed_clusters-- > 0)
        {
            res = fat32_add_cluster_to_chain(disk, (uint32_t)((desc->item->item->high_16_bits_first_cluster << 16) | desc->item->item->low_16_bits_first_cluster));
            if (res < 0)
                goto out;
        }
    }

    // step 2
    desc->item->item->filesize += size;
    res = fat32_write_directory_entry(disk, desc->path, desc->item->item);
    if (res <= 0)
        goto out;

    // step 3
    uint32_t offset = fat32_sector_to_absolute(disk, fat32_cluster_to_sector(private, first_cluster_to_write));
    uint32_t free_space_of_first_cluster = (desc->pos % (private->header.primary_header.sectors_per_cluster * disk->sector_size));
    offset += free_space_of_first_cluster;

    uint32_t target_cluster = first_cluster_to_write;

    res = diskstreamer_seek(private->cluster_stream, offset);
    if(res < 0)
        goto out;

    res = diskstreamer_write(private->cluster_stream, buf, free_space_of_first_cluster);
    if(res < 0)
        goto out;

    buf += free_space_of_first_cluster;

    size -= free_space_of_first_cluster;

    //do the rest of the clusters
    while(size > 0)
    {
        target_cluster = fat32_get_fat_entry(disk, target_cluster);
        if(target_cluster >= 0x0FFFFFF8)
            break;
        
        int total_to_write = (size <= (private->header.primary_header.sectors_per_cluster * disk->sector_size)) ? size : (private->header.primary_header.sectors_per_cluster * disk->sector_size);

        res = diskstreamer_seek(private->cluster_stream, fat32_sector_to_absolute(disk, fat32_cluster_to_sector(private, target_cluster)));
        if(res < 0)
            goto out;

        res = diskstreamer_write(private->cluster_stream, buf, total_to_write);
        if(res < 0)
            goto out;
        
        buf += total_to_write;
        size -= total_to_write;
    }    
out:
    return res;
}
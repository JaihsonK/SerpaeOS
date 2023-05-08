#include "fat16.h"
#include "fat16_common.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "idt/idt.h"
#include "time/time.h"
#include "status.h"

extern void panic(const char *msg);

void string_to_FAT_filename(char filename[13], char *out)
{
    bool ext = false;
    int f = 0, e = 0;
    memset(out, ' ', 11);
    for(int i = 0; i < 13 && filename[i]; i++)
    {
        if(filename[i] == '.')
            ext = true;
        else 
        {
            if(ext)
            {
                *(out + 8 + e) = toupper(filename[i]);
                e++;
                if(e == 3)
                    return;
            }
            else
            {
                *(out + f) = toupper(filename[i]);
                f++;
                if(f == 8)
                    return;
            }
        }
    }
}

uint16_t fat16_find_free_entry_in_directory(struct fat_directory *dir)
{
    uint16_t res = 0;
    while(1)
    {
        struct fat_directory_item *item = &dir->item[res];
        if(item->filename[0] == 0x00 || item->filename[0] == 0xE5)
            break;
        res++;
    }

    return res;
}

uint32_t fat16_filesize_to_cluster_count(struct disk *disk, uint32_t filesize)
{
    struct fat_private *private = disk->fs_private;
    uint32_t bytes_per_cluster = disk->sector_size * private->header.primary_header.sectors_per_cluster;
    uint32_t res = filesize / bytes_per_cluster;
    if (filesize % bytes_per_cluster)
        res++;
    return res;
}

int fat16_write_directory_entry(struct disk *disk, struct path_part *path, struct fat_directory_item *new_item)
{
    int i;
    struct fat_item *parent_dir_item = fat16_get_parent_dir(disk, path);
    struct fat_directory *dir = parent_dir_item->directory;
    if (!dir)
        return -EBADPATH;

    struct fat_private *private = disk->fs_private;
    uint32_t location_bytes = dir->sector_pos * disk->sector_size;

    char tmp_path[12];

    for (i = 0; i < dir->total; i++)
    {
        fat16_get_full_relative_filename(&dir->item[i], tmp_path, 12);
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
    fat16_fat_item_free(parent_dir_item);

    return i;
}

int fat16_write_fat_entry(struct disk *disk, uint32_t entry, uint16_t val)
{
    struct fat_private *private = disk->fs_private;
    uint32_t location_bytes = (fat16_get_first_fat_sector(private) * disk->sector_size) + (entry * sizeof(uint16_t));
    diskstreamer_seek(private->fat_read_stream, location_bytes);
    return diskstreamer_write(private->fat_read_stream, &val, sizeof(uint16_t));
}

uint16_t fat16_get_first_first_cluster(struct disk *disk)
{
    struct fat_private *private = disk->fs_private;
    uint32_t cluster = 3;
    uint16_t code;
    while (1)
    {
        code = fat16_get_fat_entry(disk, cluster);

        if (code == SERPAEOS_FAT16_UNUSED)
            break;
        if (cluster >= private->header.primary_header.sectors_big / private->header.primary_header.sectors_per_cluster)
            return 0;
        cluster++;
    }
    return cluster;
}

static uint16_t fat16_find_last_cluster_of_chain(struct disk *disk, uint16_t first_cluster)
{
    uint16_t curr = first_cluster, code;
    while (1)
    {
        code = (uint16_t) fat16_get_fat_entry(disk, curr);
        if (code >= 0xFFF8)
            break;
        curr = code;
    }
    return curr;
}

static int fat16_new_cluster_chain(struct disk *disk, uint16_t *out, int count)
{
    int i;
    for (i = 0; i < count - 1; i++)
    {
        out[i] = fat16_get_first_first_cluster(disk);
        if (out[i] == 0)
            return -ENODISKSPACE;
    }
    out[i + 1] = EOC;
    return i;
}

static int fat16_get_cluster_chain(struct disk *disk, uint16_t *out, uint16_t first_cluster)
{
    int i = 0;
    uint16_t curr = first_cluster;
    do
    {
        out[i] = curr;
        i++;
    } while ((curr = fat16_get_fat_entry(disk, (int)curr)) < 0xFFF8);
    return i;
}

static int fat16_count_clusters_in_chain(struct disk *disk, uint16_t first_cluster)
{
    int i = 1;
    uint16_t curr = first_cluster;

    for (; (curr = fat16_get_fat_entry(disk, curr)) < 0xFFF8; i++)
        ;

    return i;
}

static int fat16_add_cluster_to_chain(struct disk *disk, uint16_t first_cluster)
{
    uint16_t cluster = fat16_find_last_cluster_of_chain(disk, first_cluster);
    uint16_t next_first_cluster = fat16_get_first_first_cluster(disk);
    if (next_first_cluster == 0)
        return -ENODISKSPACE;

    if (fat16_write_fat_entry(disk, cluster, next_first_cluster) != 2)
        return -EIO;
    if (fat16_write_fat_entry(disk, next_first_cluster, EOC) != 2)
        return -EIO;
    return 0;
}

static uint16_t fat16_generate_cluster_chain(struct disk *disk, int count)
{
    if (count <= 0 || disk == NULL)
        return -EINVARG;
    
    uint16_t first = fat16_get_first_first_cluster(disk);
    if(first == 0)
        return 0;
    if (fat16_write_fat_entry(disk, first, 0xFFFF) != sizeof(uint16_t))
        return -EIO;
    for(int i = 0; i < count - 1; i++)
        fat16_add_cluster_to_chain(disk, first);
    return first;
}

static int fat16_erase_cluster_chain(struct disk *disk, uint16_t first)
{
    if(first <= 2)
        return -EINVARG;
    
    uint16_t curr = first, code;
    while(1)
    {
        code = fat16_get_fat_entry(disk, curr);
        fat16_write_fat_entry(disk, curr, 0x00);
        if(code == 0xFFFF)
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

int fat16_delete(struct disk *disk, void *descriptor)
{
    struct fat_file_descriptor *desc = descriptor;
    if(((desc->mode & FILE_MODE_WRITE) == 0) || (desc->item->item->attribute & FAT_FILE_READ_ONLY))
        return -EINVARG; //no permission to write or delete
    
    desc->item->item->filename[0] = 0xE5;
    int res = fat16_write_directory_entry(disk, desc->path, desc->item->item);
    if(res < 0)
        return -EIO;
    
    res = fat16_erase_cluster_chain(disk, desc->item->item->low_16_bits_first_cluster);
    fat16_free_file_descriptor(desc);
    return res;
}


/*
steps to write to file

1. add cluster to chain (if necissary)
2. add to filesize of dir entry
3. write to disk
*/

int fat16_write(struct disk *disk, void *descriptor, char *buf, int size)
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
    uint32_t old_cluster_count = fat16_filesize_to_cluster_count(disk, old_filesize);

    // first cluster to write is currently last cluster of chain
    uint16_t first_cluster_to_write = fat16_find_last_cluster_of_chain(disk, desc->item->item->high_16_bits_first_cluster << 16 | desc->item->item->low_16_bits_first_cluster);

    if ((old_filesize + size) > (old_cluster_count * (disk->sector_size * private->header.primary_header.sectors_per_cluster)))
    {
        uint32_t count_of_needed_clusters = fat16_filesize_to_cluster_count(disk, old_filesize + size) - old_cluster_count;
        // add anoher cluster
        while (count_of_needed_clusters-- > 0)
        {
            res = fat16_add_cluster_to_chain(disk, (uint16_t)(desc->item->item->high_16_bits_first_cluster << 16 | desc->item->item->low_16_bits_first_cluster));
            if (res < 0)
                goto out;
        }
    }

    // step 2
    desc->item->item->filesize += size;
    res = fat16_write_directory_entry(disk, desc->path, desc->item->item);
    if (res <= 0)
        goto out;

    // step 3
    uint32_t offset = fat16_sector_to_absolute(disk, fat16_cluster_to_sector(private, first_cluster_to_write));
    uint32_t free_space_of_first_cluster = (desc->pos % (private->header.primary_header.sectors_per_cluster * disk->sector_size));
    offset += free_space_of_first_cluster;

    uint16_t target_cluster = first_cluster_to_write;

    res = diskstreamer_seek(private->cluster_read_stream, offset);
    if(res < 0)
        goto out;

    res = diskstreamer_write(private->cluster_read_stream, buf, free_space_of_first_cluster);
    if(res < 0)
        goto out;

    buf += free_space_of_first_cluster;

    size -= free_space_of_first_cluster;

    //do the rest of the clusters
    while(size > 0)
    {
        target_cluster = (uint16_t)fat16_get_fat_entry(disk, target_cluster);
        if(target_cluster >= 0xFFF8)
            break;
        
        int total_to_write = (size <= (private->header.primary_header.sectors_per_cluster * disk->sector_size)) ? size : (private->header.primary_header.sectors_per_cluster * disk->sector_size);

        res = diskstreamer_seek(private->cluster_read_stream, fat16_sector_to_absolute(disk, fat16_cluster_to_sector(private, target_cluster)));
        if(res < 0)
            goto out;

        res = diskstreamer_write(private->cluster_read_stream, buf, total_to_write);
        if(res < 0)
            goto out;
        
        buf += total_to_write;
        size -= total_to_write;
    }    

out:
    return res;
}

/*
Steps to create file:
 1. find one free cluster
 2. create new directory entry
*/
int fat16_create_file(struct disk *disk, struct path_part *path)
{
    int res = 0;
    struct fat_private *priv = disk->fs_private;

//Step 1
    uint16_t first_cluster = fat16_generate_cluster_chain(disk, 1);
    if(first_cluster == 0)
        return -ENODISKSPACE;

//Step 2
    struct fat_item *parent_dir_item = fat16_get_parent_dir(disk, path);
    if(!parent_dir_item)
    {
        res = -EBADPATH;
        goto out;
    }
    struct fat_directory *parentdir = parent_dir_item->directory;

    struct date_time *now = get_current_time();

    struct fat_directory_item tmp =
    {
        .low_16_bits_first_cluster = first_cluster,
        .filesize = 0,
        .creation_date = ((now->day & 31) | (now->month << 5) | ((now->year - 1980) << 9)),
        .creation_time = 0, //not yet supported
        .attribute = 0,
        .creation_time_tenths_of_a_sec = 0
    };
    string_to_FAT_filename((char *)pathparser_get_last_part(path)->part, ((char *)tmp.filename));

    uint32_t offset = fat16_find_free_entry_in_directory(parentdir);
    if(offset == 0xFFFF)
    {
        res = -ENODISKSPACE;
        goto out;
    }

    uint32_t disk_loc = 0;
    uint16_t clusters = parentdir->total / FAT16_DIRECTORY_ENTRIES_PER_CLUSTER(disk, priv); //how many clusters in parent dir
    if(clusters) //if non-zero, find the right cluster/sector
    {
        uint16_t cluster = fat16_sector_to_cluster(priv, parentdir->sector_pos);
        int i;
        for(i = parentdir->total; (i > FAT16_DIRECTORY_ENTRIES_PER_CLUSTER(disk, priv)) && (cluster < 0xFFF0 && cluster > 2); i -= FAT16_DIRECTORY_ENTRIES_PER_CLUSTER(disk, priv))
            cluster = fat16_get_fat_entry(disk, cluster);
        if(i < 1)
        {
            res = -ENODISKSPACE;
            goto out;
        }
        disk_loc = (i*sizeof(struct fat_directory_item)) + fat16_sector_to_absolute(disk, fat16_cluster_to_sector(priv, cluster));
    }
    else
    {
        disk_loc = fat16_sector_to_absolute(disk, parentdir->sector_pos) + (sizeof(struct fat_directory_item) * parentdir->total);
    }

    parentdir->total++;

    res = diskstreamer_seek(priv->cluster_read_stream, disk_loc);
    if(res < 0)
        goto out;
    
    res = diskstreamer_write(priv->cluster_read_stream, &tmp, sizeof(struct fat_directory_item));


out:
    fat16_fat_item_free(parent_dir_item);
    if(res < 0)
        fat16_erase_cluster_chain(disk, first_cluster);
    return res;
}
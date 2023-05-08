#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "disk/disk.h"
#include "fat/fat16.h"
#include "fat32/fat32.h"
#include "status.h"
#include "graphics/graphics.h"
struct filesystem* filesystems[SERPAEOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[SERPAEOS_MAX_FILE_DESCRIPTORS];
struct directory_descriptor *dir_descriptors[SERPAEOS_MAX_DIRECTORY_DESCRIPTORS];

static struct filesystem** fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < SERPAEOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}

void fs_insert_filesystem(struct filesystem* filesystem)
{
    struct filesystem** fs;
    fs = fs_get_free_filesystem();
    if (!fs)
    {
        print("Problem inserting filesystem"); 
        while(1);
    }

    *fs = filesystem;
}

static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
    fs_insert_filesystem(fat32_init());
}

void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static void file_free_descriptor(struct file_descriptor* desc)
{
    file_descriptors[desc->index-1] = 0x00;
    kfree(desc);
}

static int file_new_descriptor(struct file_descriptor** desc_out)
{
    int res = -ENOMEM;
    for (int i = 0; i < SERPAEOS_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

struct file_descriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= SERPAEOS_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }

    // Descriptors start at 1
    int index = fd - 1;
    return file_descriptors[index];
}

static void directory_free_descriptor(struct directory_descriptor* desc)
{
    dir_descriptors[desc->index - 1] = 0;
    kfree(desc);
}

static int directory_new_descriptor(struct directory_descriptor** desc_out)
{
    int res = -ENOMEM;
    for(int i = 0; i < SERPAEOS_MAX_DIRECTORY_DESCRIPTORS; i++)
    {
        if(dir_descriptors[i] == 0)
        {
            struct directory_descriptor* desc = kzalloc(sizeof(struct directory_descriptor));

            desc->index = i + 1;
            dir_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }
    return res;
}

struct directory_descriptor *directory_get_descriptor(int dd)
{
    if(dd <= 0 || dd >= SERPAEOS_MAX_DIRECTORY_DESCRIPTORS)
        return 0;

    dd--;
    return dir_descriptors[dd];
}

struct filesystem* fs_resolve(struct disk* disk)
{
    struct filesystem* fs = 0;
    for (int i = 0; i < SERPAEOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    if(!fs)
    {
        print("\nKERNEL ERROR (non fatal): disk filesystem cannot be initiated (is disk partitioned?)");
    }

    return fs;
}

FILE_MODE file_get_mode_by_string(char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    while(*str)
    {
        if(*str == 'r' || *str == 'R')
            mode |= FILE_MODE_READ;
        else if(*str == 'w' || *str == 'W')
            mode |= FILE_MODE_WRITE;
        else if(*str == 'a' || *str == 'A')
            mode |= FILE_MODE_APPEND;
        str++;
    }
    return mode;
}

int fopen(const char* filename, const char* mode_str)
{
    int res = 0;
    void* descriptor_private_data = 0;
    struct path_root* root_path = pathparser_parse(filename, NULL);
    if (!root_path)
    {
        res = -EINVARG;
        goto out;
    }

    // We cannot have just a root path 0:/
    if (!root_path->first)
    {
        res = -EINVARG;
        goto out;
    }

    // Ensure the disk we are reading from exists
    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk)
    {
        res = -EIO;
        goto out;
    }

    if (!disk->filesystem)
    {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string((char *)mode_str);
    if (mode == FILE_MODE_INVALID)
    {
        res = -EINVARG;
        goto out;
    }

    descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data))
    {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0)
    {
        goto out;
    }
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    desc->mode = mode;
    res = desc->index;
    kfree(root_path);

out:
    // fopen shouldnt return negative values
    if (res < 0)
    {
        if(root_path)
            pathparser_free(root_path);
        if(descriptor_private_data)
            kfree(descriptor_private_data);
    }

    return res;
}

int fstat(int fd, file_stat* stat)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->stat(desc->disk, desc->private, stat);
out:
    return res;
}

int fclose(int fd)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->close(desc->private);
    if (res == SERPAEOS_ALL_OK)
    {
        file_free_descriptor(desc);
    }
out:
    return res;
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->seek(desc->private, offset, whence);
out:
    return res;
}
int fread(int fd, void* ptr, int size)
{
    int res = 0;
    if (size == 0 || fd < 1)
    {
        res = -EINVARG;
        goto out;
    }

    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EINVARG;
        goto out;
    }

    res = desc->filesystem->read(desc->disk, desc->private, size, 1, (char *)ptr);
out:
    return res;
}

int fwrite(int fd, void *buf, int size)
{
    if(fd <= 0 || buf == NULL || size == 0)
        return -EINVARG;
    
    struct file_descriptor *desc = file_get_descriptor(fd);
    if(!desc)
        return -EINVARG;
    
    return desc->filesystem->write(desc->disk, desc->private, buf, size);
}

int ftell(int fd)
{
    if(fd <= 0)
        return -EINVARG;
    
    struct file_descriptor *desc = file_get_descriptor(fd);
    if(!desc)
        return -EINVARG;
    
    return desc->filesystem->ftell(desc->private);
}

int fdelete(int fd)
{
    if(fd < 1)
        return -EINVARG;
    struct file_descriptor *desc = file_get_descriptor(fd);
    if(!desc)
        return -EINVARG;
    int res = desc->filesystem->delete(desc->disk, desc->private);
    file_free_descriptor(desc);
    return res;
}
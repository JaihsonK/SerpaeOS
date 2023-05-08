#ifndef FILE_H
#define FILE_H

#include "pparser.h"
#include "time/time.h"
#include <stdint.h>

typedef unsigned int FILE_SEEK_MODE;
enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END,
    SEEK_NEG
};


typedef unsigned char FILE_MODE;
enum
{ 
    FILE_MODE_READ = 1,
    FILE_MODE_WRITE = 2,
    FILE_MODE_APPEND = 4,
    FILE_MODE_INVALID = 0
};

enum
{
    FILE_STAT_READ_ONLY = 0b00000001
};

typedef unsigned int FILE_STAT_FLAGS;

struct disk;
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

typedef int (*FS_CLOSE_FUNCTION)(void* private);

typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);
typedef int (*FS_RENAME_FUNCTION)(struct disk *disk, struct path_part *path);

typedef int (*FS_WRITE_FUNCTION)(struct disk *disk, void *descriptor, char *buf, int size);

typedef int (*FS_DELETE_FUNCTION)(struct disk *disk, void *descriptor);

struct filestat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
    struct date_time creation_datetime;
    struct date_time last_access_datetime;
};
typedef struct filestat file_stat;

typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, file_stat* stat);
typedef int (*FS_FTELL)(void *private);

struct filesystem
{
    // Filesystem should return zero from resolve if the provided disk is using its filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
    FS_RENAME_FUNCTION rename;
    FS_WRITE_FUNCTION write;
    FS_DELETE_FUNCTION delete;
    FS_FTELL ftell;
    char name[20];
};

struct file_descriptor
{
    // The descriptor index
    int index;
    struct filesystem* filesystem;

    // Private data for internal file descriptor
    void* private;

    // The disk that the file descriptor should be used on
    struct disk* disk;

    FILE_MODE mode;
};

#define DIRENT_TYPE_FILE 1
#define DIRENT_TYPE_SUBDIR 2

struct dir_entry
{
    char name[20];
    int type; //subdir or file
};

struct directory_descriptor
{
    int index;

    struct filesystem *filesystem;

    int count;

    struct dir_entry *entries;
};


void fs_init();
int fopen(const char* filename, const char* mode_str);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fread(int fd, void* ptr, int size);
int fstat(int fd, file_stat* stat);
int fclose(int fd);
int fwrite(int fd, void *buf, int size);
int ftell(int fd);
int fdelete(int fd);

void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);

#endif
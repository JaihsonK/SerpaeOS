#ifndef STDIO_H
#define STDIO_H

#include <stdbool.h>
#include "cursor.h"

#define EOF (-1)
#define SIGINT (-2)

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define KEY_DOWN 220
#define KEY_UP 221
#define KEY_RIGHT 223
#define KEY_LEFT 224

struct date_time
{
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char day;
    unsigned char month;
    unsigned short year;
};

struct filestat
{
    unsigned flags;
    unsigned filesize;
    struct date_time creation_datetime;
    struct date_time last_access_datetime;
};
typedef struct filestat file_stat;

typedef int FILE;

enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END,
    SEEK_NEG
};


int putchar(int ch);
int printf(const char *fmt, ...);
int puts(const char *str);

int getline(char *out, int max);
int getkey();

int system(const char* filename);

char* gets(char* out);
int getchar();
void clrscr(int colour);

FILE* fopen(char* filename, char* mode);

int fclose(FILE* fp);
int fread(FILE* fp, int size, void* ptr);
int fwrite(FILE *fp, int size, void *ptr);


int scanf(const char* fmt, ...);
int fstat(FILE *fp, file_stat *stat);
int ftell(FILE *fp);
int fseek(FILE *fp, int offset, unsigned whence);
int fgetline(FILE *fp, char *out, int max);
char getc(FILE *fp);
void remove(char *filename);

#ifndef STDLIB_H
extern void exit(int);
#endif

#endif
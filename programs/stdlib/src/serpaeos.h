#ifndef SERPAEOS_H
#define SERPAEOS_H
#include <stddef.h>
#include "stdio.h"

struct command_argument
{
    char argument[512];
    struct command_argument* next;
};

struct process_arguments
{
    int argc;
    char** argv;
};

typedef unsigned thread_t;

void* sos_malloc(size_t size);
void sos_free();
void sos_putchar(char c);
void sos_process_load(char* filenme);
struct command_argument* sos_parse_command(const char* command, int max);
void sos_process_get_arguments();
int sos_system(struct command_argument* arg);
void sos_exit(int res);

int kill(int id);
int sos_retcode(int id);
int sos_readsector(int sector, int disk, void *buf);
int sos_writesector(int sector, int disk, void *buf);

thread_t newthread(void *addr, ...);

void play(unsigned int hertz);
void quit_sound();

#endif
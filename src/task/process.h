#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include <stdbool.h>
#include "task.h"
#include "config.h"
#include "kernel.h"

typedef unsigned char PROCESS_FILETYPE;
#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1
#define PROCESS_FILETYPE_DRIVER 2

typedef int FILE;
#define NFILE 20

struct screen
{
    bool current_screen;
    int x;
    int y;
    uint16_t video[VGA_HEIGHT * VGA_WIDTH];
};

typedef struct
{
    void* ptr;
    size_t size;
}allocation;

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


struct process
{
    struct process *host;//ptr to host process, or NULL if this process doesn't have host
    bool currently_hosting;

    struct process *parent; //NULL if this is a parent process, pointer to parent

    // The process ID.
    uint32_t id;

    char filename[SERPAEOS_MAX_PATH];

    // The main process task
    struct task* task;

    // The memory (malloc) allocations of the process
    allocation allocations[SERPAEOS_MAX_PROGRAM_ALLOCATIONS];

    PROCESS_FILETYPE filetype;

    union
    {
        // The physical pointer to the process memory.
        void* ptr;
        struct elf_file* elf_file;
    };
    

    // The physical pointer to the stack memory
    void* stack;

    // The size of the data pointed to by "ptr"
    uint32_t size;

    struct keyboard_buffer
    {
        char buffer[SERPAEOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;
    struct process_arguments arguments;
    
    struct screen *screen;
    FILE iob[NFILE];

    bool priv;
};

int process_switch(struct process* process);
int process_load_switch(const char *filename, struct process **process, struct process *hosting);
int process_load(const char *filename, struct process **process, struct process *hosting);
int process_load_for_slot(const char *filename, struct process **process, int process_slot, struct process *hosting);
struct process* process_current();
struct process* process_get(int process_id);
void* process_malloc(struct process* process, size_t size);
void process_free(struct process* process, void* ptr);
void process_get_arguments(struct process* process, int* argc, char*** argv);
int process_inject_arguments(struct process* process, struct command_argument* root);
int process_terminate(struct process* process);
void process_print(char* message, struct process* process);
void process_flip(int id);
void hide_directory();
void show_directory();
FILE *process_fopen(struct process *process, char *filename, char* mode);
void process_writechar(char c, char colour, struct screen* screen);
int process_fclose(FILE* fp);
int process_fork();

#endif
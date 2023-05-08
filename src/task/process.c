#include "process.h"
#include "config.h"
#include "status.h"
#include "task/task.h"
#include "memory/memory.h"
#include "string/string.h"
#include "fs/file.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"
#include "kernel.h"
#include "power/power.h"
#include "disk/disk.h"
#include "time/time.h"
#include "graphics/graphics.h"
#include "idt/idt.h"
#include <stdbool.h>
#include <stdint.h>

#define process_init(p) memset(p, 0, sizeof(struct process))

// The current process that is running
struct process *current_process = 0;

static struct process *processes[SERPAEOS_TOTAL_MAX_PROCESSES]; // first 13 entries are for main processes.
int ret_codes[SERPAEOS_MAX_PROCESSES];                          // return codes of main processes;

struct process *process_current()
{
    return current_process;
}

struct process *process_get(int process_id)
{
    if (process_id < 0 || process_id >= SERPAEOS_TOTAL_MAX_PROCESSES)
    {
        return NULL;
    }

    return processes[process_id];
}

static void process_save_video(struct process *process)
{
    memcpy(process->screen->video, (void *)0xb8000, sizeof(process->screen->video));
    process->screen->x = get_cursor()[0];
    process->screen->y = get_cursor()[1];
    process->screen->current_screen = false;
}

static void process_load_video(struct process *process)
{
    memcpy((void *)0xb8000, process->screen->video, sizeof(process->screen->video));
    set_cursor(process->screen->x, process->screen->y);
    process->screen->current_screen = true;
}

int process_switch(struct process *process)
{
    // switch video
    process_save_video(current_process);
    process_load_video(process);

    current_process = process;
    return 0;
}

void process_flip(int id)
{
    if (id < 0 || id > SERPAEOS_MAX_PROCESSES || !processes[id])
        return;
    hide_directory();
    process_switch(processes[id]);
    task_switch(current_process->task);
    task_return(&task_current()->registers);
}

static void process_terminate_allocs(struct process *process)
{
    for (int i = 0; i < SERPAEOS_MAX_PROGRAM_ALLOCATIONS; i++)
        kfree(process->allocations[i].ptr);
}

static int process_free_program_data(struct process *process)
{
    switch (process->filetype)
    {
    case PROCESS_FILETYPE_BINARY:
        kfree(process->ptr);
        break;
    case PROCESS_FILETYPE_ELF:
        elf_close(process->elf_file);
        break;
    default:
        return -EINVARG;
        break;
    }

    return 0;
}

static void process_switch_to_any()
{
    for (int i = 0; i < SERPAEOS_MAX_PROCESSES; i++)
        if (processes[i])
        {
            process_switch(processes[i]);
            return;
        }
    panic("no process to switch to");
}

static void process_unlink(struct process *process)
{
    processes[process->id] = 0;
    kfree(process);
    if (current_process == process)
    {
        process_switch_to_any();
    }
}

static void process_terminate_files(FILE *iob)
{
    for (int i = 0; i < NFILE; i++)
        if (iob[i] == 0)
            process_fclose(&iob[i]);
}
/*
void process_terminate_children(struct process *parent)
{
    for(struct process *current = processes[13]; current <= processes + SERPAEOS_TOTAL_MAX_PROCESSES; current++)
    {
        if(current->parent == parent)
            process_terminate(current);
    }
}
*/
int process_terminate(struct process *process)
{
    if (!process)
        return -EINVARG;
    if (process->id == 0)
        return -EINVARG;

    process_terminate_allocs(process);

    process_terminate_files(process->iob);

    int res = process_free_program_data(process);
    if (res < 0)
        return res;

    kfree(process->stack);

    task_free_all_for_process(process);

    if (process->host)
        process->host->currently_hosting = false;

    kfree(process->screen);

    process_unlink(process);

    return 0;
}

void process_get_arguments(struct process *process, int *argc, char ***argv)
{
    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

int process_count_command_arguments(struct command_argument *root)
{
    int i = 0;
    struct command_argument *current = root;
    while (current)
    {
        i++;
        current = current->next;
    }
    return i;
}

int process_inject_arguments(struct process *process, struct command_argument *root)
{
    int res = 0;
    struct command_argument *current = root;
    int i = 0;
    int argc = process_count_command_arguments(root);

    if (argc == 0)
        return -EIO;

    char **argv = process_malloc(process, sizeof(const char *) * argc);
    if (!argv)
    {
        res = -ENOMEM;
        goto out;
    }
    while (current)
    {
        char *argument_str = process_malloc(process, sizeof(current->argument));
        if (!argument_str)
        {
            res = -ENOMEM;
            goto out;
        }

        strncpy(argument_str, current->argument, sizeof(current->argument));
        argv[i] = argument_str;
        current = current->next;
        i++;
    }

    process->arguments.argc = argc;
    process->arguments.argv = argv;

out:
    return res;
}

static int find_free_alloc_index(struct process *process)
{
    int res = -ENOMEM;
    for (int i = 0; i < SERPAEOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == 0)
        {
            res = i;
            break;
        }
    }

    return res;
}

void *process_malloc(struct process *process, size_t size)
{
    void *ptr = kzalloc(size);
    if (!ptr)
    {
        goto out_err;
    }

    int index = find_free_alloc_index(process);
    if (index < 0)
    {
        goto out_err;
    }

    int res = paging_map_to(process->task->page_directory, ptr, ptr, paging_align_address(ptr + size), PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (res < 0)
    {
        goto out_err;
    }

    process->allocations[index].ptr = ptr;
    process->allocations[index].size = size;
    return ptr;
out_err:
    if (ptr)
    {
        kfree(ptr);
    }
    return 0;
}

static void process_allocation_unjoin(struct process *process, void *ptr)
{
    for (int i = 0; i < SERPAEOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == ptr)
        {
            process->allocations[i].ptr = 0x00;
            process->allocations[i].size = 0;
        }
    }
}

static allocation *process_get_allocation_by_addr(struct process *process, void *addr)
{
    for (int i = 0; i < SERPAEOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == addr)
            return &process->allocations[i];
    }

    return 0;
}

void process_free(struct process *process, void *ptr)
{
    // Unlink the pages from the process for the given address
    allocation *allocation = process_get_allocation_by_addr(process, ptr);
    if (!allocation)
    {
        // Oops, it's not our pointer.
        return;
    }

    int res = paging_map_to(process->task->page_directory, allocation->ptr, allocation->ptr, paging_align_address(allocation->ptr + allocation->size), 0x00);
    if (res < 0)
    {
        return;
    }

    // Unjoin the allocation
    process_allocation_unjoin(process, ptr);

    // We can now free the memory.
    kfree(ptr);
}
static int process_load_binary(const char *filename, struct process *process)
{
    int res = 0;
    int fd = fopen(filename, "r");
    if (!fd)
    {
        res = -EIO;
        goto out;
    }

    file_stat stat;
    res = fstat(fd, &stat);
    if (res != SERPAEOS_ALL_OK)
    {
        goto out;
    }

    void *program_data_ptr = kzalloc(stat.filesize);
    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    if (fread(fd, program_data_ptr, stat.filesize) < 0)
    {
        res = -EIO;
        kfree(program_data_ptr);
        goto out;
    }

    process->filetype = PROCESS_FILETYPE_BINARY;
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    fclose(fd);
    return res;
}

static int process_load_elf(const char *filename, struct process *process)
{
    int res = 0;
    struct elf_file *elf_file = 0;
    res = elf_load(filename, &elf_file);
    if (ISERR(res))
    {
        goto out;
    }

    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = elf_file;
out:
    return res;
}
static int process_load_data(const char *filename, struct process *process)
{
    int res = 0;
    res = process_load_elf(filename, process);
    if (res == -EINFORMAT)
    {
        res = process_load_binary(filename, process);
    }

    return res;
}

int process_map_binary(struct process *process)
{
    int res = 0;
    paging_map_to(process->task->page_directory, (void *)SERPAEOS_PROGRAM_VIRTUAL_ADDRESS, process->ptr, paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}

static int process_map_elf(struct process *process)
{
    int res = 0;

    struct elf_file *elf_file = process->elf_file;
    struct elf_header *header = elf_header(elf_file);
    struct elf32_phdr *phdrs = elf_pheader(header);
    for (int i = 0; i < header->e_phnum; i++)
    {
        struct elf32_phdr *phdr = &phdrs[i];
        void *phdr_phys_address = elf_phdr_phys_address(elf_file, phdr);
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
        if (phdr->p_flags & PF_W)
        {
            flags |= PAGING_IS_WRITEABLE;
        }
        res = paging_map_to(process->task->page_directory, paging_align_to_lower_page((void *)phdr->p_vaddr), paging_align_to_lower_page(phdr_phys_address), paging_align_address(phdr_phys_address + phdr->p_memsz), flags);
        if (ISERR(res))
        {
            break;
        }
    }
    return res;
}
int process_map_memory(struct process *process)
{
    int res = 0;

    switch (process->filetype)
    {
    case PROCESS_FILETYPE_ELF:
        res = process_map_elf(process);
        break;

    case PROCESS_FILETYPE_BINARY:
        res = process_map_binary(process);
        break;

    default:
        panic("process_map_memory: Invalid filetype\n");
    }

    if (res < 0)
    {
        goto out;
    }

    // Finally map the stack
    paging_map_to(process->task->page_directory, (void *)SERPAEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack, paging_align_address(process->stack + SERPAEOS_USER_PROGRAM_STACK_SIZE), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
out:
    return res;
}

int process_get_free_slot()
{
    for (int i = 0; i < SERPAEOS_MAX_PROCESSES; i++)
    {
        if (processes[i] == 0)
            return i;
    }

    return -EISTKN;
}

int process_get_child_slot()
{
    for (int i = 13; i < SERPAEOS_TOTAL_MAX_PROCESSES; i++)
    {
        if (processes[i] == 0)
            return i;
    }
    return -EISTKN;
}

int process_load(const char *filename, struct process **process, struct process *hosting)
{
    int res = 0;
    int process_slot = process_get_free_slot();
    if (process_slot < 0)
    {
        res = -EISTKN;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot, hosting);
out:
    return res;
}

extern void terminal_initialize();
extern uint16_t terminal_col, terminal_row;

int process_load_switch(const char *filename, struct process **process, struct process *hosting)
{
    int res = process_load(filename, process, hosting);
    if (res == 0)
    {
        struct process *target = *process;

        process_switch(target);
        if (!target->host) // if not hosting another
            terminal_initialize();
    }

    return res;
}

int process_load_for_slot(const char *filename, struct process **process, int process_slot, struct process *hosting)
{
    int res = 0;
    struct task *task = 0;
    struct process *_process;
    void *program_stack_ptr = 0;

    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    _process = kzalloc(sizeof(struct process));
    if (!_process)
    {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);

    res = process_load_data(filename, _process);
    if (res < 0)
    {
        goto out;
    }

    program_stack_ptr = kzalloc(SERPAEOS_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    // Create a task
    task = task_new(_process);
    if (ERROR_I(task) == 0)
    {
        res = ERROR_I(task);
        goto out;
    }

    _process->task = task;

    res = process_map_memory(_process);
    if (res < 0)
    {
        goto out;
    }

    _process->currently_hosting = false;
    _process->host = hosting;
    if (hosting)
    {
        _process->screen = hosting->screen;
        hosting->currently_hosting = true;
    }
    else
        _process->screen = kzalloc(sizeof(struct screen));

    *process = _process;

    // Add the process to the array
    processes[process_slot] = _process;

out:
    if (ISERR(res))
    {
        if (_process && _process->task)
        {
            task_free(_process->task);
        }

        // Free the process data
        process_free_program_data(_process);
    }
    return res;
}

static void process_scroll_down(struct screen *screen)
{
    for (int i = 1; i < VGA_HEIGHT; i++)
    {
        memcpy(&screen->video[(i - 1) * 80], &screen->video[i * 80], VGA_WIDTH * sizeof(uint16_t));
    }
    for (int i = 0; i < VGA_WIDTH; i++)
        screen->video[(24 * VGA_WIDTH) + i] = terminal_make_char(' ', terminal_get_attr());
}

static void process_backspace(struct screen *screen);

void process_writechar(char c, char colour, struct screen *screen)
{
    if (c == '\n' || c == '\r')
    {
        if (screen->y == 24) // last row
            process_scroll_down(screen);
        else
            screen->y++;
        screen->x = 0;
        return;
    }
    if (c == '\f')
    {
        if (screen->y == 24) // last row
            process_scroll_down(screen);
        else
            screen->y++;
        return;
    }
    if (c == '\b')
    {
        process_backspace(screen);
        return;
    }

    if (screen->x == 79 && screen->y == 24)
        process_scroll_down(screen);

    screen->video[(screen->y * VGA_WIDTH) + screen->x] = terminal_make_char(c, colour);
    screen->x += 1;
    if (screen->x >= VGA_WIDTH)
    {
        screen->x = 0;
        screen->y += 1;
    }
}

static void process_backspace(struct screen *screen)
{
    if (screen->x == 0 && screen->y == 0)
        return;
    if (screen->x == 0)
    {
        screen->y--;
        screen->x = VGA_WIDTH;
    }
    else
        screen->x--;

    process_writechar(' ', terminal_get_attr(), screen);
    screen->x--;
}

void process_print(char *message, struct process *process)
{
    if (process->screen->current_screen)
        print(message);
    else
        for (int i = 0; i < strlen(message); i++)
            process_writechar(message[i], terminal_get_attr(), process->screen);
}

static struct screen directory_screen;
static bool directory_screen_is_current;
extern bool dir_on;
void show_directory()
{
    memcpy(directory_screen.video, (void *)0xb8000, sizeof(directory_screen.video));
    directory_screen.x = get_cursor()[0];
    directory_screen.y = get_cursor()[1];
    directory_screen_is_current = true;

    set_cursor(0, 0);

    clrscr(15);
    char *numbers[SERPAEOS_MAX_PROCESSES] = {"Home", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};

    print("SerpaeOS v23.4.1         symbols: \"+\" = sleeping process, \"*\" = current process");
    for (int i = 0; i < SERPAEOS_MAX_PROCESSES; i++)
        if (processes[i])
        {
            terminal_writechar('\n', 15);
            print(numbers[i]);
            terminal_writechar(':', terminal_get_attr());

            if (current_process == processes[i])
                print(" *");
            else if (processes[i]->currently_hosting)
                print(" +");
            else
                print("  ");
            print(processes[i]->filename);
        }

    set_cursor(60, 24);

    char *months[13] = {"INVALID ", "January ", "Febuary ", "March ", "April ", "May ", "June ", "July ", "August ", "September ", "October ", "November ", "December "};

    print(months[get_current_time()->month]);
    print(itoa(get_current_time()->day));
    print(", ");
    print(itoa(get_current_time()->year));
}

void hide_directory()
{
    if (!directory_screen_is_current) // not showing diectory screen
        return;
    memcpy((void *)0xb8000, directory_screen.video, sizeof(directory_screen.video));
    set_cursor(directory_screen.x, directory_screen.y);
    directory_screen_is_current = false;
}

FILE *process_fopen(struct process *process, char *filename, char *mode)
{
    FILE *fp;
    for (fp = process->iob; fp < process->iob + NFILE; fp++)
        if (*fp == 0)
            break;
    if (fp >= process->iob + NFILE)
        return 0;

    int fd = fopen(filename, mode);
    if (!fd)
        return 0;

    *fp = fd;

    return fp;
}

int process_fclose(FILE *fp)
{
    fclose(*fp);
    *fp = 0;
    return 0;
}

#include "io.h"
#include "task/task.h"
#include "keyboard/keyboard.h"
#include "fs/file.h"
#include "task/process.h"
#include "memory/heap/kheap.h"
#include "disk/disk.h"
#include "kernel.h"
#include "graphics/graphics.h"
#include "sound/sound.h"
#include "status.h"

func(isr80h_command1_print)
{
    void *user_space_msg_buffer = task_get_stack_item(task_current(), 0);
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));
    // print(buf);
    process_print(buf, task_current()->process);
    return 0;
}

func(isr80h_command2_getkey)
{
    char c = keyboard_pop();
    return (void *)((int)c);
}

func(isr80h_command3_putchar)
{
    char c = (char)(int)task_get_stack_item(task_current(), 0);
    char buf[2] = {c, 0};
    process_print(buf, task_current()->process);
    return 0;
}

func(isr80h_command9_clrscr)
{
    int colour = (int)task_get_stack_item(task_current(), 0);
    struct process *process = task_current()->process;
    if (process->screen->current_screen)
        clrscr(colour);
    else
    {
        process->screen->x = 0;
        process->screen->y = 0;
        for (int i = 0; i < VGA_HEIGHT; i++)
            for (int j = 0; j < VGA_WIDTH; j++)
                process_writechar(' ', colour, process->screen);
        process->screen->x = 0;
        process->screen->y = 0;
    }
    return 0;
}

func(isr80h_command10_fopen)
{
    void *user_space_filename_buffer = task_get_stack_item(task_current(), 1);
    void *user_space_mode_buffer = task_get_stack_item(task_current(), 0);
    char filename[1024];
    char mode_buf[10];
    char *mode = mode_buf;
    copy_string_from_task(task_current(), user_space_filename_buffer, filename, 1024);
    copy_string_from_task(task_current(), user_space_mode_buffer, mode, 10);

    struct process *process = task_current()->process;

    FILE *fp = process_fopen(process, filename, mode);

    return (void *)(fp);
}

func(isr80h_command11_fclose)
{
    FILE *fp = task_get_stack_item(task_current(), 0);

    return (void *)process_fclose(fp);
}

func(isr80h_command12_fread)
{
    FILE *fp = (FILE *)task_get_stack_item(task_current(), 0);
    if (fp < task_current()->process->iob || fp > (task_current()->process->iob + NFILE))
        return ERROR(-EINVARG);
    uint32_t count = (uint32_t)task_get_stack_item(task_current(), 1);
    char *out = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 2));

    return (void *)fread(*fp, out, count);
}

func(isr80h_command13_fwrite)
{
    FILE *fp = (FILE *)task_get_stack_item(task_current(), 0);
    if (fp < task_current()->process->iob || fp > (task_current()->process->iob + NFILE))
        return ERROR(-EINVARG);
    uint32_t count = (uint32_t)task_get_stack_item(task_current(), 1);
    char *buf = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 2));

    return (void *)fwrite(*fp, buf, count);
}

func(isr80h_command18_write_sector)
{
    if (task_current()->process->priv == false)
        return ERROR(-ENOTPRIV);
    int sector_num = (int)task_get_stack_item(task_current(), 0);
    struct disk *disk = disk_get((int)task_get_stack_item(task_current(), 1));
    if (!disk)
        return ERROR(-EINVARG);
    char *buf = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 2));
    return (void *)disk_write_block(disk, sector_num, 1, buf);
}
func(isr80h_command19_read_sector)
{
    if (task_current()->process->priv == false)
        return ERROR(-ENOTPRIV);
    int sector_num = (int)task_get_stack_item(task_current(), 0);
    struct disk *disk = disk_get((int)task_get_stack_item(task_current(), 1));
    if (!disk)
        return ERROR(-EINVARG);
    char *buf = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 2));
    return (void *)disk_read_block(disk, sector_num, 1, buf);
}

func(isr80h_command20_fstat)
{
    FILE *fp = task_get_stack_item(task_current(), 0);
    if (fp < task_current()->process->iob || fp > (task_current()->process->iob + NFILE))
        return ERROR(-EINVARG);
    file_stat *stat = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 1));
    return (void *)fstat(*fp, stat);
}

func(isr80h_command21_ftell)
{
    FILE *fp = task_get_stack_item(task_current(), 0);
    if (fp < task_current()->process->iob || fp > (task_current()->process->iob + NFILE))
        return ERROR(-EINVARG);
    return (void *)ftell(*fp);
}

func(isr80h_command22_fseek)
{
    FILE *fp = task_get_stack_item(task_current(), 0);
    if (fp < task_current()->process->iob || fp > (task_current()->process->iob + NFILE))
        return ERROR(-EINVARG);
    int offset = (int) task_get_stack_item(task_current(), 1);
    unsigned whence = (unsigned) task_get_stack_item(task_current(), 2);

    return (void *)fseek(*fp, offset, whence);
}

func(isr80h_command23_remove)
{
    void *user_space_filename_buffer = task_get_stack_item(task_current(), 0);
    char filename[1024];
    copy_string_from_task(task_current(), user_space_filename_buffer, filename, 1024);

    int fd = fopen(filename, "rw");
    if(fd < 1)
        return 0;
    fdelete(fd);
    return 0;
}

func(isr80h_command24_set_cursor)
{
    uint16_t y = (uint16_t) ((uint32_t)task_get_stack_item(task_current(), 0));
    uint16_t x = (uint16_t) ((uint32_t)task_get_stack_item(task_current(), 1));
    set_cursor(x, y);
    return 0;
}

func(isr80h_command25_draw_block)
{
    uint8_t attribute = (uint8_t) ((uint32_t)task_get_stack_item(task_current(), 0));
    uint16_t y = (uint16_t) ((uint32_t)task_get_stack_item(task_current(), 1));
    uint16_t x = (uint16_t) ((uint32_t)task_get_stack_item(task_current(), 2));

    graphics_put(attribute, x, y);
    return 0;
}

func(isr80h_command26_play_sound)
{
    uint32_t hertz = (uint32_t)task_get_stack_item(task_current(), 0);
    play(hertz);
    return 0;
}

func(isr80h_command27_quit_sound)
{
    quit_sound();
    return 0;
}
#include "misc.h"
#include "idt/idt.h"
#include "task/task.h"
#include "task/process.h"
#include "power/power.h"
#include "memory/heap/kheap.h"

#define func(x) void* x (struct interrupt_frame *frame)

extern int ret_codes[SERPAEOS_MAX_PROCESSES];

func(isr80h_command0_exit)
{
    struct process* process = task_current()->process;
    if(process->id == 0)
        acpi_poweroff();
    int res = (int)task_get_stack_item(task_current(), 0);
    ret_codes[process->id] = res;
    process_terminate(process);
    task_next();
    return 0;
}
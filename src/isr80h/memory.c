#include "memory.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

#define func(x) void* x (struct interrupt_frame *frame)

func(isr80h_command4_malloc)
{
    size_t size = (int)task_get_stack_item(task_current(), 0);
    return process_malloc(task_current()->process, size);
}


func(isr80h_command5_free)
{
    void* ptr_to_free = task_get_stack_item(task_current(), 0);
    process_free(task_current()->process, ptr_to_free);
    return 0;
}

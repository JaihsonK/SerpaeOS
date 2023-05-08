#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "graphics/graphics.h"
#include "config.h"
#include "status.h"

#define func(x) void* x (struct interrupt_frame *frame)

extern int ret_codes[SERPAEOS_MAX_PROCESSES];


func(isr80h_command6_process_load_start)
{
    void *filename_userspace = task_get_stack_item(task_current(), 0);
    char filename[SERPAEOS_MAX_PATH];

    int res = copy_string_from_task(task_current(), filename_userspace, filename, SERPAEOS_MAX_PATH);
    if (res < 0)
        return (void *)res;

    char path[SERPAEOS_MAX_PATH];
    strcpy(path, "0:/");
    strcpy(path + 3, filename);

    struct process *process;
    res = process_load_switch(path, &process, NULL);
    if (res < 0)
        return (void *)res;
    task_switch(process->task);
    task_return(&process->task->registers);

    return (void *)res;
}

static int run(char *program, char *location, struct process **process, int size, struct process *host)
{
    char path[SERPAEOS_MAX_PATH];
    strcpy(path, location);
    strncpy(path + size, program, SERPAEOS_MAX_PATH - size);

    return process_load_switch(path, process, host);
}
func(isr80h_command7_invoke_system_command)
{
    bool priv = false, hosted = false;
    struct command_argument *arguments = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 0));
    if (!arguments || strlen(arguments[0].argument) == 0)
    {
        return ERROR(-EINVARG);
    }

    struct command_argument *root_command_argument = &arguments[0];
    if(istrncmp("he", root_command_argument->argument, sizeof(root_command_argument->argument)) == 0)
    {
        hosted = true;
        root_command_argument = root_command_argument->next;
        print("\n");
    }
    if(istrncmp("0:/sys/bin/pr", task_current()->process->filename, SERPAEOS_MAX_PATH) == 0)
        priv = true;
    
    const char *program_name = root_command_argument->argument;

    struct process *process = 0;
    struct process *host = 0;
    if(hosted)
    {
        host = task_current()->process;
        task_current()->process->currently_hosting = true;
    }
    int res = 0, loc = 0;

loc_switch:
    switch (loc)
    {
    case 0:
        res = run((char *)program_name, "0:/sys/bin/", &process, 11, host);
        break;
    case 1:
        res = run((char *)program_name, "0:/usr/bin/", &process, 11, host);
        break;
    case 2:
        res = run((char *)program_name, NULL, &process, 0, host); //just run the path
        break;
    default:
        return ERROR(-EINVARG);
    }

    if (res != 0)
    {
        loc++;
        goto loc_switch;
    }

    res = process_inject_arguments(process, root_command_argument);
    if (res < 0)
    {
        return ERROR(-EINVARG);
    }
    process->priv = priv;
    task_switch(process->task);
    task_return(&process->task->registers);

    return 0;
}

func(isr80h_command8_get_program_arguments)
{
    struct process *process = task_current()->process;
    struct process_arguments *args = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 0));

    process_get_arguments(process, &args->argc, &args->argv);

    return 0;
}

func(isr80h_command14_remote_terminate)
{
    int id = (int)task_get_stack_item(task_current(), 0);
    if (id == task_current()->process->id)
        return ERROR(-EINVARG);
    if (task_current()->process->priv == false)
        return ERROR(-ENOTPRIV);
    if (id <= 0 || id > SERPAEOS_MAX_PROCESSES) // can't terminate master shell or non-existing processes
        return ERROR(-EINVARG);
    ret_codes[id] = SERPAEOS_REMOTE_DISCONNECTED;
    return (void *)process_terminate(process_get(id));
}

func(isr80h_command15_process_inf)
{
    if (task_current()->process->priv == false)
        return NULL;
    int id = (int)task_get_stack_item(task_current(), 0);
    struct process *process = process_get(id);
    if (!process)
        return NULL;
    if (paging_map_to(task_current()->page_directory, process, process, paging_align_address(process + sizeof(struct process)), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL) != 0)
        return NULL;
    return (void *)process;
}

uint32_t loadEAX(uint32_t value)
{
    return value;
}

func(isr80h_command16_new_thread)
{
    struct process *process = task_current()->process;

    void *virtual_start_addr = task_get_stack_item(task_current(), 0);
    int arg_count = (int)task_get_stack_item(task_current(), 1);
    uint32_t *arguments = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 2));

    struct task *new_task = task_add_to_process(process, virtual_start_addr);
    if(ISERR(new_task))
        return NULL;

    for(int p = arg_count - 1; p >= 0; p--)
        task_push_stack_item(new_task, arguments[p]);

    loadEAX((uint32_t) new_task);

    task_switch(new_task);
    task_return(&new_task->registers);

    return 0;
}

func(isr80h_command255_get_retcode)
{
    if (task_current()->process->priv == false)
        return ERROR(-ENOTPRIV);
    int process = (int)task_get_stack_item(task_current(), 0);
    return (void *)ret_codes[process];
}
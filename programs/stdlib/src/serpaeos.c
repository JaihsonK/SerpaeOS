#include "serpaeos.h"
#include "string.h"
#include <stdarg.h>

extern thread_t sos_newthread(void *addr, int arg_count, unsigned int *args); //all arguments can be casted to unsigned int

struct command_argument* sos_parse_command(const char* command, int max)
{
    strtok(NULL, NULL);
    struct command_argument* root_command = 0;
    char scommand[1025];
    if (max >= (int) sizeof(scommand))
    {
        return 0;
    }


    strncpy(scommand, command, sizeof(scommand));
    char* token = strtok(scommand, " ");
    if (!token)
    {
        goto out;
    }

    root_command = sos_malloc(sizeof(struct command_argument));
    if (!root_command)
    {
        goto out;
    }

    strncpy(root_command->argument, token, sizeof(root_command->argument));
    root_command->next = 0;


    struct command_argument* current = root_command;
    token = strtok(NULL, " ");
    while(token != 0)
    {
        struct command_argument* new_command = sos_malloc(sizeof(struct command_argument));
        if (!new_command)
        {
            break;
        }

        strncpy(new_command->argument, token, sizeof(new_command->argument));
        new_command->next = 0x00;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    }
out:
    return root_command;
}

thread_t newthread(void *addr, ...)
{
    va_list list, tmp;
    va_start(list, addr); //start list after addr argument
    va_start(tmp, addr);

    int count = 0, test = 1;
    while((test = va_arg(list, int)))
        count++;
    
    va_end(list);
    
    unsigned int *arguments = sos_malloc(count * sizeof(unsigned int));
    //copy arguments
    for(int i = 0; i < count; i++)
        arguments[i] = va_arg(tmp, unsigned int);
    va_end(tmp);

    return sos_newthread(addr, count, arguments);
}
#include"stdlib.h"
#include "serpaeos.h"


void* malloc(size_t size)
{
    return (void*)sos_malloc(size);
}

void free(void* ptr)
{
    sos_free(ptr);
}

typedef void (*FUNCTION)();

FUNCTION exit_funcs[20] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

int atexit(FUNCTION func)
{
    for(int i = 0; i < 20; i++)
        if(exit_funcs[i] == NULL)
        {
            exit_funcs[i] = func;
            break;
        }

    return 0;
}

void exit(int code)
{
    for(int i = 19; i >= 0; i--)
    {
        if(exit_funcs[i])
            exit_funcs[i]();
    }
    printf("\n>>>EXIT: status %d. Press any key to terminate", code);
    getchar();
    sos_exit(code);
}

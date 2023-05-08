#include "serpaeos.h"
#include "memory.h"

typedef void (*FUNCTION)();

extern FUNCTION exit_funcs[20];

extern int main(int argc, char** argv);

int c_start()
{
    struct process_arguments args;
    sos_process_get_arguments(&args);

    memset(exit_funcs, 0, sizeof(exit_funcs)); //make sure they're all NULL

    int res = main(args.argc, args.argv);
    
    clrscr(colour(WHITE, BLACK));
    return res;
}
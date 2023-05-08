#include "stdio.h"
#include "stdlib.h"
#include "cursor.h"

FILE *file;

void error(char *msg)
{
    printf("edit: ERROR: %s", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if(argc < 2)
        error("invalid usage\nUSAGE: edit [filename]");
    
    if(!(file = fopen(argv[1], "rw")))
        error("file is unaccessable");

    clrscr(colour(BLACK, WHITE));
    
    
    exit(EXIT_SUCCESS);
    return 0;
}

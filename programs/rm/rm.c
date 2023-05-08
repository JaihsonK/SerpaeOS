#include "stdio.h"
#include "string.h"

int main(int argc, char** argv)
{
    if(argc < 2)
        return -1;
    char path[108];
    char *cur = path + 3;
    for(int i = 1; i < argc; i++)
    {
        if(*argv[i] == '~')
        {
            strcpy(cur, "0:/usr/files");
            cur += 12;
            argv[i]++;
        }
        strcpy(cur, argv[i]);
        remove(path);
    }
    return 0;
}
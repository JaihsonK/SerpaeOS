#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("fapp: ERROR: invalid usage\n");
        while (getkey() != EOF)
            ;
        return -2;
    }
    char path[108];
    char *cur = path;
    if (*argv[1] == '~')
    {
        strcpy(cur, "0:/usr/files");
        argv[1]++;
        cur += 12;
    }
    strcpy(cur, argv[1]);

    FILE *fp = fopen(path, "rwa");
    if(!fp)
    {
        printf("fapp: ERROR: file %s is inaccessable\n", path);
        while (getkey() != EOF)
            ;
        return -1;
    }

    char buf[1024];
    int i = 0, c;
    while(1)
    {
        c = getkey();
        if(c == 0)
            continue;
        if(i == 1024 || c == EOF)
        {
            fwrite(fp, i, buf);
        }
        if(c == EOF)
            break;
        putchar(c);
        if(c != '\b')
            buf[i++] = c;
        else
            i--;
    }

    fclose(fp);

    return 0;
}

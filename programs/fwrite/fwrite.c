#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("fwrite: ERROR: invalid usage\n");
        while (getchar() != EOF)
            ;
        return -2;
    }
    char path[108];
    char *cur = path + 3;
    if (*argv[1] == '~')
    {
        strncpy(cur, "0:/usr/files", 12);
        argv[1]++;
        cur += 12;
    }
    strncpy(cur, argv[1], 108);

    FILE *fp = fopen(path, "rw");
    if(!fp)
    {
        printf("fwrite: ERROR: file %s is inaccessable\n", path);
        while (getchar() != EOF)
            ;
        return -1;
    }

    char buf[1024];
    int i = 0, c;
    while(1)
    {
        c = getchar();
        if(i == 1024 || c == '\e')
        {
            fwrite(fp, i, buf);
        }
        if(c == '\e')
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

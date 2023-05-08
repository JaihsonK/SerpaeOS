#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "memory.h"

FILE *fp;

int print_line()
{
    static char buf[81];
    char c;
    memset(buf, 0, 81);
    for(int i = 0; i < 80; i++)
    {
        c = getc(fp);
        if(c == '\n' || c == '\r')
        {
            buf[i] = c;
            break;
        }
        if(c == 0 || c == EOF)
        {
            buf[i] = '\n';
            printf(buf);
            return 1;
        }

        buf[i] = c;
    }
    printf(buf);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("view: ERROR: invalid usage\n");
        getchar();
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

    fp = fopen(path, "r");
    if (!fp)
    {
        printf("view: ERROR: file %s is inaccessable for reading\n", path);
        getchar();
        return -1;
    }
    
    while(1)
    {
        clrscr(colour(BLACK, WHITE));
        for(int x = 0; x < 24; x++)
            if(print_line())
            {
                while(getkey() != EOF);
                goto out;
            }
        set_cursor(0, 79);
        printf("---------PAGING: press 'c' to continue with the next page, 'q' to quit---------");
        char cm;
        while((cm = getchar()) != 'c' && cm != 'q');
        if(cm == 'q')
            break;
    }

out:
    fclose(fp);

    return 0;
}

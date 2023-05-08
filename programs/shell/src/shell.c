#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "memory.h"
#include "serpaeos.h"

const char help_page[] = "Shell help center\
\nYou are currently running SerpaeOS Shell v23.4.1. If this is an older version\
\nthan the Kernel version (release the super key), please update shell.\
\nBug reports, questions, and tips can be emailed to \"serpaeos.devers@gmail.com\"\
\nAll software that comes wih SerpaeOS is copyright (c) 2023 Jaihson Kresak\
\nAll software under GNU General Pubic Licese v2. See 0:/usr/files/LICENSE for \ndetails\
\n\
List of programs and commands:\
\nfapp        -    File Append\
\n    example: fapp ~/sample.txt\
\nview        -    View File\
\n    example: view ~/sample.txt\
\nrm          -    remove file(s)\
\n    example: rm ~/sample.txt ~/another.txt\
\nshell.elf   -    SerpaeOS Shell\
\npr          -    give privileged execution to program\
\n    example: pr shell.elf\
\nhelp        -    SerpaeOS Help Page (this)\
\nclear       -    clear he terminal\
\nkill        -    Kill another application (by process ID)\
\n    example: kill 5\
\n    note: find process ID by pressing and releasing the Windows Key\
\nrc          -    get return code (by process ID)\
\n    example: rc 5\
";

int main(int argc, char** argv)
{
    printf("SerpaeOS Shell v23.4.1");
    char *input = malloc(1024 * 2);
    char *buf = input + 1024;
    char* token;
    bool newline = true;
    while(1)
    {
        if(newline)
            putchar('\n');
        else
            newline = !newline;

        printf(">");

        strtok(NULL, NULL);
        gets(input);
        memcpy(buf, input, 1024);

        token = strtok(input, " ");
        if((token < input) || (token > input + 1024))
            continue;
        
        if(istrncmp("exit", token, 5) == 0)
        {
            printf("\nExiting...");
            goto exit;
        }
        if(istrncmp("help", token, 5) == 0)
        {
            clrscr(15);
            printf(help_page);
            continue;
        }
        if(istrncmp("about", token, 6) == 0 || istrncmp("man", token, 4) == 0)
        {
            system("view ~/ABOUT");
            continue;
        }
        if(istrncmp("clear", token, 6) == 0)
        {
            clrscr(15);
            newline = false;
            continue;
        }
        if(istrncmp("kill", token, 5) == 0)
        {
            int id = atoi(strtok(NULL, " "));
            int res = kill(id);
            if(res != 0)
                printf("\nERROR: kill: Process %d could not be killed (error code %d)", id, res);
            continue;
        }
        if(istrncmp("rc", token, 3) == 0)
        {
            int id = atoi(strtok(NULL, " "));
            int res = sos_retcode(id);
            printf("\nreturn code for process %d is %d", id, res);
            continue;
        }

        if(system(buf) < 0)
            printf("\nERROR: \"%s\" is not recognized as an operable program or command. Try \"man\"", token);
    }
exit:
    return 0;
}

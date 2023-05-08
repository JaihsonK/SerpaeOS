#include "stdio.h"
#include "string.h"

struct configeration
{
    unsigned char magic[4]; //S, O, S, 0x3E
    int timezone;
    char username[21];
    char password[31];
}__attribute__((packed)) config;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("[pr] ERROR: invalid usage\n");
        return -2;
    }

    FILE *fp = fopen("0:/sys/config/config.dat", "r");
    fseek(fp, 0x3c, SEEK_SET);
    fread(fp, sizeof(config), &config);

    char buf[30];
    char c;
    int i = 0;

    while(1)
    {
        printf("[%s] password: ", config.username);
        for (i = 0; i < sizeof(buf) - 1 && (c = getchar()) != '\r'; i++)
            buf[i] = c;
        buf[i] = 0;

        if (strcmp(config.password, buf) == 0)
            break; //password correct
        else if(strncmp("exit", buf, sizeof(buf)) == 0)
            return 0;
        putchar('\n');
    }
    char path[108] = "he ";
    char *curr = path + 3;
    for(int i = 1; i < argc; i++)
    {
        strcpy(curr, argv[i]);
        curr += strlen(argv[i]);
        *curr++ = ' ';
    }
    
    if(system(path) == -2)
        printf("\nERROR: \"%s\" is not a valid program name.", argv[1]);
    return 0;
}

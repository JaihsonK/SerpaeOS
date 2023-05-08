#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "serpaeos.h"

struct configeration
{
    unsigned char magic[4]; //S, O, S, 0x3E
    int timezone;
    char username[21];
    char password[31];
}__attribute__((packed));
struct configeration config;

const unsigned char magic[4] = {'S', 'O', 'S', 0x3E};

static char *get_s(char *out, int lim)
{
    char *tmp = out;
    unsigned char c, i;
    while ((c = getchar()) != '\r' && i < lim && c != EOF)
        if (c >= KEY_DOWN)
            continue;
        else if (c == '\b')
        {
            if (out == tmp)
                continue;
            *(--out) = 0;
            putchar('\b');
            i--;
        }
        else
        {
            *(out++) = (char)c;
            putchar('*');
            i++;
        }
    *out = 0;
    return tmp;
}
int main(int argc, char **argv)
{
    FILE *fd = fopen("0:/sys/config/config.dat", "rwa");
    if(!fd)
    {
        printf("ERROR: 0:/sys/config/config.dat in inaccesable");
        exit(EXIT_FAILURE);
    }

    char buf[10];
    char password1[31], password2[31];

    memset(&config, 0, sizeof(config));

    printf("It looks like you're a new user. Welcome to SerpaeOS!\nPlease enter your timezone from UTC (ie, enter -6 for Central time zone):\n");
    config.timezone = atoi(gets(buf));

    printf("\nGreat! Please enter your new username (20 character limit):\n");
    gets(config.username);

password:
    printf("\nPlease enter your new password (30 character limit): ");
    get_s(password1, 30);
    printf("\nPlease re-enter your password: ");
    get_s(password2, 30);
    if(strcmp(password1, password2) != 0)
    {
        printf("\nPasswords do not match. Please try again...\n");
        goto password;
    }

    printf("\nWriting... ");
    strcpy(config.password, password1);
    memcpy(config.magic, (void *)magic, 4);

    int res = fwrite(fd, sizeof(config), &config);
    if(res < 0)
    {
        printf("\nERROR");
        exit(EXIT_FAILURE);
    }

    system("he pr shell.elf");
    return 0;
}

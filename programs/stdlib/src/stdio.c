#include "stdio.h"
#include "stdlib.h"
#include "serpaeos.h"
#include "string.h"
#include <stdarg.h>

void print(const char *msg);

int puts(const char *str)
{
    print(str);
    return 0;
}
int putchar(int ch)
{
    sos_putchar((char)ch);
    return 0;
}

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
        }
        else
        {
            *(out++) = (char)c;
            putchar(c);
            if (c == ' ')
                i++;
        }
    *out = 0;
    return tmp;
}

int scanf(const char *fmt, ...)
{
    va_list ap;
    char *p;

    char *sval;
    int *ival;
    double *dval;

    va_start(ap, fmt);

    static char buf[2048];
    get_s(buf, strnocc((char *)fmt, ' ') + 1);
    char *token = strtok(buf, " ");

    for (p = (char *)fmt; *p && token != NULL; p++)
    {
        if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' || *p != '%')
            continue;

        switch (*++p)
        {
        case 's':
            sval = va_arg(ap, char *);
            strcpy(sval, token);
            break;
        case 'i':
        case 'd':
            ival = va_arg(ap, int *);
            *ival = atoi(token);
            break;
        case 'f':
            dval = va_arg(ap, double *);
            *dval = atof(token);
            break;
        default:
            continue;
        }
        token = strtok(NULL, " ");
    }
    va_end(ap);
    return 0;
}
int printf(const char *fmt, ...)
{
    va_list ap;
    const char *p;
    char *sval;
    int ival;
    int cval;

    va_start(ap, fmt);
    for (p = fmt; *p; p++)
    {
        if (*p != '%')
        {
            putchar(*p);
            continue;
        }

        switch (*++p)
        {
        case 'i':
        case 'd':
            ival = va_arg(ap, int);
            print(itoa(ival));
            break;

        case 's':
            sval = va_arg(ap, char *);
            print(sval);
            break;

        case 'c':
            cval = va_arg(ap, int);
            putchar(cval);
            break;

        default:
            putchar(*p);
            break;
        }
    }

    va_end(ap);

    return 0;
}

int getline(char *out, int max)
{
    char *tmp = out;
    unsigned char c, i;
    while ((c = getchar()) != '\r' && i < max)
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
            putchar(c);
            i++;
        }
    *out = 0;
    return i;
}

char *gets(char *out)
{
    char *tmp = out;
    unsigned char c;
    while ((c = getchar()) != '\r')
        if (c >= KEY_DOWN)
            continue;
        else if (c == '\b')
        {
            if (out == tmp)
                continue;
            *(--out) = 0;
            putchar('\b');
        }
        else
        {
            *(out++) = (char)c;
            putchar(c);
        }
    *out = 0;
    return tmp;
}
int system(const char *filename)
{
    struct command_argument *args = sos_parse_command(filename, 512);
    if (!args)
        return -2;
    return sos_system(args);
}

int getchar()
{
    unsigned char c;
    while ((c = getkey()) == 0)
        ;
    return (int)c;
}

char getc(FILE *fp)
{
    char res = 0;
    if (!fp)
        return 0;
    fread(fp, 1, &res);
    return res;
}

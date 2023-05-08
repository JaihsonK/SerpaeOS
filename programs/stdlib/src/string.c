#include "string.h"
#include "stdlib.h"

int strlen(const char *ptr)
{
    int i = 0;
    while (*ptr != 0)
    {
        i++;
        ptr += 1;
    }

    return i;
}

int strnlen(const char *ptr, int max)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (ptr[i] == 0)
            break;
    }

    return i;
}

int strnlen_terminator(const char *s, int max, char terminator)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (s[i] == '\0' || s[i] == terminator)
            break;
    }

    return i;
}

int istrncmp(const char *s1, const char *s2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}
int strncmp(const char *s1, const char *s2, int n)
{
    unsigned char u1, u2;

    while (n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

int strcmp(const char *s1, const char *s2)
{
    unsigned char u1, u2;

    do
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if(u1 != u2)
            return u1 - u2;
    }while(u1 && u2);
    return 0;
}

char *strcpy(char *dest, const char *src)
{
    char *res = dest;
    while (*src != 0)
    {
        *dest++ = *src++;
    }

    *dest = 0x00;

    return res;
}

char *strncpy(char *dest, const char *src, int count)
{
    int i = 0;
    for (i = 0; i < count - 1; i++)
    {
        if (src[i] == 0x00)
            break;

        dest[i] = src[i];
    }

    dest[i] = 0x00;
    return dest;
}

static char *sp = 0;
char *strtok(char *s, const char *delimiters)
{
    int i = 0;
    int len = strlen(delimiters);
    if (!s && !sp)
        return 0;
    if (s && !sp)
        sp = s;
    if (s == NULL && delimiters == NULL)
    {
        sp = 0;
        return sp;
    }
    char *p_start = sp;
    while (1)
    {
        for (i = 0; i < len; i++)
            if (*p_start == delimiters[i])
            {
                p_start++;
                break;
            }
        if (i == len)
        {
            sp = p_start;
            break;
        }
    }
    if (*sp == 0)
    {
        sp = 0;
        return sp;
    }

    // find end of subs
    while (*sp != 0)
    {
        for (i = 0; i < len; i++)
            if (*sp == delimiters[i])
            {
                *sp = 0;
                break;
            }
        sp++;
        if (i < len)
            break;
    }
    return p_start;
}

void reverse(char* s)
{
    int c, i, j;
    for(i = 0, j = strlen(s) - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strnocc(char* s, char delim)
{
    int i = 0;
    for(i = 0; *s; s++)
        if(*s == delim)
            i++;
    return i;
}

char *itoa(int i)
{
    static char text[12];
    int loc = 11;
    text[11] = 0;
    char neg = 1;
    if (i >= 0)
    {
        neg = 0;
        i = -i;
    }
    while (i)
    {
        text[--loc] = '0' - (i % 10);
        i /= 10;
    }
    if (loc == 11)
        text[--loc] = '0';
    if (neg)
        text[--loc] = '-';

    return &text[loc];
}

int atoi(char *s)
{
    int res = 0;
    while (*s != 0 && isdigit(*s))
    {
        res *= 10;
        res += tonumericdigit(*s);
        s++;
    }
    return res;
}

double atof(char *s)
{
    double val, power;
    int i, sign;

    for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t'; i++)
        ;

    sign = 1;
    if (s[i] == '+' || s[i] == '-')
        sign = (s[i++] == '+') ? 1 : -1;
    for (val = 0; isdigit(s[i]); i++)
        val = 10 * val + s[i] - '0';
    if (s[i] == '.')
        i++;
    for (power = 1; isdigit(s[i]); i++)
    {
        val = 10 * val + s[i] - '0';
        power *= 10;
    }
    return sign * val / power;
}

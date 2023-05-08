#ifndef STRING_H
#define STRING_H
#include <stdbool.h>
int strlen(const char* ptr);
int strnlen(const char* ptr, int max);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int count);
int strncmp(const char* str1, const char* str2, int n);
int istrncmp(const char* s1, const char* s2, int n);
int strnlen_terminator(const char* str, int max, char terminator);
char* strtok(char* str, const char* delimiters);
char* itoa(int i);
int atoi(char* str);
double atof(char* str);
int strnocc(char* s, char delim);
int strcmp(const char *s1, const char *s2);


#define isdigit(c) ((c >= '0' && c <= '9')? 1 : 0)
#define isalpha(c) (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? 1 : 0)
#define tolower(c) ((c <= 'z' && c >= 'a')? c : c + 32)
#define tonumericdigit(c) (isdigit(c)? c - '0' : c)


#endif

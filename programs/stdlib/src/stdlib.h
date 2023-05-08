#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
void* malloc(size_t size);
void free(void* ptr);
int atexit(void (*func)(void));

void exit(int code);

#endif